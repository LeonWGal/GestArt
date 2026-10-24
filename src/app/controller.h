#pragma once
#include "domain/session.h"
#include "domain/countdown.h"
#include "media/catalog.h"
#include "media/image_provider.h"
#include "storage/store.h"
#include "ui/localization.h"
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QTimer>
#include <QQuickWindow>
#include <QSoundEffect>
#include <QFileInfo>
#include <random>

class Controller : public QObject {
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* catalog READ catalog CONSTANT)
    Q_PROPERTY(QString imageFilter READ imageFilter NOTIFY settingsChanged)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(bool softwareRenderer READ softwareRenderer NOTIFY rendererChanged)
    Q_PROPERTY(QVariantMap settings READ settings NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList recentSources READ recentSources NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList collections READ collections NOTIFY collectionsChanged)
    Q_PROPERTY(int sessionImageCount READ sessionImageCount NOTIFY collectionsChanged)
    Q_PROPERTY(int finishedImageCount READ finishedImageCount NOTIFY collectionsChanged)
    Q_PROPERTY(bool canUndoCollectionRemoval READ canUndoCollectionRemoval NOTIFY collectionsChanged)
    Q_PROPERTY(QString browseCollection READ browseCollection WRITE setBrowseCollection NOTIFY collectionsChanged)
    Q_PROPERTY(QString previewImage READ previewImage NOTIFY previewChanged)
    Q_PROPERTY(QString previewPath READ previewPath NOTIFY previewChanged)
    Q_PROPERTY(int previewIndex READ previewIndex NOTIFY previewChanged)
    Q_PROPERTY(int previewCount READ previewCount NOTIFY previewChanged)
    Q_PROPERTY(QString page READ page WRITE setPage NOTIFY navigationChanged)
    Q_PROPERTY(QString currentSource READ currentSource NOTIFY catalogChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY catalogChanged)
    Q_PROPERTY(int imageCount READ imageCount NOTIFY catalogChanged)
    Q_PROPERTY(QString currentImage READ currentImage NOTIFY imageChanged)
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY imageChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY imageChanged)
    Q_PROPERTY(QSize imagePixelSize READ imagePixelSize NOTIFY imageMetadataChanged)
    Q_PROPERTY(quint64 requestId READ requestId NOTIFY imageChanged)
    Q_PROPERTY(QString state READ state NOTIFY sessionChanged)
    Q_PROPERTY(QString timerText READ timerText NOTIFY sessionChanged)
    Q_PROPERTY(double progress READ progress NOTIFY sessionChanged)
    Q_PROPERTY(double visualProgress READ progress NOTIFY visualProgressChanged)
    Q_PROPERTY(qint64 remainingMs READ remainingMs NOTIFY sessionChanged)
    Q_PROPERTY(qint64 sessionDurationMs READ sessionDurationMs NOTIFY sessionChanged)
    Q_PROPERTY(int completed READ completed NOTIFY sessionChanged)
    Q_PROPERTY(int ordinal READ ordinal NOTIFY sessionChanged)
    Q_PROPERTY(int sessionLimit READ sessionLimit NOTIFY sessionChanged)
    Q_PROPERTY(bool active READ active NOTIFY sessionChanged)
    Q_PROPERTY(bool freeView READ freeView NOTIFY sessionChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY sessionChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY imageChanged)
    Q_PROPERTY(bool canPrevious READ canPrevious NOTIFY sessionChanged)
    Q_PROPERTY(bool canNext READ canNext NOTIFY sessionChanged)
    Q_PROPERTY(bool historyHealthy READ historyHealthy NOTIFY historyChanged)
    Q_PROPERTY(bool historyLoading READ historyLoading NOTIFY historyChanged)
    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)
    Q_PROPERTY(QVariantMap stats READ stats NOTIFY historyChanged)
    Q_PROPERTY(QVariantMap practice READ practice NOTIFY practiceChanged)
    Q_PROPERTY(int statsPeriod READ statsPeriod WRITE setStatsPeriod NOTIFY practiceChanged)
    Q_PROPERTY(qint64 sessionPracticeMs READ sessionPracticeMs NOTIFY sessionChanged)
    Q_PROPERTY(QString toast READ toast NOTIFY toastChanged)
    Q_PROPERTY(bool dark READ dark NOTIFY settingsChanged)
    Q_PROPERTY(QString accent READ accent NOTIFY settingsChanged)
    Q_PROPERTY(QVariantMap themeCatalog READ themeOptions CONSTANT)
    Q_PROPERTY(QVariantMap themePalette READ palette NOTIFY settingsChanged)
    Q_PROPERTY(bool pointerInsideWindow READ pointerInsideWindow NOTIFY pointerInsideChanged)
    Q_PROPERTY(bool bw READ bw NOTIFY effectsChanged)
    Q_PROPERTY(bool flipH READ flipH NOTIFY effectsChanged)
    Q_PROPERTY(bool flipV READ flipV NOTIFY effectsChanged)
    Q_PROPERTY(int rotation READ rotation NOTIFY effectsChanged)
    Q_PROPERTY(bool grid READ grid NOTIFY effectsChanged)
public:
    Controller(QString dataDirectory,ImageProvider* images,Localization* i18n,QObject* parent=nullptr);
    ~Controller() override;
    void setWindow(QQuickWindow* window);
    QAbstractItemModel* catalog(){return &filter_;}
    QString appVersion()const{return QCoreApplication::applicationVersion();}
    QString imageFilter()const{return i18n_->s("Images")+" (*."+CatalogScanner::extensions().join(" *.")+")";}
    bool softwareRenderer()const{return window_ && window_->rendererInterface()->graphicsApi()==QSGRendererInterface::Software;}
    QVariantMap settings()const{return settings_;}
    QVariantList recentSources()const{return settings_.value("recent_sources").toList();}
    QVariantList collections()const{return collectionView_;}
    int sessionImageCount()const{return eligiblePaths_.size();}
    int finishedImageCount()const{return finishedImageCount_;}
    bool canUndoCollectionRemoval()const{return !removedCollection_.isEmpty();}
    QString browseCollection()const{return browseCollection_;}
    void setBrowseCollection(const QString& id);
    Q_INVOKABLE void setCompletedFilter(bool enabled){filter_.setCompletedOnly(enabled);}
    QString page()const{return page_;}
    void setPage(const QString& page);
    QString currentSource()const{return sourceLabel_;}
    bool scanning()const{return scanning_;}
    int imageCount()const{return model_.rowCount();}
    QString currentImage()const{return currentImage_;}
    QString currentPath()const{return currentPath_;}
    QString fileName()const{return QFileInfo(currentPath_).fileName();}
    QSize imagePixelSize()const{return imagePixelSize_;}
    quint64 requestId()const{return requestId_;}
    QString state()const;
    QString timerText()const;
    double progress()const;
    qint64 remainingMs()const{return session_.remaining(now());}
    qint64 sessionDurationMs()const{return session_.settings().durationMs;}
    int completed()const{return session_.completed();}
    int ordinal()const{return session_.ordinal();}
    int sessionLimit()const{return session_.settings().imageLimit;}
    bool active()const;
    bool freeView()const{return session_.settings().durationMs==0 && session_.settings().imageLimit==0;}
    bool paused()const{return session_.paused();}
    bool loading()const{return loading_;}
    bool canPrevious()const{return !loading_ && historyIndex_>0;}
    bool canNext()const{return !loading_ && (historyIndex_+1<liveHistory_.size() || active() || sessionImageCount()>0);}
    bool historyHealthy()const{return historyHealthy_;}
    bool historyLoading()const{return !historyReady_;}
    Q_INVOKABLE void openDataFolder();
    QVariantList history()const{return history_;}
    QVariantMap stats()const{return stats_;}
    QVariantMap practice()const{return practice_;}
    int statsPeriod()const{return statsPeriod_;}
    void setStatsPeriod(int days);
    qint64 sessionPracticeMs()const{return sessionPracticeMs_;}
    Q_INVOKABLE void shiftTrackerMonth(int delta);
    Q_INVOKABLE void resetTrackerMonth();
    Q_INVOKABLE void setPresetDuration(int seconds);
    Q_INVOKABLE void setPresetCount(int count);
    QString toast()const{return toast_;}
    bool dark()const;
    QString accent()const;
    QVariantMap themeOptions()const;
    QVariantMap palette()const;
    bool pointerInsideWindow()const{return pointerInsideWindow_;}
    bool bw()const{return bw_;} bool flipH()const{return flipH_;} bool flipV()const{return flipV_;}
    int rotation()const{return rotation_;} bool grid()const{return grid_;}
    Q_INVOKABLE void loadSource(const QVariant& source);
    Q_INVOKABLE void loadFiles(const QList<QUrl>& files);
    Q_INVOKABLE void loadPaths(QStringList paths);
    Q_INVOKABLE void addFolder(const QUrl& folder);
    Q_INVOKABLE void startDownloadedSession(const QString& folder);
    void importOnlineCollection(const QString& folder,const QString& name);
    Q_INVOKABLE void selectCollection(const QString& id,bool selected);
    Q_INVOKABLE void selectAllCollections(bool selected);
    Q_INVOKABLE void removeCollection(const QString& id);
    Q_INVOKABLE void undoCollectionRemoval();
    Q_INVOKABLE void renameCollection(const QString& id,const QString& name);
    Q_INVOKABLE bool beginWindowMove();
    Q_INVOKABLE void dropUrls(const QList<QUrl>& urls);
    Q_INVOKABLE void cancelScan();
    Q_INVOKABLE void startSession(bool freeView=false);
    Q_INVOKABLE void stopSession();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void next();
    Q_INVOKABLE void skip();
    Q_INVOKABLE void openMedia(const QString& path);
    Q_INVOKABLE void previewPresented(const QString& source);
    Q_INVOKABLE void closePreview();
    Q_INVOKABLE void stepPreview(int delta);
    QString previewImage() const {return previewPath_.isEmpty()?QString():imageSource(previewPath_,previewGeneration_);}
    QString previewPath() const {return previewPath_;}
    int previewIndex() const {return previewIndex_;}
    int previewCount() const {return previewPaths_.size();}
    Q_INVOKABLE void ready(quint64 id);
    Q_INVOKABLE void loadError(quint64 id);
    Q_INVOKABLE void openSettings();
    Q_INVOKABLE void closeSettings();
    Q_INVOKABLE void applySettings(const QVariantMap& settings);
    Q_INVOKABLE void setSearch(const QString& query);
    Q_INVOKABLE void setEffect(const QString& effect);
    Q_INVOKABLE void resetEffects();
    Q_INVOKABLE void copyImage(bool effects=true);
    Q_INVOKABLE void openCurrentFile();
    Q_INVOKABLE void trashCurrent();
    Q_INVOKABLE void clearHistory(bool all=false);
    Q_INVOKABLE void importLegacy();
    Q_INVOKABLE void toggleFullscreen();
    Q_INVOKABLE void togglePin();
    Q_INVOKABLE void setSidebarCompact(bool compact){settings_["sidebar_compact"]=compact;persist();emit settingsChanged();}
    Q_INVOKABLE void notify(const QString& text);
    Q_INVOKABLE QVariantMap diagnostics()const;
    void framePresented();
signals:
    void previewChanged();
    void catalogChanged(); void imageChanged(); void sessionChanged(); void settingsChanged();
    void effectsChanged(); void navigationChanged(); void historyChanged(); void toastChanged();
    void settingsRequested(); void presented(QString path);
    void imageMetadataChanged();
    void rendererChanged();
    void pointerInsideChanged();
    void collectionsChanged();
    void practiceChanged();
    void countdownCue(int secondsRemaining);
    void visualProgressChanged();
protected:
    bool eventFilter(QObject* watched,QEvent* event) override;
private:
    enum class Presentation { Preview, Live, Review };
    void scan(QStringList paths);
    void appendSources(const QStringList& paths);
    void rescanLibrary();
    void refreshCollections(bool selectionChanged=true);
    void indexCollections();
    void recountCompleted();
    void completeCurrent();
    void updateWindowPin();
    void request(const QString& path,Presentation role);
    void rebuildQueue(bool newCycle=false);
    void chooseNext();
    void advance(bool count);
    void updateClock();
    void scheduleCue();
    void stopSounds();
    void updateProgressTimer();
    void requestAnalytics();
    void handleAction(gestart::Action action);
    void persist();
    qint64 now()const{return clock_.elapsed();}
    QString dataDirectory_;
    CatalogModel model_;
    CatalogFilter filter_;
    CatalogScanner scanner_;
    ImageProvider* images_;
    Localization* i18n_;
    Store store_;
    QVariantMap settings_;
    QVariantList history_;
    QVariantList collections_,collectionView_;
    QVariantMap removedCollection_;
    int removedCollectionIndex_=0,finishedImageCount_=0,windowPinApplied_=-1;
    QVariantMap stats_,practice_;
    int statsPeriod_=30;
    quint64 analyticsRequest_=0;
    QDate trackerMonth_=QDate::currentDate(),analyticsDate_=QDate::currentDate();
    qint64 sessionPracticeMs_=0;
    QSet<QString> seen_,invalid_;
    QSet<QString> ephemeralCompleted_;
    QStringList previewPaths_;
    QString previewPath_;
    int previewIndex_=-1;
    quint64 previewGeneration_=0,previewRecorded_=0;
    QStringList sources_,queue_,liveHistory_;
    QStringList eligiblePaths_,sessionPaths_;
    int queueIndex_=0,historyIndex_=-1;
    quint64 generation_=0,requestId_=0,readyId_=0;
    QString page_="home",sourceLabel_,sourceId_,currentImage_,currentPath_,toast_;
    QSize imagePixelSize_,pendingPixelSize_;
    Presentation presentation_=Presentation::Preview;
    bool scanning_=false,loading_=false,historyReady_=false,resumeAfterReview_=false;
    bool historyHealthy_=false,timelineLoaded_=false;
    QString lastStorageError_;
    int liveAnchor_=-1;
    bool bw_=false,flipH_=false,flipV_=false,grid_=false;
    bool pointerInsideWindow_=true;
    int rotation_=0;
    gestart::Session session_;
    gestart::Countdown countdown_;
    QElapsedTimer clock_,requestClock_;
    QTimer tick_,toastTimer_,dateTimer_,cueTimer_,progressTimer_;
    qint64 lastTick_=0;
    QVector<qint64> timings_;
    QStringList loadErrors_;
    std::mt19937 random_{std::random_device{}()};
    QQuickWindow* window_=nullptr;
    QSoundEffect sound_,endSound_;
    QThreadPool actionPool_,metadataPool_,indexPool_;
    quint64 indexRevision_=0;
    QHash<QString,QStringList> collectionMembers_,pathCollections_;
    QHash<QString,int> collectionCompleted_;
    std::shared_ptr<std::atomic_bool> indexCancel_;
    bool exportBusy_=false,onlineRefreshPending_=false;
    QString browseCollection_,pendingDownloadedSession_;
};
