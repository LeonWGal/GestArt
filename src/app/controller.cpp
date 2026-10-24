#include "controller.h"
#include "ui/themes.h"
#include "media/collections.h"
#include <QEvent>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QClipboard>
#include <QGuiApplication>
#include <QStyleHints>
#include <QImageReader>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent>
#include <QTransform>
#include <QCryptographicHash>
#include <cmath>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

Controller::Controller(QString directory,ImageProvider* images,Localization* i18n,QObject* parent)
    :QObject(parent),dataDirectory_(std::move(directory)),model_(this),filter_(this),scanner_(this),
    images_(images),i18n_(i18n),store_(dataDirectory_,this),settings_(loadPreferences(dataDirectory_)){
    const auto refreshOnline=[this]{
        if(onlineRefreshPending_&&!active()&&page_!="studio"){
            onlineRefreshPending_=false;QTimer::singleShot(0,this,[this]{if(!active())rescanLibrary();else onlineRefreshPending_=true;});
        }
    };
    connect(this,&Controller::sessionChanged,this,refreshOnline,Qt::QueuedConnection);
    connect(this,&Controller::navigationChanged,this,refreshOnline,Qt::QueuedConnection);
    clock_.start();filter_.setSourceModel(&model_);filter_.setFilterRole(CatalogModel::NameRole);
    actionPool_.setMaxThreadCount(1);metadataPool_.setMaxThreadCount(1);indexPool_.setMaxThreadCount(1);
    filter_.setFilterCaseSensitivity(Qt::CaseInsensitive);
    i18n_->setLanguage(settings_.value("language").toString());
    settings_["language"]=i18n_->language();
    connect(i18n_,&Localization::changed,this,[this]{emit settingsChanged();emit sessionChanged();});
    connect(QGuiApplication::styleHints(),&QStyleHints::colorSchemeChanged,this,[this]{emit settingsChanged();});
    connect(&scanner_,&CatalogScanner::batch,this,[this](quint64 generation,const QVector<MediaEntry>& batch){
        if(generation!=generation_)return;model_.append(batch);emit catalogChanged();
    });
    connect(&scanner_,&CatalogScanner::finished,this,[this](quint64 generation,int count,const QStringList& warnings){
        if(generation!=generation_)return;invalid_.clear();indexCollections();
        if(count==0)notify(i18n_->s("No images found in selected folder and subfolders"));
        else if(currentPath_.isEmpty())request(model_.paths().first(),Presentation::Preview);
        if(!warnings.isEmpty())notify(i18n_->s("Unavailable files")+": "+QString::number(warnings.size()));
    });
    connect(&store_,&Store::completedPaths,this,[this](const QStringList& paths){model_.setCompleted(QSet<QString>(paths.begin(),paths.end())+ephemeralCompleted_);recountCompleted();refreshCollections(false);});
    connect(&store_,&Store::snapshot,this,[this](QVariantList history,QStringList seen,QVariantMap stats){
        history_=std::move(history);stats_=std::move(stats);
        if(!timelineLoaded_){
            for(auto it=history_.crbegin();it!=history_.crend();++it){const auto event=it->toMap();const QString kind=event.value("kind").toString();
                if(kind=="shown" || kind=="browsed" || kind=="legacy")liveHistory_.append(event.value("path").toString());}
            historyIndex_=liveHistory_.size()-1;timelineLoaded_=true;emit sessionChanged();
        }
        seen_=QSet<QString>(seen.begin(),seen.end());historyReady_=true;emit historyChanged();
        if(!active())rebuildQueue();
        requestAnalytics();
    });
    connect(&store_,&Store::error,this,[this](const QString& e){
        if(lastStorageError_==e)return;lastStorageError_=e;qWarning().noquote()<<"Storage:"<<e;
        const QString error=e.toLower();
        const char* message=error.contains("full")?"Free disk space to save history, then restart the app."
            :error.contains("locked")||error.contains("busy")?"History is busy. Close another app using this profile and restart."
            :"Cannot save data. Check write access to the data folder, then restart the app.";
        notify(i18n_->s(message));
    });
    connect(&store_,&Store::healthChanged,this,[this](bool healthy){const bool readyBefore=historyReady_;if(healthy)lastStorageError_.clear();else historyReady_=true;if(historyHealthy_!=healthy||readyBefore!=historyReady_){historyHealthy_=healthy;emit historyChanged();}});
    connect(&store_,&Store::recovered,this,[this](int count,const QString& backup){notify(i18n_->s("History recovered")+": "+QString::number(count)+". "+i18n_->s("Original database backed up")+": "+backup);});
    connect(&store_,&Store::analyticsReady,this,[this](quint64 request,const QVariantMap& values){if(request==analyticsRequest_){practice_=values;emit practiceChanged();}});
    connect(&store_,&Store::imported,this,[this](int count){notify(i18n_->s("Imported images")+": "+QString::number(count));});
    tick_.setInterval(100);tick_.setTimerType(Qt::PreciseTimer);connect(&tick_,&QTimer::timeout,this,&Controller::updateClock);
    cueTimer_.setSingleShot(true);cueTimer_.setTimerType(Qt::PreciseTimer);
    connect(&cueTimer_,&QTimer::timeout,this,&Controller::updateClock);
    connect(this,&Controller::sessionChanged,this,&Controller::scheduleCue);
    progressTimer_.setInterval(16);progressTimer_.setTimerType(Qt::PreciseTimer);
    connect(&progressTimer_,&QTimer::timeout,this,&Controller::visualProgressChanged);
    connect(this,&Controller::sessionChanged,this,&Controller::updateProgressTimer);
    connect(this,&Controller::settingsChanged,this,&Controller::updateProgressTimer);
    dateTimer_.setInterval(60000);connect(&dateTimer_,&QTimer::timeout,this,[this]{if(analyticsDate_!=QDate::currentDate()){analyticsDate_=QDate::currentDate();resetTrackerMonth();}});dateTimer_.start();
    toastTimer_.setSingleShot(true);toastTimer_.setInterval(5000);
    connect(&toastTimer_,&QTimer::timeout,this,[this]{toast_.clear();emit toastChanged();});
    sound_.setSource(QUrl("qrc:/assets/countdown-beat.wav"));
    endSound_.setSource(QUrl("qrc:/assets/countdown-end.wav"));
    sound_.setVolume(settings_.value("timer_volume").toDouble()/100.0);
    endSound_.setVolume(sound_.volume());
    collections_=settings_.value("collections").toList();
    if(collections_.isEmpty()&&!settings_.value("collections_initialized").toBool()){
        const QStringList selected=settings_.value("source").toStringList();
        appendSources(selected);
        for(const auto& recent:recentSources())appendSources(recent.toMap().value("paths").toStringList());
        for(auto& value:collections_){auto item=value.toMap();bool enabled=false;for(const auto& path:item.value("paths").toStringList())if(selected.contains(path))enabled=true;item["selected"]=enabled;value=item;}
    }
    settings_["collections_initialized"]=true;settings_["collections"]=collections_;persist();
    refreshCollections();
    if(!collections_.isEmpty())QTimer::singleShot(0,this,&Controller::rescanLibrary);
}
Controller::~Controller(){scanner_.cancel();if(indexCancel_)indexCancel_->store(true);actionPool_.waitForDone();metadataPool_.waitForDone();indexPool_.waitForDone();persist();}
void Controller::setWindow(QQuickWindow* window){
    if(window_)window_->removeEventFilter(this);
    window->installEventFilter(this);
    window_=window;
    connect(window,&QQuickWindow::sceneGraphInitialized,this,&Controller::rendererChanged,Qt::QueuedConnection);
    connect(window,&QQuickWindow::sceneGraphInvalidated,this,&Controller::rendererChanged,Qt::QueuedConnection);
    emit rendererChanged();
    connect(window,&QQuickWindow::frameSwapped,this,&Controller::framePresented,Qt::QueuedConnection);
    windowPinApplied_=-1;updateWindowPin();
    connect(window,&QWindow::visibilityChanged,this,[this]{windowPinApplied_=-1;updateWindowPin();},Qt::QueuedConnection);
}
void Controller::updateWindowPin(){
    if(!window_)return;const bool top=settings_.value("always_on_top").toBool();
    if(windowPinApplied_==int(top))return;
#ifdef Q_OS_WIN
    // Change only z-order: keep the native title bar, size, position and activation.
    SetWindowPos(reinterpret_cast<HWND>(window_->winId()),top?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
#else
    const auto geometry=window_->geometry();const bool visible=window_->isVisible();
    window_->setFlag(Qt::WindowStaysOnTopHint,top);
    if(window_->geometry()!=geometry)window_->setGeometry(geometry);
    if(visible&&!window_->isVisible())window_->show();
#endif
    windowPinApplied_=int(top);
}
bool Controller::beginWindowMove(){return window_ && window_->visibility()!=QWindow::FullScreen && window_->startSystemMove();}
void Controller::setPage(const QString& requestedPage){
    const auto page=requestedPage=="statistics"||requestedPage=="tracker"?QString("practice"):requestedPage;
    if(!QStringList{"home","studio","web","library","practice","history","sites"}.contains(page))return;
    if(page_=="studio" && page!="studio"){session_.pauseForNavigation(now());stopSounds();}
    else if(page_!="studio" && page=="studio")session_.resumeFromNavigation(now());
    if(page_==page)return;
    closePreview();page_=page;emit navigationChanged();if(page=="history")store_.refresh();
    if(page=="home"||page=="practice")requestAnalytics();
    emit sessionChanged();
}
void Controller::requestAnalytics(){store_.analytics(++analyticsRequest_,statsPeriod_,trackerMonth_,settings_.value("daily_goal_minutes").toInt(),settings_.value("daily_goal_studies").toInt(),settings_.value("daily_goal_mode").toString());}
void Controller::setStatsPeriod(int days){if(days!=0&&days!=7&&days!=30&&days!=90)return;statsPeriod_=days;requestAnalytics();emit practiceChanged();}
void Controller::shiftTrackerMonth(int delta){const QDate current=QDate::currentDate();const QDate first(current.year(),current.month(),1);trackerMonth_=std::clamp(QDate(trackerMonth_.year(),trackerMonth_.month(),1).addMonths(std::clamp(delta,-1,1)),first.addMonths(-120),first);requestAnalytics();}
void Controller::resetTrackerMonth(){trackerMonth_=QDate::currentDate();requestAnalytics();}
void Controller::setPresetDuration(int seconds){if(!QList<int>{30,60,120,300}.contains(seconds))return;settings_["display_time"]=seconds;settings_["unlimited_time"]=false;persist();emit settingsChanged();}
void Controller::setPresetCount(int count){if(!QList<int>{5,10,20,30}.contains(count))return;settings_["num_images"]=count;settings_["unlimited_images"]=false;persist();emit settingsChanged();}
bool Controller::dark()const{
    if(settings_.value("theme_follow_system").toBool())return QGuiApplication::styleHints()->colorScheme()!=Qt::ColorScheme::Light;
    return settings_.value("theme_base").toString()!="white";
}
QString Controller::accent()const{
    return palette().value("accent").toString();
}
QVariantMap Controller::themeOptions()const{return themeCatalog();}
QVariantMap Controller::palette()const{return themePalette(settings_,dark());}
bool Controller::eventFilter(QObject* watched,QEvent* event){
    if(watched==window_ && (event->type()==QEvent::Enter || event->type()==QEvent::Leave)){
        const bool inside=event->type()==QEvent::Enter;
        if(pointerInsideWindow_!=inside){pointerInsideWindow_=inside;emit pointerInsideChanged();}
    }
    return QObject::eventFilter(watched,event);
}
QString Controller::state()const{
    using gestart::State;
    switch(session_.state()){
    case State::Idle:return "idle";case State::Loading:return "loading";case State::Running:return "running";
    case State::Paused:return "paused";case State::Review:return "review";case State::Break:return "break";
    case State::Completed:return "completed";
    }return "idle";
}
QString Controller::timerText()const{
    if(session_.state()==gestart::State::Idle)return "—";
    if(session_.state()==gestart::State::Completed)return i18n_->s("Session completed");
    const auto ms=session_.remaining(now());
    if(ms==0 && ((session_.state()==gestart::State::Break && session_.settings().breakMs==0)
        || (session_.state()!=gestart::State::Break && session_.settings().durationMs==0)))return QString::fromUtf8("∞");
    const auto sec=static_cast<int>((ms+999)/1000);
    return QString("%1:%2").arg(sec/60,2,10,QChar('0')).arg(sec%60,2,10,QChar('0'));
}
double Controller::progress()const{
    const auto duration=session_.state()==gestart::State::Break?session_.settings().breakMs:session_.settings().durationMs;
    if(duration==0)return 0;return std::clamp(1.0-double(session_.remaining(now()))/double(duration),0.0,1.0);
}
bool Controller::active()const{return session_.state()!=gestart::State::Idle && session_.state()!=gestart::State::Completed;}
void Controller::persist(){store_.savePreferences(settings_);}
void Controller::notify(const QString& text){toast_=text;emit toastChanged();toastTimer_.start();}
void Controller::loadSource(const QVariant& source){
    QString path=source.toString();const QUrl url(path);if(url.isLocalFile())path=url.toLocalFile();
    if(path.isEmpty())return;scan({QDir::cleanPath(QFileInfo(path).absoluteFilePath())});
}
void Controller::loadFiles(const QList<QUrl>& urls){QStringList paths;for(const auto& u:urls)if(u.isLocalFile())paths.append(u.toLocalFile());if(!paths.isEmpty())scan(paths);}
void Controller::dropUrls(const QList<QUrl>& urls){loadFiles(urls);}
void Controller::loadPaths(QStringList paths){scan(std::move(paths));}
void Controller::addFolder(const QUrl& folder){if(folder.isLocalFile())scan({folder.toLocalFile()});}
void Controller::startDownloadedSession(const QString& folder){
    if(active()||!QFileInfo(folder).isDir())return;
    const auto id=makeCollection({folder},"folder",QString{}).value("id");bool found=false;
    for(const auto& value:collections_)if(value.toMap().value("id")==id)found=true;
    if(!found){appendSources({folder});settings_["collections"]=collections_;persist();}
    for(auto& value:collections_){auto item=value.toMap();item["selected"]=item.value("id")==id;value=item;}
    onlineRefreshPending_=false;rescanLibrary();pendingDownloadedSession_=folder;
    setPage("studio");
}
void Controller::importOnlineCollection(const QString& folder,const QString& name){
    if(!QDir(folder).exists())return;
    appendSources({folder});const auto id=makeCollection({folder},"folder",name).value("id");
    for(auto& value:collections_){auto item=value.toMap();if(item.value("id")==id){item["name"]=name;value=item;break;}}
    settings_["collections"]=collections_;persist();
    if(active()||page_=="studio"){
        onlineRefreshPending_=true;refreshCollections(false);
        notify(i18n_->s("References downloaded. They will be added when you leave the session."));
    }else rescanLibrary();
}
void Controller::appendSources(const QStringList& paths){
    QStringList files;
    for(const auto& source:paths){if(source.trimmed().isEmpty())continue;const QFileInfo info(source);const QString path=info.absoluteFilePath();
        if(info.isDir()){auto item=makeCollection({path},"folder",info.fileName().isEmpty()?path:info.fileName());bool found=false;
            for(auto& existing:collections_)if(existing.toMap().value("id")==item.value("id")){auto value=existing.toMap();value["selected"]=true;existing=value;found=true;break;}
            if(!found)collections_.append(item);
        }else files.append(path);
    }
    if(!files.isEmpty()){auto item=makeCollection(files,"files",i18n_->s("Selected images"));bool found=false;
        for(auto& existing:collections_)if(existing.toMap().value("id")==item.value("id")){auto value=existing.toMap();value["selected"]=true;existing=value;found=true;break;}
        if(!found)collections_.append(item);
    }
}
void Controller::scan(QStringList paths){
    paths.removeAll(QString{});paths.removeDuplicates();if(paths.isEmpty())return;
    appendSources(paths);rescanLibrary();
}
void Controller::rescanLibrary(){
    pendingDownloadedSession_.clear();stopSession();sources_=collectionPaths(collections_);sourceLabel_=collections_.size()==1?collections_.first().toMap().value("name").toString():i18n_->s("Collections")+" · "+QString::number(collections_.size());
    auto identityPaths=sources_;identityPaths.sort();
    sourceId_=sources_.size()==1?sources_.first():"files:"+QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(QJsonArray::fromStringList(identityPaths)).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
    ++indexRevision_;if(indexCancel_)indexCancel_->store(true);collectionMembers_.clear();pathCollections_.clear();collectionCompleted_.clear();finishedImageCount_=0;
    model_.clear();currentPath_.clear();currentImage_.clear();loading_=false;++requestId_;
    eligiblePaths_.clear();settings_["collections"]=collections_;settings_["source"]=sources_;
    scanning_=!sources_.isEmpty();scanner_.cancel();++generation_;
    if(scanning_)generation_=scanner_.scan(sources_);
    refreshCollections();
    persist();emit settingsChanged();emit catalogChanged();emit imageChanged();
}
void Controller::indexCollections(){
    const auto revision=++indexRevision_;const auto paths=model_.paths();const auto collections=collections_;
    if(indexCancel_)indexCancel_->store(true);indexCancel_=std::make_shared<std::atomic_bool>(false);const auto cancelled=indexCancel_;
    indexPool_.clear();
    indexPool_.start([this,revision,paths,collections,cancelled]{
        QHash<QString,QStringList> members,reverse;
        for(const auto& value:collections){const auto item=value.toMap();const auto id=item.value("id").toString();
            members.insert(id,{});
            const auto roots=item.value("paths").toStringList();
            for(const auto& path:paths){if(cancelled->load())return;if(pathInSources(path,roots)){members[id].append(path);reverse[path].append(id);}}}
        QMetaObject::invokeMethod(this,[this,revision,members=std::move(members),reverse=std::move(reverse)]{
            if(revision!=indexRevision_)return;
            collectionMembers_=members;pathCollections_=reverse;scanning_=false;recountCompleted();refreshCollections();
            setBrowseCollection(browseCollection_);rebuildQueue();emit catalogChanged();
            if(!pendingDownloadedSession_.isEmpty()){
                pendingDownloadedSession_.clear();
                QTimer::singleShot(0,this,[this]{if(!scanning_&&sessionImageCount()>0)startSession();else setPage("library");});
            }
        });
    });
}
void Controller::recountCompleted(){
    collectionCompleted_.clear();finishedImageCount_=0;
    for(auto it=pathCollections_.cbegin();it!=pathCollections_.cend();++it)if(model_.isCompleted(it.key())){
        ++finishedImageCount_;for(const auto& id:it.value())++collectionCompleted_[id];}
}
void Controller::completeCurrent(){
    if(!model_.isCompleted(currentPath_)){
        ++finishedImageCount_;for(const auto& id:pathCollections_.value(currentPath_))++collectionCompleted_[id];
        model_.markCompleted(currentPath_);
    }
    ephemeralCompleted_.insert(currentPath_);refreshCollections(false);
}
void Controller::refreshCollections(bool selectionChanged){
    sourceLabel_=collections_.size()==1?collections_.first().toMap().value("name").toString():i18n_->s("Collections")+" · "+QString::number(collections_.size());
    if(selectionChanged && !scanning_){
        QSet<QString> unique;eligiblePaths_.clear();
        for(const auto& value:collections_){const auto item=value.toMap();if(!item.value("selected").toBool())continue;
            for(const auto& path:collectionMembers_.value(item.value("id").toString()))if(!unique.contains(path)){unique.insert(path);eligiblePaths_.append(path);}}
    }
    collectionView_.clear();
    for(const auto& value:collections_){auto item=value.toMap();const auto id=item.value("id").toString();
        item["count"]=collectionMembers_.value(id).size();item["completed"]=collectionCompleted_.value(id);
        item["path"]=item.value("paths").toStringList().value(0);collectionView_.append(item);}
    emit collectionsChanged();
}
void Controller::setBrowseCollection(const QString& id){
    browseCollection_=collectionMembers_.contains(id)?id:QString{};
    const auto paths=collectionMembers_.value(browseCollection_);
    filter_.setAllowedPaths(QSet<QString>(paths.begin(),paths.end()),!browseCollection_.isEmpty());emit collectionsChanged();
}
void Controller::selectCollection(const QString& id,bool selected){
    for(auto& value:collections_){auto item=value.toMap();if(item.value("id").toString()==id){item["selected"]=selected;value=item;break;}}
    settings_["collections"]=collections_;refreshCollections();persist();
}
void Controller::selectAllCollections(bool selected){for(auto& value:collections_){auto item=value.toMap();item["selected"]=selected;value=item;}settings_["collections"]=collections_;refreshCollections();persist();}
void Controller::removeCollection(const QString& id){
    for(int index=0;index<collections_.size();++index)if(collections_[index].toMap().value("id").toString()==id){removedCollection_=collections_[index].toMap();removedCollectionIndex_=index;collections_.removeAt(index);rescanLibrary();notify(i18n_->s("Collection removed. Files stay on disk."));return;}
}
void Controller::undoCollectionRemoval(){if(removedCollection_.isEmpty())return;bool exists=false;for(const auto& value:collections_)if(value.toMap().value("id")==removedCollection_.value("id"))exists=true;if(!exists)collections_.insert(std::min(removedCollectionIndex_,int(collections_.size())),removedCollection_);removedCollection_.clear();rescanLibrary();notify(i18n_->s("Collection restored"));}
void Controller::renameCollection(const QString& id,const QString& name){if(name.trimmed().isEmpty())return;for(auto& value:collections_){auto item=value.toMap();if(item.value("id").toString()==id){item["name"]=name.trimmed().left(100);value=item;break;}}settings_["collections"]=collections_;refreshCollections(false);emit catalogChanged();persist();}
void Controller::cancelScan(){scanner_.cancel();++generation_;scanning_=true;indexCollections();emit catalogChanged();}
void Controller::rebuildQueue(bool newCycle){
    queue_.clear();queueIndex_=0;
    for(const auto& path:active()?sessionPaths_:eligiblePaths_)if(!invalid_.contains(path)
        && (newCycle || !settings_["save_history"].toBool() || !seen_.contains(path)))queue_.append(path);
    std::shuffle(queue_.begin(),queue_.end(),random_);
}
void Controller::chooseNext(){
    while(queueIndex_<queue_.size()){
        const QString path=queue_[queueIndex_++];if(invalid_.contains(path))continue;
        request(path,active()?Presentation::Live:Presentation::Preview);return;
    }
    rebuildQueue(true);
    if(queue_.isEmpty()){stopSession();currentPath_.clear();currentImage_.clear();loading_=false;emit imageChanged();notify(i18n_->s("No more valid images found in the folder."));return;}
    notify(i18n_->s("All images have been viewed. Starting over."));
    const QString path=queue_[queueIndex_++];request(path,active()?Presentation::Live:Presentation::Preview);
}
void Controller::request(const QString& path,Presentation role){
    if(path.isEmpty())return;
    presentation_=role;currentPath_=path;loading_=true;readyId_=0;++requestId_;
    // Keep the old frame's geometry until the replacement is ready.
    pendingPixelSize_={};
    const auto id=requestId_;
    metadataPool_.clear();metadataPool_.start([this,path,id]{const auto size=imageNaturalSize(path);
        QMetaObject::invokeMethod(this,[this,size,id]{if(id!=requestId_)return;pendingPixelSize_=size;if(!loading_||readyId_==id){imagePixelSize_=size;emit imageMetadataChanged();}});});
    sound_.stop();cueTimer_.stop();
    requestClock_.restart();currentImage_=imageSource(path,requestId_);
    emit imageChanged();emit sessionChanged();
}
void Controller::startSession(bool freeView){
    if(scanning_ || model_.rowCount()==0){notify(i18n_->s(scanning_?"Scanning":"No images to display. Select a folder."));return;}
    if(!historyReady_ && settings_["save_history"].toBool()){notify(i18n_->s("Loading history"));return;}
    if(eligiblePaths_.isEmpty()){notify(i18n_->s("Choose folders for the session"));return;}
    if(!historyHealthy_ && settings_["save_history"].toBool())notify(i18n_->s("Practice is available. History cannot be saved right now."));
    sessionPaths_=eligiblePaths_;const auto selected=collectionPaths(collections_,true);auto identity=selected;identity.sort();
    sourceId_=selected.size()==1?selected.first():"selection:"+QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(QJsonArray::fromStringList(identity)).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
    gestart::SessionSettings s;
    s.durationMs=(freeView || settings_["unlimited_time"].toBool())?0:settings_["display_time"].toLongLong()*1000;
    s.imageLimit=(freeView || settings_["unlimited_images"].toBool())?0:settings_["num_images"].toInt();
    s.breakEnabled=!freeView && settings_["use_break"].toBool();
    s.breakMs=settings_["unlimited_break"].toBool()?0:settings_["break_duration"].toLongLong()*1000;
    stopSounds();session_.begin(s);sessionPracticeMs_=0;resumeAfterReview_=false;liveAnchor_=-1;rebuildQueue();setPage("studio");chooseNext();
    lastTick_=now();tick_.start();emit sessionChanged();
}
void Controller::stopSession(){session_.stop();presentation_=Presentation::Preview;tick_.stop();stopSounds();liveAnchor_=-1;readyId_=0;resumeAfterReview_=false;emit sessionChanged();}
void Controller::togglePause(){
    if(session_.state()==gestart::State::Idle || session_.state()==gestart::State::Completed){startSession();return;}
    if(session_.state()==gestart::State::Review){
        if(liveAnchor_>=0 && liveAnchor_<liveHistory_.size()){historyIndex_=liveAnchor_;resumeAfterReview_=true;request(liveHistory_[historyIndex_],Presentation::Review);}
        return;
    }
    session_.togglePause(now());if(paused())stopSounds();emit sessionChanged();
}
void Controller::ready(quint64 id){if(id==requestId_){readyId_=id;if(pendingPixelSize_.isValid()&&imagePixelSize_!=pendingPixelSize_){imagePixelSize_=pendingPixelSize_;emit imageMetadataChanged();}}}
void Controller::framePresented(){
    if(readyId_==0 || readyId_!=requestId_ || !loading_ || page_!="studio")return;
    readyId_=0;loading_=false;timings_.append(requestClock_.elapsed());if(timings_.size()>2000)timings_.removeFirst();
    if(presentation_==Presentation::Live){
        session_.presented(now());countdown_.reset(session_.settings().durationMs);
        if(settings_["save_history"].toBool()){seen_.insert(currentPath_);store_.record(currentPath_,sourceId_,"shown");}
    }
    if(presentation_!=Presentation::Review){
        liveHistory_.append(currentPath_);if(liveHistory_.size()>2000)liveHistory_.removeFirst();historyIndex_=liveHistory_.size()-1;
        if(presentation_==Presentation::Live)liveAnchor_=historyIndex_;
        else if(settings_["save_history"].toBool()){seen_.insert(currentPath_);store_.record(currentPath_,QFileInfo(currentPath_).absolutePath(),"browsed");}
    }
    if(presentation_==Presentation::Review && resumeAfterReview_){resumeAfterReview_=false;session_.leaveReview(now());}
    emit imageChanged();emit sessionChanged();emit presented(currentPath_);
}
void Controller::loadError(quint64 id){
    if(id!=requestId_ || !loading_)return;invalid_.insert(currentPath_);loading_=false;
    loadErrors_.append(currentPath_);if(loadErrors_.size()>20)loadErrors_.removeFirst();
    notify(i18n_->s("Could not load image")+": "+fileName());
    if(presentation_==Presentation::Review){emit imageChanged();emit sessionChanged();return;}
    QTimer::singleShot(0,this,&Controller::chooseNext);
}
void Controller::advance(bool count){
    if(loading_ || page_!="studio" || session_.settingsOpen())return;
    if(session_.state()==gestart::State::Idle){chooseNext();return;}
    if(session_.state()==gestart::State::Review){next();return;}
    if(session_.state()!=gestart::State::Running && session_.state()!=gestart::State::Paused)return;
    count=count&&!freeView();
    const auto duration=session_.activeElapsed(now());
    const auto action=session_.advance(now(),count);
    if(count)sessionPracticeMs_+=duration;
    if(count){completeCurrent();}
    if(settings_["save_history"].toBool())store_.record(currentPath_,sourceId_,count?"completed":"skipped",count?int(std::min<qint64>(duration,2147483647)):0);
    handleAction(action);emit sessionChanged();
}
void Controller::handleAction(gestart::Action action){
    if(action==gestart::Action::NextImage)chooseNext();
    else if(action==gestart::Action::Restart){sessionPracticeMs_=0;liveAnchor_=-1;chooseNext();}
    if(session_.state()==gestart::State::Completed){tick_.stop();store_.refresh();}
    emit sessionChanged();
}
void Controller::next(){
    if(loading_)return;
    if(historyIndex_+1<liveHistory_.size()){
        ++historyIndex_;resumeAfterReview_=session_.state()==gestart::State::Review && historyIndex_==liveAnchor_;
        request(liveHistory_[historyIndex_],Presentation::Review);
        emit sessionChanged();return;
    }
    if(session_.state()==gestart::State::Completed){rebuildQueue();chooseNext();return;}
    if(session_.state()==gestart::State::Break){handleAction(session_.skipBreak());return;}
    advance(true);
}
void Controller::skip(){if(session_.state()==gestart::State::Break)handleAction(session_.skipBreak());else advance(false);}
void Controller::previous(){
    if(loading_ || historyIndex_<=0)return;
    session_.enterReview(now());stopSounds();resumeAfterReview_=false;--historyIndex_;request(liveHistory_[historyIndex_],Presentation::Review);emit sessionChanged();
}
void Controller::openMedia(const QString& path){
    if(path.isEmpty())return;
    // Snapshot the browsing order. Preview never touches the session queue or clock.
    previewPaths_.clear();QSet<QString> added;
    if(page_=="history"){
        for(const auto& event:history_){const auto entry=event.toMap().value("path").toString();
            if(!entry.isEmpty()&&!added.contains(entry)){added.insert(entry);previewPaths_.append(entry);}}
    }else{
        for(int row=0;row<filter_.rowCount();++row)
            previewPaths_.append(filter_.data(filter_.index(row,0),CatalogModel::PathRole).toString());
    }
    previewIndex_=previewPaths_.indexOf(path);
    if(previewIndex_<0){previewPaths_.prepend(path);previewIndex_=0;}
    previewPath_=path;++previewGeneration_;emit previewChanged();
}
void Controller::previewPresented(const QString& source){
    if(previewPath_.isEmpty()||source!=previewImage()||previewRecorded_==previewGeneration_)return;
    previewRecorded_=previewGeneration_;
    if(settings_.value("save_history").toBool())store_.record(previewPath_,QFileInfo(previewPath_).absolutePath(),"browsed");
}
void Controller::closePreview(){
    if(previewPath_.isEmpty())return;
    previewPath_.clear();previewPaths_.clear();previewIndex_=-1;emit previewChanged();if(page_=="history")store_.refresh();
}
void Controller::stepPreview(int delta){
    if(previewPath_.isEmpty())return;
    const int next=std::clamp(previewIndex_+std::clamp(delta,-1,1),0,int(previewPaths_.size())-1);
    if(next==previewIndex_)return;
    previewIndex_=next;previewPath_=previewPaths_[next];++previewGeneration_;emit previewChanged();
}
void Controller::stopSounds(){cueTimer_.stop();sound_.stop();endSound_.stop();}
void Controller::updateProgressTimer(){
    const bool moving=page_=="studio"&&!loading_&&!session_.settingsOpen()&&!paused()&&
        ((session_.state()==gestart::State::Running&&session_.settings().durationMs>0)||
         (session_.state()==gestart::State::Break&&session_.settings().breakMs>0));
    if(moving){if(!progressTimer_.isActive())progressTimer_.start();}else progressTimer_.stop();
    emit visualProgressChanged();
}
void Controller::scheduleCue(){
    cueTimer_.stop();
    if(page_!="studio" || session_.state()!=gestart::State::Running || session_.settingsOpen() || loading_)return;
    const auto delay=countdown_.nextDelay(session_.remaining(now()));
    if(delay>=0)cueTimer_.start(int(delay));
}
void Controller::updateClock(){
    const qint64 current=now();
    if(lastTick_>0 && current-lastTick_>30000){session_.pauseAfterSleep(lastTick_);stopSounds();notify(i18n_->s("Paused after inactivity"));}
    lastTick_=current;
    if(session_.state()==gestart::State::Running && !loading_){
        const int cue=countdown_.poll(session_.remaining(current));
        if(cue>=0){
            auto& effect=cue==0?endSound_:sound_;
            if(effect.volume()>0 && effect.status()==QSoundEffect::Ready)effect.play();
            emit countdownCue(cue);
        }
    }
    const auto before=session_.completed();const auto action=session_.tick(current);
    if(session_.completed()>before){sessionPracticeMs_+=session_.settings().durationMs;completeCurrent();
        if(settings_["save_history"].toBool())store_.record(currentPath_,sourceId_,"completed",int(session_.settings().durationMs));}
    if(action!=gestart::Action::None)handleAction(action);
    if(session_.state()==gestart::State::Completed){tick_.stop();store_.refresh();}
    emit sessionChanged();
}
void Controller::openSettings(){session_.pauseForSettings(now());stopSounds();emit sessionChanged();emit settingsRequested();}
void Controller::closeSettings(){session_.resumeFromSettings(now());emit sessionChanged();}
void Controller::applySettings(const QVariantMap& values){
    const auto oldGoalMinutes=settings_.value("daily_goal_minutes"),oldGoalStudies=settings_.value("daily_goal_studies"),oldGoalMode=settings_.value("daily_goal_mode");
    for(auto it=settings_.begin();it!=settings_.end();++it)if(values.contains(it.key()) && it.key()!="source" && it.key()!="recent_sources" && it.key()!="collections" && it.key()!="collections_initialized")it.value()=values.value(it.key());
    settings_["display_time"]=std::clamp(settings_["display_time"].toInt(),1,86400);
    settings_["num_images"]=std::clamp(settings_["num_images"].toInt(),1,100000);
    settings_["break_duration"]=std::clamp(settings_["break_duration"].toInt(),1,86400);
    settings_["timer_volume"]=std::clamp(settings_["timer_volume"].toInt(),0,100);
    settings_["daily_goal_minutes"]=std::clamp(settings_["daily_goal_minutes"].toInt(),1,240);
    settings_["daily_goal_studies"]=std::clamp(settings_["daily_goal_studies"].toInt(),1,1000);
    settings_["session_panel_opacity"]=std::clamp(settings_["session_panel_opacity"].toInt(),70,95);
    if(!QStringList{"time","studies","both"}.contains(settings_["daily_goal_mode"].toString()))settings_["daily_goal_mode"]="time";
    normalizeTheme(settings_,values.contains("theme") && !values.contains("theme_base"));
    i18n_->setLanguage(settings_["language"].toString());settings_["language"]=i18n_->language();sound_.setVolume(settings_["timer_volume"].toDouble()/100.0);
    endSound_.setVolume(sound_.volume());
    updateWindowPin();
    persist();emit settingsChanged();
    if(oldGoalMinutes!=settings_.value("daily_goal_minutes")||oldGoalStudies!=settings_.value("daily_goal_studies")||oldGoalMode!=settings_.value("daily_goal_mode"))requestAnalytics();
    notify(i18n_->s("Settings saved"));
}
void Controller::setSearch(const QString& query){filter_.setFilterFixedString(query);}
void Controller::setEffect(const QString& effect){
    if(effect=="bw")bw_=!bw_;else if(effect=="horizontal")flipH_=!flipH_;
    else if(effect=="vertical")flipV_=!flipV_;else if(effect=="rotate")rotation_=(rotation_+90)%360;
    else if(effect=="grid")grid_=!grid_;emit effectsChanged();
}
void Controller::resetEffects(){bw_=false;flipH_=false;flipV_=false;rotation_=0;emit effectsChanged();}
void Controller::copyImage(bool effects){
    if(currentPath_.isEmpty())return;
    if(exportBusy_){notify(i18n_->s("Copying image…"));return;}exportBusy_=true;
    const QString path=currentPath_;const bool gray=effects&&bw_,h=effects&&flipH_,v=effects&&flipV_;const int angle=effects?rotation_:0;
    actionPool_.start([this,path,gray,h,v,angle]{
        QImage image=decodeImage(path,{8192,8192});
        if(gray)image=grayscaleImage(image);
        if(h||v)image=image.mirrored(h,v);if(angle)image=image.transformed(QTransform().rotate(angle));
        QMetaObject::invokeMethod(this,[this,image]{exportBusy_=false;if(image.isNull()){notify(i18n_->s("Could not load image"));return;}
            QGuiApplication::clipboard()->setImage(image);notify(i18n_->s("Image copied to clipboard"));});
    });
}
void Controller::openCurrentFile(){if(!currentPath_.isEmpty()&&!QDesktopServices::openUrl(QUrl::fromLocalFile(currentPath_)))notify(i18n_->s("Cannot open file"));}
void Controller::openDataFolder(){if(!QDesktopServices::openUrl(QUrl::fromLocalFile(dataDirectory_)))notify(i18n_->s("Cannot open data folder"));}
void Controller::trashCurrent(){
    if(currentPath_.isEmpty())return;const QString path=currentPath_;
    actionPool_.start([this,path]{
        const bool moved=QFile::moveToTrash(path);
        QMetaObject::invokeMethod(this,[this,path,moved]{
            if(!moved){notify(i18n_->s("Failed to move file to trash"));return;}
            invalid_.insert(path);if(model_.isCompleted(path)){--finishedImageCount_;for(const auto& id:pathCollections_.value(path))--collectionCompleted_[id];}
            for(const auto& id:pathCollections_.take(path))collectionMembers_[id].removeAll(path);
            model_.removePath(path);images_->cache()->invalidate(path);setBrowseCollection(browseCollection_);
            if(currentPath_==path){if(active())session_.requestImage();chooseNext();}
            refreshCollections();emit catalogChanged();notify(i18n_->s("Moved to trash"));
        });
    });
}
void Controller::clearHistory(bool all){
    const auto roots=collectionPaths(collections_,true);
    if(!all && roots.isEmpty()){notify(i18n_->s("Choose folders for the session"));return;}
    if(all){ephemeralCompleted_.clear();store_.clearHistory({});}
    else{for(auto it=ephemeralCompleted_.begin();it!=ephemeralCompleted_.end();)if(pathInSources(*it,roots))it=ephemeralCompleted_.erase(it);else++it;store_.clearSources(roots);}
    // Clearing history must also clear the arrows' in-memory history.
    QStringList retained;for(const auto& path:liveHistory_)if(!all&&!pathInSources(path,roots))retained.append(path);
    liveHistory_=retained;historyIndex_=liveHistory_.size()-1;liveAnchor_=-1;resumeAfterReview_=false;
    if(session_.state()==gestart::State::Review)session_.leaveReview(now());
    emit sessionChanged();images_->cache()->clear();notify(i18n_->s("History cleared"));
}
void Controller::importLegacy(){
    const QString directory=QDir::homePath()+"/.gestart";store_.importLegacy(directory);
    const auto legacy=loadPreferences(directory);
    if(QFile::exists(directory+"/settings.json"))applySettings(legacy);
}
void Controller::toggleFullscreen(){if(!window_)return;if(window_->visibility()==QWindow::FullScreen)window_->showNormal();else window_->showFullScreen();}
void Controller::togglePin(){settings_["always_on_top"]=!settings_["always_on_top"].toBool();updateWindowPin();persist();emit settingsChanged();}
QVariantMap Controller::diagnostics()const{
    auto timings=timings_;std::sort(timings.begin(),timings.end());
    QVariantMap d{{"catalogCount",model_.rowCount()},{"cacheBytes",images_->cache()->bytes()},
        {"presentations",timings.size()},{"invalidFiles",invalid_.size()},{"state",state()},{"completed",completed()},
        {"graphicsApi",window_?int(window_->rendererInterface()->graphicsApi()):0},{"loadErrors",loadErrors_},
        {"collections",collectionView_},{"selectedImageCount",eligiblePaths_.size()},{"finishedImageCount",finishedImageCount_},
        {"page",page_},{"practice",practice_},{"sessionPracticeMs",sessionPracticeMs_},
        {"countdownSoundReady",sound_.isLoaded()},{"countdownEndReady",endSound_.isLoaded()}};
    d["historyHealthy"]=historyHealthy_;d["browseHistorySize"]=liveHistory_.size();d["browseHistoryIndex"]=historyIndex_;
    if(!timings.isEmpty()){d["p50Ms"]=timings[timings.size()/2];d["p95Ms"]=timings[std::min<qsizetype>(timings.size()-1,qsizetype(timings.size()*0.95))];}
    return d;
}

