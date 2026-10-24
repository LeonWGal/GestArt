#include "store.h"
#include "ui/themes.h"
#include "media/collections.h"
#include "practice_stats.h"
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSaveFile>
#include <QFile>
#include <QDateTime>
#include <QUuid>
#include <QFileInfo>
#include <algorithm>
#include <cmath>
#include <functional>
#include <QElapsedTimer>
#include <QTimeZone>

QVariantMap loadPreferences(const QString& dir) {
    QVariantMap defaults{{"schema",2},{"display_time",30},{"num_images",10},{"use_break",false},
        {"break_duration",300},{"unlimited_time",false},{"unlimited_images",false},
        {"unlimited_break",false},{"save_history",true},{"confirm_delete",true},
        {"show_timer",true},{"always_on_top",false},{"language","ru"},{"theme","dark"},
        {"grid_h_lines",2},{"grid_v_lines",2},{"timer_volume",50},{"timer_position","Center"},
        {"reduce_motion",false},{"recent_sources",QVariantList{}},{"source",QStringList{}},
        {"theme_base","black"},{"theme_follow_system",false},{"theme_white_tone","neutral"},
        {"theme_black_tone","neutral"},{"theme_accent_group","stones"},{"theme_accent","amber"},
        {"collections",QVariantList{}},{"collections_initialized",false},{"hide_timer_on_leave",false},{"drag_when_pinned",true},
        {"daily_goal_minutes",15},{"daily_goal_studies",10},{"daily_goal_mode","time"},
        {"session_glass",true},{"session_blur",true},{"session_panel_opacity",78},{"sidebar_compact",false}};
    QFile f(dir + "/settings.json");
    if (!f.open(QIODevice::ReadOnly)) return defaults;
    const QJsonDocument document = QJsonDocument::fromJson(f.readAll());
    if (!document.isObject()) return defaults;
    auto saved = document.object().toVariantMap();
    if(!saved.contains("use_break") && saved.contains("enable_breaks"))saved["use_break"]=saved["enable_breaks"];
    for (auto it = defaults.begin(); it != defaults.end(); ++it) {
        if (!saved.contains(it.key())) continue;
        const QVariant v = saved.value(it.key());
        const auto type = it.value().metaType();
        if (type.id() == QMetaType::Bool) { if (v.metaType().id() == QMetaType::Bool) it.value() = v; }
        else if (type.id() == QMetaType::Int) {
            if(v.metaType().id()!=QMetaType::Double && v.metaType().id()!=QMetaType::LongLong && v.metaType().id()!=QMetaType::Int)continue;
            bool ok=false;const double n=v.toDouble(&ok);
            if(ok && std::isfinite(n) && std::floor(n)==n)it.value()=int(std::clamp(n,0.0,it.key()=="num_images"?100000.0:86400.0));
        }
        else if (type.id() == QMetaType::QString) { if(v.metaType().id() == QMetaType::QString) it.value() = v; }
        else if(v.canConvert<QVariantList>() || v.canConvert<QStringList>()) it.value() = v;
    }
    defaults["timer_volume"] = std::clamp(defaults["timer_volume"].toInt(),0,100);
    defaults["grid_h_lines"] = std::clamp(defaults["grid_h_lines"].toInt(),0,10);
    defaults["grid_v_lines"] = std::clamp(defaults["grid_v_lines"].toInt(),0,10);
    defaults["display_time"] = std::max(1,defaults["display_time"].toInt());
    defaults["num_images"] = std::max(1,defaults["num_images"].toInt());
    defaults["daily_goal_minutes"]=std::clamp(defaults["daily_goal_minutes"].toInt(),1,240);
    defaults["daily_goal_studies"]=std::clamp(defaults["daily_goal_studies"].toInt(),1,1000);
    defaults["session_panel_opacity"]=std::clamp(defaults["session_panel_opacity"].toInt(),70,95);
    if(!QStringList{"time","studies","both"}.contains(defaults["daily_goal_mode"].toString()))defaults["daily_goal_mode"]="time";
    normalizeTheme(defaults,!saved.contains("theme_base"));
    defaults["collections"]=normalizeCollections(defaults.value("collections").toList());
    return defaults;
}
StoreWorker::~StoreWorker() { close(); }
bool StoreWorker::exec(const QString& sql) {
    QSqlQuery q(db_); if (q.exec(sql)) return true;
    emit healthChanged(false);emit error(q.lastError().text()); return false;
}
void StoreWorker::initialize(const QString& directory) {
    directory_ = directory; QDir().mkpath(directory);
    connection_ = QUuid::createUuid().toString();
    db_ = QSqlDatabase::addDatabase("QSQLITE",connection_);
    db_.setDatabaseName(directory + "/gestart.sqlite");
    db_.setConnectOptions("QSQLITE_BUSY_TIMEOUT=5000");
    if (!db_.open()) { emit healthChanged(false);emit error(db_.lastError().text()); return; }
    bool checked=false,healthy=false;QString failure,code;
    {QSqlQuery check(db_);
        checked=check.exec("PRAGMA quick_check") && check.next();
        healthy=checked && check.value(0).toString()=="ok";
        failure=check.lastError().text();code=check.lastError().nativeErrorCode();
    } // Destroy the statement before closing/renaming the SQLite file on Windows.
    // Never replace a busy or inaccessible database. Only repair confirmed corruption.
    if(!healthy){
        if((checked || code=="11" || code=="26") && recoverDatabase()){}
        else{emit healthChanged(false);emit error(failure.isEmpty()?QStringLiteral("History integrity check failed"):failure);return;}
    }
    if(!exec("PRAGMA journal_mode=WAL") || !exec("PRAGMA synchronous=FULL") ||
       !exec("CREATE TABLE IF NOT EXISTS view_events(id INTEGER PRIMARY KEY, path TEXT NOT NULL, root TEXT NOT NULL, kind TEXT NOT NULL, at TEXT, duration INTEGER NOT NULL DEFAULT 0)") ||
       !exec("CREATE INDEX IF NOT EXISTS events_root ON view_events(root,id)") ||
       !exec("CREATE INDEX IF NOT EXISTS events_path ON view_events(path,kind)") ||
       !exec("CREATE TABLE IF NOT EXISTS imports(source TEXT PRIMARY KEY)") ||
       !exec("PRAGMA user_version=1"))return;
    emit healthChanged(true);refresh();
}
bool StoreWorker::recoverDatabase(){
    const QString original=directory_+"/gestart.sqlite";
    const QString backup=directory_+"/history-backups/"+QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmsszzz")+"-"+QUuid::createUuid().toString(QUuid::Id128);
    if(!QDir().mkpath(backup))return false;
    // Preserve raw files before SQLite closes/checkpoints its journal.
    for(const QString suffix:{QString{},QStringLiteral("-wal"),QStringLiteral("-shm")}){
        if(QFile::exists(original+suffix) && !QFile::copy(original+suffix,backup+"/gestart.sqlite"+suffix))return false;
    }
    QVariantList records;QStringList imports;QSet<qint64> recoveredIds;bool partial=false;
    auto collect=[&](QSqlQuery& read){
        const qint64 id=read.value(0).toLongLong();const QString kind=read.value(3).toString();
        if(id<=0 || recoveredIds.contains(id) || read.value(1).toString().isEmpty() || !QStringList{"shown","browsed","completed","skipped","legacy"}.contains(kind))return;
        recoveredIds.insert(id);
        records.append(QVariant::fromValue(QVariantList{id,read.value(1),read.value(2).isNull()?QVariant(QStringLiteral("")):read.value(2),kind,read.value(4),std::max(0,read.value(5).toInt())}));
    };
    qint64 highest=0;
    {QSqlQuery read(db_);
        for(const auto& direction:{QStringLiteral("ASC"),QStringLiteral("DESC")}){
            if(read.exec("SELECT id,path,root,kind,at,duration FROM view_events NOT INDEXED ORDER BY id "+direction)){
                while(read.next()){highest=std::max(highest,read.value(0).toLongLong());collect(read);}
                if(read.lastError().isValid())partial=true;
            }else partial=true;
            read.finish();
        }
    }
    // Only split ranges that fail. A damaged page cannot terminate the entire
    // recovery, and healthy ranges are read once. Preserve sparse row ids.
    QElapsedTimer recoveryClock;recoveryClock.start();int attempts=0;
    std::function<void(qint64,qint64)> recoverRange=[&](qint64 low,qint64 high){
        if(++attempts>100000 || recoveryClock.elapsed()>30000){partial=true;return;}
        QSqlQuery read(db_);read.prepare("SELECT id,path,root,kind,at,duration FROM view_events NOT INDEXED WHERE id BETWEEN ? AND ? ORDER BY id");
        read.addBindValue(low);read.addBindValue(high);bool ok=read.exec();
        if(ok){while(read.next())collect(read);ok=!read.lastError().isValid();}
        read.finish();
        if(!ok){partial=true;}
        if(!ok && low<high){const auto middle=low+(high-low)/2;recoverRange(low,middle);recoverRange(middle+1,high);}
    };
    if(highest>0)recoverRange(1,highest);
    {QSqlQuery read(db_);if(read.exec("SELECT source FROM imports"))while(read.next())imports.append(read.value(0).toString());}
    std::sort(records.begin(),records.end(),[](const QVariant& a,const QVariant& b){return a.toList().first().toLongLong()<b.toList().first().toLongLong();});
    QSaveFile report(backup+"/recovery-report.json");if(report.open(QIODevice::WriteOnly)){report.write(QJsonDocument(QJsonObject{{"recoveredEvents",records.size()},{"highestReadableId",highest},{"partial",partial},{"rangeAttempts",attempts}}).toJson());report.commit();}
    const QString repaired=directory_+"/gestart-recovered-"+QUuid::createUuid().toString(QUuid::Id128)+".sqlite";
    const QString repairConnection=connection_+"-repair";bool valid=false;
    {
        auto target=QSqlDatabase::addDatabase("QSQLITE",repairConnection);target.setDatabaseName(repaired);
        if(target.open() && target.transaction()){
            QSqlQuery insert(target);
            valid=insert.exec("CREATE TABLE view_events(id INTEGER PRIMARY KEY,path TEXT NOT NULL,root TEXT NOT NULL,kind TEXT NOT NULL,at TEXT,duration INTEGER NOT NULL DEFAULT 0)") && insert.exec("CREATE TABLE imports(source TEXT PRIMARY KEY)");
            if(valid)valid=insert.prepare("INSERT INTO view_events VALUES(?,?,?,?,?,?)");
            for(const auto& record:records){if(!valid)break;const auto values=record.toList();for(int i=0;i<values.size();++i)insert.bindValue(i,values[i]);valid=insert.exec();}
            if(valid)valid=insert.prepare("INSERT OR IGNORE INTO imports VALUES(?)");
            for(const auto& source:imports){if(!valid)break;insert.bindValue(0,source);valid=insert.exec();}
            valid=valid && target.commit();
            if(valid)valid=insert.exec("PRAGMA quick_check") && insert.next() && insert.value(0).toString()=="ok";
        }
        target.close();
    }
    QSqlDatabase::removeDatabase(repairConnection);
    if(!valid)return false;
    db_.close();
    // Move every remaining original file aside; leave the raw backup intact.
    QStringList moved;
    for(const QString suffix:{QString{},QStringLiteral("-wal"),QStringLiteral("-shm")}){
        if(!QFile::exists(original+suffix))continue;
        if(!QFile::rename(original+suffix,backup+"/closed.sqlite"+suffix)){
            for(const auto& previous:moved)QFile::rename(backup+"/closed.sqlite"+previous,original+previous);
            db_.open();return false;
        }
        moved.append(suffix);
    }
    if(!QFile::rename(repaired,original)){
        for(const auto& suffix:moved)QFile::rename(backup+"/closed.sqlite"+suffix,original+suffix);
        db_.open();return false;
    }
    if(!db_.open())return false;
    emit recovered(records.size(),backup);return true;
}
void StoreWorker::record(const QString& path, const QString& root, const QString& kind, int duration) {
    if (!db_.isOpen()) return;
    QSqlQuery q(db_); q.prepare("INSERT INTO view_events(path,root,kind,at,duration) VALUES(?,?,?,?,?)");
    q.addBindValue(path); q.addBindValue(root.isEmpty()?QFileInfo(path).absolutePath():root); q.addBindValue(kind);
    q.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)); q.addBindValue(duration);
    if (!q.exec()){emit healthChanged(false);emit error(q.lastError().text());}else{if(kind=="completed"||kind=="legacy")analyticsDirty_=true;emit healthChanged(true);}
    // A UI snapshot is explicitly requested on the history page, not rebuilt per image.
}
void StoreWorker::refresh() {
    if (!db_.isOpen()) return;
    QVariantList history; QStringList seen; QVariantMap stats;
    QSqlQuery q(db_);
    if(!q.exec("SELECT path,kind,at,duration FROM view_events ORDER BY id DESC LIMIT 2000")){emit healthChanged(false);emit error(q.lastError().text());return;}
    while(q.next()) history.append(QVariantMap{{"path",q.value(0)},{"kind",q.value(1)},
        {"at",q.value(2)},{"duration",q.value(3)}});
    q.exec("SELECT DISTINCT path FROM view_events WHERE kind IN ('shown','browsed','legacy','skipped')");
    while(q.next()) seen.append(q.value(0).toString());
    q.exec("SELECT COUNT(*),COALESCE(SUM(duration),0) FROM view_events WHERE kind='completed'");
    if(q.next()){stats["completed"]=q.value(0);stats["practiceMs"]=q.value(1);}
    q.exec("SELECT COUNT(*) FROM view_events WHERE kind IN ('shown','browsed','legacy')");
    if(q.next()) stats["viewed"]=q.value(0);
    QStringList completed;q.exec("SELECT DISTINCT path FROM view_events WHERE kind='completed'");
    while(q.next())completed.append(q.value(0).toString());
    emit completedPaths(completed);
    emit snapshot(history,seen,stats);
}
void StoreWorker::clearHistory(const QString& root) {
    analyticsDirty_=true;
    QSqlQuery q(db_);
    if(root.isEmpty()) q.prepare("DELETE FROM view_events");
    else {q.prepare("DELETE FROM view_events WHERE root=?");q.addBindValue(root);}
    if(!q.exec()) emit error(q.lastError().text());
    refresh();
}
void StoreWorker::savePreferences(const QVariantMap& preferences) {
    QSaveFile file(directory_ + "/settings.json");
    if(!file.open(QIODevice::WriteOnly)){emit error(file.errorString());return;}
    const auto bytes=QJsonDocument(QJsonObject::fromVariantMap(preferences)).toJson();
    if(file.write(bytes)!=bytes.size() || !file.commit()) emit error(file.errorString());
}
void StoreWorker::analytics(quint64 request,int period,const QDate& month,int minutes,int studies,const QString& mode){
    if(!db_.isOpen())return;
    const auto zone=QTimeZone::systemTimeZoneId();if(zone!=analyticsTimeZone_){analyticsTimeZone_=zone;analyticsDirty_=true;}
    if(analyticsDirty_){
        QVector<PracticeDay> days;QSqlQuery q(db_);
        if(!q.exec("SELECT date(at,'localtime'),COUNT(*),COALESCE(SUM(duration),0) FROM view_events WHERE kind='completed' AND date(at,'localtime') IS NOT NULL GROUP BY date(at,'localtime') ORDER BY date(at,'localtime')")){emit error(q.lastError().text());return;}
        while(q.next())days.append({QDate::fromString(q.value(0).toString(),Qt::ISODate),q.value(1).toInt(),q.value(2).toLongLong()});
        if(q.lastError().isValid()){emit error(q.lastError().text());return;}
        if(!q.exec("SELECT COUNT(*) FROM view_events WHERE kind='legacy'")||!q.next()){emit error(q.lastError().text());return;}
        legacyCount_=q.value(0).toInt();analyticsDays_=std::move(days);analyticsDirty_=false;
    }
    auto result=summarizePractice(analyticsDays_,QDate::currentDate(),month,period,minutes,studies,mode);
    result["legacyCount"]=legacyCount_;
    emit analyticsReady(request,result);
}
void StoreWorker::clearSources(const QStringList& roots) {
    if(!db_.isOpen() || roots.isEmpty())return;
    analyticsDirty_=true;
    QStringList paths;QSqlQuery query(db_);
    if(!query.exec("SELECT DISTINCT path FROM view_events")){emit error(query.lastError().text());return;}
    while(query.next())if(pathInSources(query.value(0).toString(),roots))paths.append(query.value(0).toString());
    query.finish();
    if(!db_.transaction()){emit error(db_.lastError().text());return;}
    query.prepare("DELETE FROM view_events WHERE path=?");
    for(const auto& path:paths){query.bindValue(0,path);if(!query.exec()){db_.rollback();emit error(query.lastError().text());return;}}
    if(!db_.commit()){db_.rollback();emit error(db_.lastError().text());return;}
    refresh();
}
void StoreWorker::importLegacy(const QString& directory) {
    analyticsDirty_=true;
    const QString key=QDir(directory).absolutePath();
    QSqlQuery q(db_);q.prepare("SELECT 1 FROM imports WHERE source=?");q.addBindValue(key);q.exec();
    if(q.next()){emit imported(0);return;}
    QFile f(key+"/history.json");
    if(!f.open(QIODevice::ReadOnly)){emit error("Cannot read legacy history.json");return;}
    const auto doc=QJsonDocument::fromJson(f.readAll());
    if(!doc.isArray()){emit error("Legacy history must be a JSON array");return;}
    if(!db_.transaction()){emit error(db_.lastError().text());return;}
    int count=0;QSet<QString> unique;
    q.prepare("INSERT INTO view_events(path,root,kind,at,duration) VALUES(?,?,'legacy',NULL,0)");
    for(const auto& value:doc.array()){
        if(!value.isString() || unique.contains(value.toString()))continue;
        const QString path=value.toString();unique.insert(path);
        q.addBindValue(path);q.addBindValue(QFileInfo(path).absolutePath());
        if(!q.exec()){db_.rollback();emit error(q.lastError().text());return;} ++count;
    }
    q.prepare("INSERT INTO imports(source) VALUES(?)");q.addBindValue(key);
    if(!q.exec() || !db_.commit()){db_.rollback();emit error(q.lastError().text());return;}
    refresh();emit imported(count);
}
void StoreWorker::close() {
    if(connection_.isEmpty())return;
    db_.close();db_=QSqlDatabase{};QSqlDatabase::removeDatabase(connection_);connection_.clear();
}
Store::Store(const QString& directory,QObject* parent):QObject(parent),worker_(new StoreWorker){
    worker_->moveToThread(&thread_);
    connect(&thread_,&QThread::finished,worker_,&QObject::deleteLater);
    connect(worker_,&StoreWorker::snapshot,this,&Store::snapshot);
    connect(worker_,&StoreWorker::error,this,&Store::error);
    connect(worker_,&StoreWorker::imported,this,&Store::imported);
    connect(worker_,&StoreWorker::completedPaths,this,&Store::completedPaths);
    connect(worker_,&StoreWorker::analyticsReady,this,&Store::analyticsReady);
    connect(worker_,&StoreWorker::healthChanged,this,&Store::healthChanged);
    connect(worker_,&StoreWorker::recovered,this,&Store::recovered);
    thread_.start();QMetaObject::invokeMethod(worker_,[=]{worker_->initialize(directory);});
}
Store::~Store(){QMetaObject::invokeMethod(worker_,&StoreWorker::close,Qt::BlockingQueuedConnection);thread_.quit();thread_.wait();}
void Store::record(QString path,QString root,QString kind,int ms){QMetaObject::invokeMethod(worker_,[=,this]{worker_->record(path,root,kind,ms);});}
void Store::refresh(){QMetaObject::invokeMethod(worker_,&StoreWorker::refresh);}
void Store::clearHistory(QString root){QMetaObject::invokeMethod(worker_,[=,this]{worker_->clearHistory(root);});}
void Store::clearSources(QStringList roots){QMetaObject::invokeMethod(worker_,[=,this]{worker_->clearSources(roots);});}
void Store::analytics(quint64 request,int period,QDate month,int minutes,int studies,QString mode){latestAnalytics_.store(request);QMetaObject::invokeMethod(worker_,[=,this]{if(latestAnalytics_.load()==request)worker_->analytics(request,period,month,minutes,studies,mode);});}
void Store::savePreferences(QVariantMap p){QMetaObject::invokeMethod(worker_,[=,this]{worker_->savePreferences(p);});}
void Store::importLegacy(QString directory){QMetaObject::invokeMethod(worker_,[=,this]{worker_->importLegacy(directory);});}
