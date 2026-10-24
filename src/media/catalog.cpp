#include "catalog.h"
#include "collections.h"
#include <QtConcurrent>
#include <QDirIterator>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QSet>
#include <QDateTime>
#include <QImageReader>

QString imageSource(const QString& path, quint64 request) {
    const auto encoded = path.toUtf8().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    return QStringLiteral("image://media/%1/%2/%3").arg(QString::fromLatin1(encoded))
        .arg(0).arg(request); // File freshness is checked by the worker, never by QML or the GUI thread.
}
QVariant CatalogModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) return {};
    const auto& e = entries_[index.row()];
    switch (role) {
    case PathRole: return e.path;
    case NameRole: return e.name;
    case SourceRole: return e.source;
    case BytesRole: return e.bytes;
    case CompletedRole:return completed_.contains(e.path);
    default: return {};
    }
}
QHash<int, QByteArray> CatalogModel::roleNames() const { return {{PathRole,"mediaPath"},{NameRole,"mediaName"},{SourceRole,"mediaSource"},{BytesRole,"mediaBytes"},{CompletedRole,"mediaCompleted"}}; }
void CatalogModel::clear() { beginResetModel(); entries_.clear();rows_.clear(); endResetModel(); }
void CatalogModel::append(const QVector<MediaEntry>& entries) {
    if (entries.isEmpty()) return;
    const int first = static_cast<int>(entries_.size());
    beginInsertRows({}, first, first + static_cast<int>(entries.size()) - 1);
    entries_ += entries;
    for(int i=first;i<entries_.size();++i)rows_[entries_[i].path]=i;
    endInsertRows();
}
QStringList CatalogModel::paths() const {
    QStringList result; result.reserve(entries_.size());
    for (const auto& e : entries_) result.append(e.path);
    return result;
}
void CatalogModel::removePath(const QString& path) {
    for (int i = 0; i < entries_.size(); ++i) if (entries_[i].path == path) {
        beginRemoveRows({},i,i); entries_.removeAt(i);rows_.remove(path);for(int row=i;row<entries_.size();++row)rows_[entries_[row].path]=row;endRemoveRows(); return;
    }
}
void CatalogModel::setCompleted(const QSet<QString>& paths){
    const auto changed=(completed_-paths)+(paths-completed_);completed_=paths;
    for(const auto& path:changed)if(rows_.contains(path)){const auto row=index(rows_.value(path),0);emit dataChanged(row,row,{CompletedRole});}
}
void CatalogModel::markCompleted(const QString& path){
    if(completed_.contains(path))return;completed_.insert(path);
    if(rows_.contains(path)){const auto row=index(rows_.value(path),0);emit dataChanged(row,row,{CompletedRole});}
}
QStringList CatalogModel::pathsForSources(const QStringList& sources)const{
    QStringList result;for(const auto& item:entries_)if(pathInSources(item.path,sources))result.append(item.path);return result;
}
CatalogScanner::CatalogScanner(QObject* parent) : QObject(parent) { pool_.setMaxThreadCount(1); }
CatalogScanner::~CatalogScanner() { cancel(); pool_.waitForDone(); }
void CatalogScanner::cancel() { if (cancelled_) cancelled_->store(true); }
QStringList CatalogScanner::extensions() {
    static const QStringList supported=[] {
        QStringList result{"jpg","jpeg","png","bmp","gif","webp","tif","tiff","ico","svg","svgz"};
        const auto formats=QImageReader::supportedImageFormats();
#ifdef GESTART_HAS_HEIF
        result.append({"heic","heif"});
#else
        if(formats.contains("heic"))result.append("heic");
        if(formats.contains("heif"))result.append("heif");
#endif
        if(formats.contains("avif"))result.append("avif");
        return result;
    }();
    return supported;
}
quint64 CatalogScanner::scan(const QStringList& sources) {
    cancel();
    cancelled_ = std::make_shared<std::atomic_bool>(false);
    const auto token = cancelled_;
    const quint64 generation = ++generation_;
    pool_.start([this, sources, token, generation] {
        QVector<MediaEntry> pending;
        QSet<QString> visited;
        QStringList warnings;
        int count = 0;
        QElapsedTimer interval; interval.start();
        auto flush = [&] {
            if (pending.isEmpty() || token->load()) return;
            emit batch(generation, pending); pending.clear(); interval.restart();
        };
        auto add = [&](const QFileInfo& info) {
            if (!info.isFile() || info.isSymLink() || !extensions().contains(info.suffix().toLower())) return;
            const QString path = info.absoluteFilePath();
            if (visited.contains(path)) return;
            visited.insert(path);
            if (!info.isReadable()) { if (warnings.size() < 20) warnings.append(path); return; }
            pending.append({path, info.fileName(), imageSource(path), info.size()}); ++count;
            if (pending.size() >= 256 || interval.elapsed() >= 100) flush();
        };
        for (const auto& source : sources) {
            if (token->load()) break;
            const QFileInfo info(source);
            if (!info.exists()) { warnings.append(source); continue; }
            if (info.isDir()) {
                QDirIterator it(source, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
                while (!token->load() && it.hasNext()) { it.next(); add(it.fileInfo()); }
            } else add(info);
        }
        flush();
        if (!token->load()) emit finished(generation, count, warnings);
    });
    return generation;
}
