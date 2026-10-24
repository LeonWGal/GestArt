#include "download_queue.h"
#include <QTimer>
#include <QSet>
#include <algorithm>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>
#include <QDesktopServices>
#include <QImageReader>
#include <QJsonArray>
DownloadQueue::DownloadQueue(QString directory,QObject* parent,QNetworkAccessManager* network):QObject(parent),worker_(directory,nullptr,network),directory_(directory),destination_(directory+"/online-references"){
    QFile preferences(directory_+"/downloader-settings.json");if(preferences.open(QIODevice::ReadOnly)){
        const auto previous=QJsonDocument::fromJson(preferences.readAll()).object().value("destination").toString();
        if(QDir::isAbsolutePath(previous))destination_=QDir::cleanPath(previous);
    }
    QFile library(directory_+"/download-library.json");
    if(library.open(QIODevice::ReadOnly)){
        for(const auto& value:QJsonDocument::fromJson(library.readAll()).array()){
            const auto set=value.toObject().toVariantMap();
            if(QDir::isAbsolutePath(set.value("folder").toString())&&set.value("count").toInt()>0)savedSets_.append(set);
        }
    }
    library.close();
    refreshSavedSets();
    connect(&worker_,&OnlineSources::changed,this,&DownloadQueue::workerChanged);
    connect(&worker_,&OnlineSources::imported,this,[this](const QString& folder,const QString& name,int count){
        if(current_<0)return;
        auto job=jobs_[current_].toMap();job["completed"]=count;job["folder"]=folder;job["savedName"]=name;job["target"]=worker_.requested();jobs_[current_]=job;
        rememberSaved(folder,name,job);
        emit imported(folder,name,count);
        if(phase_==Phase::Cancelling){finishJob("Stopped",worker_.error());return;}
        const bool practice=current_==practiceJob_;if(practice)practiceJob_=-1;
        finishJob(worker_.error().isEmpty()?"Completed":"Partial download",worker_.error());
        if(practice)emit practiceReady(folder);
    });
}
DownloadQueue::~DownloadQueue(){if(running_)cancel();}
void DownloadQueue::persistSaved(){
    QDir().mkpath(directory_);
    QSaveFile file(directory_+"/download-library.json");
    if(file.open(QIODevice::WriteOnly)){file.write(QJsonDocument::fromVariant(savedSets_).toJson());file.commit();}
}
void DownloadQueue::rememberSaved(const QString& folder,const QString& name,const QVariantMap& job){
    QFile manifest(folder+"/sources.json");if(!manifest.open(QIODevice::ReadOnly))return;
    const auto records=QJsonDocument::fromJson(manifest.readAll()).object();if(records.isEmpty())return;
    int count=0;QString preview;QSet<QString> names;
    for(auto it=records.begin();it!=records.end();++it){
        if(QFileInfo(it.key()).fileName()!=it.key()||!QFileInfo::exists(folder+"/"+it.key()))continue;
        ++count;names.insert(it.value().toObject().value("set").toString());
        if(preview.isEmpty()&&QImageReader(folder+"/"+it.key()).canRead())preview=QUrl::fromLocalFile(folder+"/"+it.key()).toString();
    }
    if(count==0)return;
    auto saved=job;const auto first=records.begin().value().toObject();
    saved["plannedCount"]=job.value("count",count);
    saved["folder"]=folder;saved["count"]=count;saved["preview"]=preview;saved["available"]=QFileInfo(folder).isDir();
    saved["name"]=job.isEmpty()?(names.size()>1?QString("Combined references"):first.value("set").toString()):job.value("name",name).toString();
    saved["provider"]=job.value("provider",first.value("provider").toString());
    saved["fullPack"]=job.value("all").toBool()&&worker_.error().isEmpty();
    saved["partial"]=!job.isEmpty()&&!worker_.error().isEmpty();
    saved["updated"]=QFileInfo(folder+"/sources.json").lastModified().toString(Qt::ISODate);
    for(int i=0;i<savedSets_.size();++i){
        const auto previous=savedSets_[i].toMap();if(previous.value("folder").toString()!=folder)continue;
        if(job.isEmpty()){
            auto refreshed=previous;
            for(const auto& key:QStringList{"count","preview","available","updated"})refreshed[key]=saved.value(key);
            if(refreshed==previous)return;
            saved=refreshed;
        }
        else if(previous.value("fullPack").toBool()&&count>=previous.value("count").toInt()){saved["fullPack"]=true;saved["partial"]=false;}
        savedSets_.removeAt(i);break;
    }
    savedSets_.prepend(saved);persistSaved();emit savedChanged();
}
void DownloadQueue::refreshSavedSets(){
    // Discover the previous release's separate sets and provider folders without scanning arbitrary user directories.
    QStringList roots{directory_+"/online-references",destination_};roots.removeDuplicates();
    for(const auto& root:roots)for(const auto& provider:QStringList{"quickposes","proko","poselibrary","sketchdaily"}){
        const auto merged=root+"/"+provider;
        if(QFileInfo::exists(merged+"/sources.json"))rememberSaved(merged,provider);
        const auto packs=root+"/sets/"+provider;
        for(const auto& name:QDir(packs).entryList(QDir::Dirs|QDir::NoDotAndDotDot))rememberSaved(packs+"/"+name,name);
    }
    for(auto& value:savedSets_){auto set=value.toMap();set["available"]=QFileInfo(set.value("folder").toString()).isDir();value=set;}
    emit savedChanged();
}
bool DownloadQueue::resumeSaved(int index){
    if(running_||index<0||index>=savedSets_.size())return false;
    const auto saved=savedSets_[index].toMap();
    if(saved.value("setId").toString().isEmpty())return false;
    jobs_.clear();current_=-1;
    const int count=saved.value("all").toBool()?0:saved.value("plannedCount",20).toInt();
    if(!add(saved.value("provider").toString(),{QVariantMap{{"id",saved.value("setId")},{"name",saved.value("name")}}},count,true,saved.value("filters").toMap()))return false;
    auto job=jobs_.first().toMap();job["destination"]=saved.value("destination",destination_);jobs_[0]=job;
    startSelected({0});return true;
}
void DownloadQueue::openSavedFolder(int index){
    if(index<0||index>=savedSets_.size())return;
    const auto folder=savedSets_[index].toMap().value("folder").toString();
    if(QFileInfo(folder).isDir())QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
}
void DownloadQueue::setDestination(const QUrl& folder){
    if(running_||!folder.isLocalFile()||!QFileInfo(folder.toLocalFile()).isDir())return;
    destination_=QDir::cleanPath(QFileInfo(folder.toLocalFile()).absoluteFilePath());
    QSaveFile file(directory_+"/downloader-settings.json");if(file.open(QIODevice::WriteOnly)){file.write(QJsonDocument(QJsonObject{{"destination",destination_}}).toJson());file.commit();}
    emit changed();
}
void DownloadQueue::openFolder(int index){
    if(index<0||index>=jobs_.size())return;const auto folder=jobs_[index].toMap().value("folder").toString();
    if(!folder.isEmpty()&&QFileInfo(folder).isDir())QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
}
int DownloadQueue::completed()const{int value=0;for(const auto& job:jobs_)value+=job.toMap().value("completed").toInt();return value;}
int DownloadQueue::requested()const{int value=0;for(const auto& job:jobs_)value+=job.toMap().value("target",job.toMap().value("count")).toInt();return value;}
bool DownloadQueue::add(const QString& provider,const QVariantList& sets,int count,bool separate,const QVariantMap& filters){
    if(running_||OnlineSources::website(provider).isEmpty())return false;
    error_.clear();
    const auto commonFilters=OnlineSources::normalizedFilters(provider,filters);
    const bool all=count==0;
    if(all&&provider=="sketchdaily"){error_="This source offers random references, not a complete pack. Choose an image count.";emit changed();return false;}
    count=all?0:std::clamp(count,1,100);QVariantList additions;QSet<QString> ids;
    for(const auto& value:sets){const auto set=value.toMap();const auto id=set.value("id").toString();const auto name=set.value("name").toString();
        if(id.isEmpty()||id.size()>256||name.trimmed().isEmpty()||ids.contains(id))continue;
        ids.insert(id);auto normalized=commonFilters;
        if(provider=="quickposes"){const auto type=id.section('_',0,0);if(!QStringList{"pose","feet","hands","face"}.contains(type))normalized.clear();else if(type=="hands"||type=="face")normalized.remove("clothing");}
        bool duplicate=false;
        for(const auto& existing:jobs_){const auto job=existing.toMap();if(job.value("provider")==provider&&job.value("setId")==id&&job.value("separate").toBool()==separate&&job.value("filters").toMap()==normalized&&job.value("destination")==destination_&&job.value("status")=="Queued"&&job.value("count").toInt()==count)duplicate=true;}
        if(!duplicate)additions.append(QVariantMap{{"provider",provider},{"setId",id},{"name",name},{"count",count},{"all",all},{"target",all?set.value("imageCount").toInt():count},{"separate",separate},{"filters",normalized},{"destination",destination_},{"completed",0},{"status","Queued"},{"error",QString{}}});
    }
    if(jobs_.size()+additions.size()>20||(!all&&requested()+int(additions.size())*count>500)){error_="The queue supports up to 20 sets and 500 images.";emit changed();return false;}
    if(additions.isEmpty())return false;
    error_.clear();jobs_.append(additions);emit changed();return true;
}
void DownloadQueue::remove(int index){if(running_||index<0||index>=jobs_.size())return;jobs_.removeAt(index);error_.clear();emit changed();}
void DownloadQueue::clear(){if(running_)return;jobs_.clear();error_.clear();current_=-1;emit changed();}
void DownloadQueue::startSelected(const QList<int>& indices,int practiceJob){
    if(running_||indices.isEmpty())return;
    selectedJobs_=QSet<int>(indices.begin(),indices.end());practiceJob_=practiceJob;
    for(const auto i:indices){auto job=jobs_[i].toMap();job["status"]="Queued";job["completed"]=0;job["error"]=QString{};jobs_[i]=job;}
    ++run_;running_=true;error_.clear();current_=-1;phase_=Phase::Idle;emit changed();next();
}
bool DownloadQueue::download(const QString& provider,const QVariantList& sets,int count,const QVariantMap& filters,bool startAfter){
    if(running_)return false;
    // Finished jobs belong in Saved sets; keep only the new selection in this run.
    jobs_.clear();current_=-1;
    const int first=jobs_.size();
    if(!add(provider,sets,count,true,filters))return false;
    QList<int> indices;for(int i=first;i<jobs_.size();++i)indices.append(i);
    startSelected(indices,startAfter&&indices.size()==1?indices.first():-1);return true;
}
void DownloadQueue::retry(int index){
    if(index<0||index>=jobs_.size()||running_)return;
    startSelected({index});
}
bool DownloadQueue::practice(const QString& provider,const QVariantMap& set,int count,const QVariantMap& filters){
    if(running_)return false;
    const auto before=jobs_.size();
    if(add(provider,{set},count,true,filters)){practiceJob_=before;start();return true;}
    // A queued selection can be promoted to practice without duplicating its job.
    if(!error_.isEmpty())return false;
    auto normalized=OnlineSources::normalizedFilters(provider,filters);
    if(provider=="quickposes"){
        const auto type=set.value("id").toString().section('_',0,0);
        if(!QStringList{"pose","feet","hands","face"}.contains(type))normalized.clear();
        else if(type=="hands"||type=="face")normalized.remove("clothing");
    }
    for(int i=0;i<jobs_.size();++i){const auto job=jobs_[i].toMap();
        if(job.value("status")=="Queued"&&job.value("provider")==provider&&job.value("setId")==set.value("id")&&job.value("separate").toBool()&&job.value("destination")==destination_&&job.value("filters").toMap()==normalized){practiceJob_=i;start();return true;}
    }
    return false;
}
void DownloadQueue::start(){
    selectedJobs_.clear();
    if(running_||jobs_.isEmpty())return;
    bool pending=false;
    for(auto& value:jobs_){auto job=value.toMap();if(job.value("status")!="Completed"){job["status"]="Queued";job["completed"]=0;job["error"]=QString{};value=job;pending=true;}}
    if(!pending)return;
    ++run_;running_=true;error_.clear();current_=-1;phase_=Phase::Idle;emit changed();next();
}
void DownloadQueue::next(){
    if(!running_)return;
    for(int i=current_+1;i<jobs_.size();++i){auto job=jobs_[i].toMap();if(job.value("status")!="Queued"||(!selectedJobs_.isEmpty()&&!selectedJobs_.contains(i)))continue;
        current_=i;phase_=Phase::Loading;job["status"]="Loading sources";jobs_[i]=job;emit changed();
        if(catalogSource_)worker_.copyCatalogFrom(*catalogSource_);
        worker_.selectProvider(job.value("provider").toString());return;
    }
    running_=false;phase_=Phase::Idle;current_=-1;emit changed();
}
void DownloadQueue::workerChanged(){
    if(current_<0||phase_==Phase::Cancelling)return;
    auto job=jobs_[current_].toMap();
    if(phase_==Phase::Loading&&!worker_.busy()){
        if(!worker_.error().isEmpty()||worker_.sets().isEmpty()){finishJob("Failed",worker_.error());return;}
        const auto id=job.value("setId").toString();bool valid=false;
        for(const auto& set:worker_.sets())if(set.toMap().value("id").toString()==id)valid=true;
        if(!valid){finishJob("Failed","This set is no longer available. Refresh the source.");return;}
        worker_.setDownloadRoot(job.value("destination").toString());
        phase_=Phase::Importing;job["status"]="Downloading images";jobs_[current_]=job;emit changed();const auto run=run_;
        QTimer::singleShot(0,this,[this,run,id]{if(running_&&run==run_&&current_>=0){const auto job=jobs_[current_].toMap();worker_.importImages(id,job.value("count").toInt(),job.value("separate").toBool(),job.value("filters").toMap());}});
    }else if(phase_==Phase::Importing){
        job["completed"]=worker_.completed();job["target"]=worker_.requested();jobs_[current_]=job;emit changed();
        if(!worker_.busy()&&!worker_.importing()&&worker_.completed()==0)finishJob("Failed",worker_.error());
    }
}
void DownloadQueue::finishJob(const QString& status,const QString& error){
    if(status=="Failed"&&current_==practiceJob_)practiceJob_=-1;
    if(current_<0)return;auto job=jobs_[current_].toMap();job["status"]=status;job["error"]=error;jobs_[current_]=job;if(!error.isEmpty())error_=error;
    phase_=Phase::Idle;emit changed();const auto run=run_;QTimer::singleShot(0,this,[this,run]{if(running_&&run==run_)next();});
}
void DownloadQueue::cancel(){
    if(!running_)return;
    practiceJob_=-1;
    ++run_;running_=false;phase_=Phase::Cancelling;worker_.cancel();
    if(current_>=0){auto job=jobs_[current_].toMap();job["status"]="Stopped";job["completed"]=worker_.completed();jobs_[current_]=job;}
    phase_=Phase::Idle;emit changed();
}
