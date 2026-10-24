#pragma once
#include <QQuickImageProvider>
#include <QThreadPool>
#include <QImage>
#include <QHash>
#include <QObject>
#include <list>
#include <mutex>
#include <memory>
#include <atomic>

struct ImageCache {
    struct Entry { QImage image; qint64 bytes; std::list<QString>::iterator order; };
    mutable std::mutex mutex;
    std::list<QString> lru;
    QHash<QString, Entry> entries;
    qint64 used = 0;
    qint64 limit = 256ll * 1024 * 1024;
    quint64 hits = 0, misses = 0;
    QImage get(const QString& key);
    void put(const QString& key, const QImage& image);
    void invalidate(const QString& path);
    void clear();
    qint64 bytes() const;
};
QImage decodeImage(const QString& path, QSize requestedSize, QString* error = nullptr);
QSize imageNaturalSize(const QString& path);
QImage grayscaleImage(QImage image);

class ImageProvider : public QQuickAsyncImageProvider {
public:
    ImageProvider();
    ~ImageProvider() override;
    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& size) override;
    std::shared_ptr<ImageCache> cache() const { return cache_; }
private:
    std::shared_ptr<ImageCache> cache_;
    QThreadPool pool_;
};
class IconProvider : public QQuickImageProvider {
public:
    IconProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}
    QImage requestImage(const QString& id, QSize* size, const QSize& requested) override;
};
