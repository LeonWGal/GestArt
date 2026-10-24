#include "image_provider.h"
#include <QImageReader>
#include <QColorSpace>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QSvgRenderer>
#include <QPainter>
#include <QQuickTextureFactory>
#include <QRunnable>
#ifdef GESTART_HAS_HEIF
#include "heif_decoder.h"
#endif

QImage ImageCache::get(const QString& key) {
    std::lock_guard lock(mutex);
    auto it = entries.find(key);
    if (it == entries.end()) { ++misses; return {}; }
    ++hits; lru.splice(lru.end(), lru, it->order); return it->image;
}
void ImageCache::put(const QString& key, const QImage& image) {
    const qint64 cost = image.sizeInBytes();
    std::lock_guard lock(mutex);
    if (image.isNull() || cost > limit || entries.contains(key)) return;
    while (used + cost > limit && !lru.empty()) {
        auto oldest = entries.find(lru.front()); used -= oldest->bytes;
        entries.erase(oldest); lru.pop_front();
    }
    lru.push_back(key); entries.insert(key, {image,cost,std::prev(lru.end())}); used += cost;
}
void ImageCache::invalidate(const QString& path) {
    std::lock_guard lock(mutex);
    for (auto it = entries.begin(); it != entries.end();) {
        if (it.key().startsWith(path + '\n')) { used -= it->bytes; lru.erase(it->order); it = entries.erase(it); }
        else ++it;
    }
}
void ImageCache::clear() { std::lock_guard lock(mutex); entries.clear(); lru.clear(); used = 0; }
qint64 ImageCache::bytes() const { std::lock_guard lock(mutex); return used; }
QSize imageNaturalSize(const QString& path) {
#ifdef GESTART_HAS_HEIF
    if(path.endsWith(".heic",Qt::CaseInsensitive)||path.endsWith(".heif",Qt::CaseInsensitive))return heifNaturalSize(path);
#endif
    if(path.endsWith(".svg",Qt::CaseInsensitive)||path.endsWith(".svgz",Qt::CaseInsensitive))return QSvgRenderer(path).defaultSize();
    QImageReader reader(path);QSize size=reader.size();
    if(reader.transformation().testFlag(QImageIOHandler::TransformationRotate90))size.transpose();
    return size;
}
QImage grayscaleImage(QImage image) {
    if(image.isNull())return image;
    image=image.convertToFormat(QImage::Format_ARGB32);
    for(int y=0;y<image.height();++y){auto* row=reinterpret_cast<QRgb*>(image.scanLine(y));
        for(int x=0;x<image.width();++x){const int luminance=qRound(qRed(row[x])*0.2126+qGreen(row[x])*0.7152+qBlue(row[x])*0.0722);
            row[x]=qRgba(luminance,luminance,luminance,qAlpha(row[x]));}}
    return image;
}
QImage decodeImage(const QString& path, QSize requested, QString* error) {
    if (!QFileInfo::exists(path)) { if (error) *error = "File does not exist"; return {}; }
    if (!requested.isValid() || requested.isEmpty()) requested = {1920,1080};
    requested = requested.boundedTo({8192,8192});
#ifdef GESTART_HAS_HEIF
    if(path.endsWith(".heic",Qt::CaseInsensitive)||path.endsWith(".heif",Qt::CaseInsensitive))return decodeHeif(path,requested,error);
#endif
    if (path.endsWith(".svg",Qt::CaseInsensitive) || path.endsWith(".svgz",Qt::CaseInsensitive)) {
        QSvgRenderer svg(path);
        if (!svg.isValid()) { if(error) *error = "Invalid SVG"; return {}; }
        QSize natural = svg.defaultSize(); if (!natural.isValid()) natural = {800,600};
        const QSize target = natural.scaled(requested, Qt::KeepAspectRatio);
        QImage result(target,QImage::Format_ARGB32_Premultiplied); result.fill(Qt::transparent);
        QPainter painter(&result); svg.render(&painter); painter.end();
        result.setColorSpace(QColorSpace::SRgb); return result;
    }
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QSize natural = reader.size();
    if (natural.isValid()) {
        QSize rawRequested=requested;
        if(reader.transformation().testFlag(QImageIOHandler::TransformationRotate90))rawRequested.transpose();
        reader.setScaledSize(natural.scaled(rawRequested,Qt::KeepAspectRatio));
    }
    QImage result = reader.read();
    if (result.isNull()) { if (error) *error = reader.errorString(); return {}; }
    if (result.colorSpace().isValid() && result.colorSpace() != QColorSpace(QColorSpace::SRgb))
        result.convertToColorSpace(QColorSpace::SRgb);
    else result.setColorSpace(QColorSpace::SRgb);
    return result;
}
class MediaImageResponse final : public QQuickImageResponse {
public:
    MediaImageResponse(QString id, QSize requested, std::shared_ptr<ImageCache> cache)
        : id_(std::move(id)), size_(requested), cache_(std::move(cache)) {}
    void run() {
        const auto parts = id_.split('/');
        const QString path = QString::fromUtf8(QByteArray::fromBase64(parts.value(0).toLatin1(), QByteArray::Base64UrlEncoding));
        const QString key = path + '\n' + QString::number(QFileInfo(path).lastModified().toMSecsSinceEpoch()) + '\n' + QString::number(size_.width()) + 'x' + QString::number(size_.height());
        if (!cancelled_) {
            image_ = cache_->get(key);
            if (image_.isNull()) {
                image_ = decodeImage(path,size_,&error_);
                if (!cancelled_ && !image_.isNull()) cache_->put(key,image_);
            }
            if (parts.last()=="bw") image_=grayscaleImage(image_);
        }
        emit finished();
    }
    void cancel() override { cancelled_.store(true); }
    QString errorString() const override { return error_; }
    QQuickTextureFactory* textureFactory() const override { return QQuickTextureFactory::textureFactoryForImage(image_); }
private:
    QString id_, error_;
    QSize size_;
    QImage image_;
    std::shared_ptr<ImageCache> cache_;
    std::atomic_bool cancelled_ = false;
};
ImageProvider::ImageProvider() : cache_(std::make_shared<ImageCache>()) { pool_.setMaxThreadCount(2); pool_.setExpiryTimeout(10000); }
ImageProvider::~ImageProvider() { pool_.waitForDone(); }
QQuickImageResponse* ImageProvider::requestImageResponse(const QString& id, const QSize& size) {
    auto* response = new MediaImageResponse(id,size,cache_);
    pool_.start([response]{response->run();}, size.width() > 512 ? 10 : 0); return response;
}
QImage IconProvider::requestImage(const QString& id, QSize* size, const QSize& requested) {
    const auto parts = id.split('/');
    QFile file(":/assets/icons/" + parts.value(0) + ".svg");
    if (!file.open(QIODevice::ReadOnly)) return {};
    QByteArray data = file.readAll();
    const QByteArray color = parts.value(1,"a3a3a3").toLatin1();
    data.replace("currentColor", "#" + color);
    QSvgRenderer svg(data);
    const QSize target = requested.isValid()&&!requested.isEmpty() ? requested.boundedTo({512,512}) : QSize(24,24);
    QImage result(target,QImage::Format_ARGB32_Premultiplied); result.fill(Qt::transparent);
    if(result.isNull())return {};
    QPainter painter(&result); svg.render(&painter); painter.end();
    if (size) *size = target; return result;
}
