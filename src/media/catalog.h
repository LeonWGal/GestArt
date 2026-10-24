#pragma once
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QThreadPool>
#include <QUrl>
#include <atomic>
#include <memory>
#include <QSet>

struct MediaEntry { QString path; QString name; QString source; qint64 bytes = 0; };
Q_DECLARE_METATYPE(QVector<MediaEntry>)

QString imageSource(const QString& path, quint64 request = 0);
class CatalogModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role { PathRole = Qt::UserRole + 1, NameRole, SourceRole, BytesRole, CompletedRole };
    explicit CatalogModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}
    int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : static_cast<int>(entries_.size()); }
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void clear();
    void append(const QVector<MediaEntry>& entries);
    QStringList paths() const;
    void removePath(const QString& path);
    void setCompleted(const QSet<QString>& paths);
    void markCompleted(const QString& path);
    bool isCompleted(const QString& path)const{return completed_.contains(path);}
    QStringList pathsForSources(const QStringList& sources)const;
private:
    QVector<MediaEntry> entries_;
    QSet<QString> completed_;
    QHash<QString,int> rows_;
};

class CatalogFilter : public QSortFilterProxyModel {
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    void setAllowedPaths(QSet<QString> paths,bool limited){paths_=std::move(paths);limited_=limited;invalidateFilter();}
    void setCompletedOnly(bool enabled){completedOnly_=enabled;invalidateFilter();}
protected:
    bool filterAcceptsRow(int row,const QModelIndex& parent)const override {
        const auto idx=sourceModel()->index(row,0,parent);
        return (!limited_||paths_.contains(sourceModel()->data(idx,CatalogModel::PathRole).toString()))
            && (!completedOnly_||sourceModel()->data(idx,CatalogModel::CompletedRole).toBool())
            && QSortFilterProxyModel::filterAcceptsRow(row,parent);
    }
private:
    QSet<QString> paths_;bool limited_=false,completedOnly_=false;
};

class CatalogScanner : public QObject {
    Q_OBJECT
public:
    explicit CatalogScanner(QObject* parent = nullptr);
    ~CatalogScanner() override;
    quint64 scan(const QStringList& sources);
    void cancel();
    static QStringList extensions();
signals:
    void batch(quint64 generation, QVector<MediaEntry> entries);
    void finished(quint64 generation, int count, QStringList warnings);
private:
    QThreadPool pool_;
    quint64 generation_ = 0;
    std::shared_ptr<std::atomic_bool> cancelled_;
};
