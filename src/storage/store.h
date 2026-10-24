#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QVariantList>
#include <QVariantMap>
#include <QThread>
#include <QSet>
#include <QDate>
#include <atomic>
#include "practice_stats.h"

class StoreWorker : public QObject {
    Q_OBJECT
public:
    ~StoreWorker() override;
public slots:
    void initialize(const QString& directory);
    void record(const QString& path, const QString& root, const QString& kind, int durationMs);
    void refresh();
    void clearHistory(const QString& root);
    void clearSources(const QStringList& roots);
    void analytics(quint64 request,int period,const QDate& month,int minutes,int studies,const QString& mode);
    void savePreferences(const QVariantMap& preferences);
    void importLegacy(const QString& directory);
    void close();
signals:
    void snapshot(QVariantList history, QStringList seen, QVariantMap stats);
    void error(QString message);
    void imported(int count);
    void completedPaths(QStringList paths);
    void analyticsReady(quint64 request,QVariantMap values);
    void healthChanged(bool healthy);
    void recovered(int entries,QString backup);
private:
    bool exec(const QString& sql);
    bool recoverDatabase();
    QSqlDatabase db_;
    QString connection_, directory_;
    QVector<PracticeDay> analyticsDays_;
    bool analyticsDirty_=true;
    int legacyCount_=0;
    QByteArray analyticsTimeZone_;
};
class Store : public QObject {
    Q_OBJECT
public:
    explicit Store(const QString& directory, QObject* parent = nullptr);
    ~Store() override;
    void record(QString path, QString root, QString kind, int durationMs = 0);
    void refresh();
    void clearHistory(QString root = {});
    void clearSources(QStringList roots);
    void analytics(quint64 request,int period,QDate month,int minutes,int studies,QString mode);
    void savePreferences(QVariantMap preferences);
    void importLegacy(QString directory);
signals:
    void snapshot(QVariantList history, QStringList seen, QVariantMap stats);
    void error(QString message);
    void imported(int count);
    void completedPaths(QStringList paths);
    void analyticsReady(quint64 request,QVariantMap values);
    void healthChanged(bool healthy);
    void recovered(int entries,QString backup);
private:
    QThread thread_;
    StoreWorker* worker_;
    std::atomic<quint64> latestAnalytics_{0};
};
QVariantMap loadPreferences(const QString& directory);
