#include "app_paths.h"
#include <QScreen>
#include <QCursor>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QDir>
#include <QImageReader>
#include <QFile>
#include <QLockFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QThreadPool>
#include <QFontDatabase>
#include <QIcon>
#include <QEvent>
#include <QQuickItem>
#include <QQuickRenderTarget>
#include <QJSValue>
#include <QtTest/QTest>
#include <QtMath>
#include <cmath>
#include "media/collections.h"
#include "media/online_sources.h"
#include "media/download_queue.h"
#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include "qa/polish_checks.h"
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include "app/controller.h"

static QQuickItem* findVisualItem(QQuickItem* root,const QString& name){
    if(!root)return nullptr;if(root->objectName()==name)return root;
    for(auto* child:root->childItems())if(auto* found=findVisualItem(child,name))return found;
    return nullptr;
}
static QQuickItem* findVisibleItem(QQuickItem* root,const QString& name){
    if(!root)return nullptr;if(root->objectName()==name&&root->isVisible())return root;
    for(auto* child:root->childItems())if(auto* found=findVisibleItem(child,name))return found;
    return nullptr;
}

int main(int argc,char** argv){
    AppPaths::configureTemporaryFiles("GestArt Ultimate");
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QtWebEngineQuick::initialize();
    QGuiApplication application(argc,argv);
    QCoreApplication::setOrganizationName("LeonWGal");
    QCoreApplication::setApplicationName("GestArt Ultimate");
    QCoreApplication::setApplicationVersion("1.8.0");
    QQuickStyle::setStyle("Basic");
    QFontDatabase::addApplicationFont(":/assets/fonts/Inter.ttf");
    QFontDatabase::addApplicationFont(":/assets/fonts/NotoSansArabic.ttf");
    QFontDatabase::addApplicationFallbackFontFamily(QChar::Script_Arabic,"Noto Sans Arabic");
    QGuiApplication::setWindowIcon(QIcon(":/assets/app-icon.svg"));
    QCommandLineParser parser;parser.setApplicationDescription("GestArt Ultimate — reference studio for artists");parser.addHelpOption();parser.addVersionOption();
    parser.addOption({"portable","Store data in the data folder beside the portable package"});
    parser.addOption({"data-dir","Use an isolated data directory","directory"});
    parser.addOption({"source","Open a folder or an image","path"});
    parser.addOption({"capture","Save a screenshot and exit (QA)","file"});
    parser.addOption({"capture-page","home, studio, result, library, practice, history, settings, appearance, window, goal, data, collections, navigation or session-menu (QA)","page","home"});
    parser.addOption({"capture-scroll","Settings scroll offset (QA)","pixels","0"});
    parser.addOption({"diagnostics","Write diagnostic JSON on exit","file"});
    parser.addOption({"smoke-test","Run end-to-end session checks and exit"});
    parser.addOption({"timed-test","Check timed completion, break and restart; requires --data-dir"});
    parser.addOption({"web-focus-test","Check hiding/restoring website controls (QA; use with --web-test)"});
    parser.addOption({"web-test","Check a website inside the embedded browser (QA)","provider"});
    parser.addOption({"saved-flow-test","Check starting a persistent saved set with its session settings (QA)"});
    parser.addOption({"queue-test","Check batch downloads, separate folders and starting a downloaded session (QA)"});
    parser.addOption({"online-preview-test","Check reference set preview in an isolated profile (QA)","provider"});
    parser.addOption({"online-test","Import public references into an isolated profile (QA)","provider"});
    parser.addOption({"online-during-session","Check deferred import while a session is active (QA)"});
    parser.addOption({"online-via-queue","Check the selected online source through the download flow (QA)"});
    parser.addOption({"online-count","Images for online QA","count","2"});
    parser.addOption({"benchmark","Run 100 navigation steps and exit"});
    parser.addOption({"size","Window size for QA, for example 800x600","size"});
    parser.addOption({"capture-bw","Enable grayscale for screenshot QA"});
    parser.addOption({"capture-session","Start a timed session for screenshot QA"});
    parser.addOption({"capture-hidden","Send a window leave event before screenshot QA"});
    parser.addOption({"capture-grid","Show the image grid for screenshot QA"});
    parser.addOption({"capture-flip-h","Reflect the image and grid horizontally for screenshot QA"});
    parser.addOption({"capture-flip-v","Reflect the image and grid vertically for screenshot QA"});
    parser.addOption({"capture-rotate","Rotate the reference 90 degrees for screenshot QA"});
    parser.addOption({"capture-zoom","Reference zoom factor for screenshot QA","factor","1"});
    parser.addOption({"capture-hover","Hover the session play button for screenshot QA"});
    parser.addOption({"ui-test","Check immersive geometry and cursor visibility; requires --data-dir"});
    parser.addOption({"flow-test","Check home, statistics, tracker, completion and settings save/cancel; requires --data-dir"});
    parser.addOption({"refinement-test","Check settings text hit target, merged practice, timer size and countdown with pause; requires --data-dir"});
    parser.addOption({"studio-test","Check image grid, progress animation, toolbar and glass alignment; requires --data-dir"});
    parser.addOption({"controls-test","Check toolbar, timer anchors and keyboard shortcuts; requires --data-dir"});
    parser.addOption({"preview-test","Check session isolation and independent previews; requires --data-dir"});
    parser.addOption({"lobe-test","Check segmented keyboard, history search and status actions; requires --data-dir"});
    parser.addOption({"interaction-test","Check normal-motion zoom, active forms and break pause; requires --data-dir"});
    parser.addOption({"polish-test","Check combined transforms, exposed toolbar, session popover and completion flow; requires --data-dir"});
    parser.addOption({"glass-test","Check native blur suppresses a dense checker pattern; requires --data-dir and checker source"});
    parser.addOption({"collection-test","Check collection selection, rename, removal and undo; requires --data-dir, --source and --second-source"});
    parser.addOption({"collection-test-empty","Remove all collections at the end of --collection-test"});
    parser.addOption({"second-source","Second collection for QA","path"});
    parser.process(application);
    if((parser.isSet("controls-test") || parser.isSet("preview-test") || parser.isSet("timed-test") || parser.isSet("ui-test") || parser.isSet("flow-test") || parser.isSet("refinement-test") || parser.isSet("studio-test") || parser.isSet("lobe-test") || parser.isSet("interaction-test") || parser.isSet("polish-test") || parser.isSet("glass-test") || parser.isSet("capture-session") || parser.isSet("collection-test") || parser.value("capture-page")=="result") && !parser.isSet("data-dir"))return 6;
    if(parser.isSet("collection-test")&&(!parser.isSet("source")||!parser.isSet("second-source")))return 6;
    if((parser.isSet("online-test")||parser.isSet("online-preview-test")||parser.isSet("web-test")||parser.isSet("queue-test")||parser.isSet("saved-flow-test"))&&!parser.isSet("data-dir"))return 6;
    const bool portable=parser.isSet("portable")||QFileInfo::exists(QCoreApplication::applicationDirPath()+"/../portable.flag");
    const QString defaultDirectory=portable?QDir::cleanPath(QCoreApplication::applicationDirPath()+"/../data"):AppPaths::dataDirectory("GestArt Ultimate");
    const QString directory=parser.isSet("data-dir")?QFileInfo(parser.value("data-dir")).absoluteFilePath():defaultDirectory;
    if (!parser.isSet("data-dir")&&!portable)
        AppPaths::migrateProfile(AppPaths::dataRoot()+"/GestArt/GestArt Ultimate", directory);
    AppPaths::configureQmlCache(directory);
    QDir().mkpath(directory);
    QLockFile profileLock(directory+"/profile.lock");
    if(!profileLock.tryLock(0)){qWarning("This GestArt profile is already open or cannot be locked.");return 2;}
    QDir().mkpath(directory);
    QImageReader::setAllocationLimit(256);
    auto* provider=new ImageProvider;
    Localization i18n;
    Controller controller(directory,provider,&i18n);
    DownloadQueue downloads(directory);
    QObject::connect(&downloads,&DownloadQueue::imported,&controller,[&](const QString& folder,const QString& name,int){controller.importOnlineCollection(folder,name);});
    OnlineSources online(directory);
    downloads.setCatalogSource(&online);
    QObject::connect(&online,&OnlineSources::imported,&controller,[&](const QString& folder,const QString& name,int){controller.importOnlineCollection(folder,name);});
    QQmlApplicationEngine engine;
    engine.addImageProvider("media",provider);engine.addImageProvider("icons",new IconProvider);
    engine.rootContext()->setContextProperty("app",&controller);
    engine.rootContext()->setContextProperty("i18n",&i18n);
    engine.rootContext()->setContextProperty("online",&online);
    engine.rootContext()->setContextProperty("downloads",&downloads);
    engine.rootContext()->setContextProperty("webStoragePath",directory+"/browser");
    engine.rootContext()->setContextProperty("webQaEnabled",parser.isSet("web-test"));
    QObject::connect(&engine,&QQmlApplicationEngine::objectCreationFailed,&application,[]{QCoreApplication::exit(2);},Qt::QueuedConnection);
    engine.loadFromModule("GestArt","Main");
    if(engine.rootObjects().isEmpty())return 2;
    auto* window=qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    if(!window)return 3;controller.setWindow(window);
    if(parser.isSet("online-test")){
        auto started=std::make_shared<bool>(false);
        auto sessionStarted=std::make_shared<bool>(false);
        auto sessionAnchor=std::make_shared<QString>();
        QObject::connect(&online,&OnlineSources::changed,&application,[&,started,sessionStarted,sessionAnchor]{
            if(online.importing()&&online.busy()&&!*sessionStarted&&parser.isSet("online-during-session")){
                *sessionStarted=true;controller.startSession(true);*sessionAnchor=controller.currentPath();
            }
            if(online.busy())return;
            if(!online.error().isEmpty()){qWarning().noquote()<<"Online QA:"<<online.error();application.exit(171);return;}
            if(!*started&&!online.sets().isEmpty()){*started=true;QTimer::singleShot(0,&online,[&]{if(parser.isSet("online-via-queue"))downloads.download(online.provider(),{online.sets().first()},parser.value("online-count").toInt());
                else online.importImages(online.sets().first().toMap().value("id").toString(),parser.value("online-count").toInt());});}
        });
        auto importedCheck=[&,sessionAnchor](const QString& folder,const QString& name,int count){
            if(parser.isSet("online-during-session")){
                if(!controller.active()||controller.currentPath()!=*sessionAnchor){qWarning("Online import interrupted active session");application.exit(174);return;}
                controller.stopSession();controller.setPage("home");qInfo("Online QA: active session preserved, deferred refresh released");
            }
            qInfo().noquote()<<"Online QA:"<<name<<"imported"<<count;
            QTimer::singleShot(2000,&application,[&,folder,count]{
                int readable=0;for(const auto& file:QDir(folder).entryList(QDir::Files))if(QImageReader(folder+"/"+file).canRead())++readable;
                application.exit(readable>=count&&controller.imageCount()>=count?0:172);
            });
        };
        QObject::connect(&online,&OnlineSources::imported,&application,importedCheck);
        if(parser.isSet("online-via-queue"))QObject::connect(&downloads,&DownloadQueue::imported,&application,importedCheck);
        QTimer::singleShot(500,&online,[&]{online.selectProvider(parser.value("online-test"));});
        QTimer::singleShot(100000,&application,[&]{qWarning("Online QA timed out");application.exit(173);});
    }
    if(parser.isSet("online-preview-test")){
        auto started=std::make_shared<bool>(false);
        QObject::connect(&online,&OnlineSources::changed,&application,[&,started]{
            if(online.busy()||*started)return;
            if(!online.error().isEmpty()||online.sets().isEmpty()){application.exit(189);return;}
            *started=true;
            QTimer::singleShot(0,&application,[&]{
                QMetaObject::invokeMethod(window,"openProviderDownloads",Q_ARG(QVariant,QVariant(online.provider())));
                auto* drawer=window->findChild<QObject*>("onlineSourcesDrawer");
                if(drawer)QMetaObject::invokeMethod(drawer,"showPreview",Q_ARG(QVariant,online.sets().first()));
            });
        });
        auto* check=new QTimer(&application);check->setInterval(500);
        QObject::connect(check,&QTimer::timeout,&application,[&,check]{
            if(online.previewBusy())return;
            auto* image=window->findChild<QQuickItem*>("onlinePreviewImage");
            if(!image||image->property("status").toInt()!=1)return;
            qInfo("Online preview QA: image ready");
            if(parser.isSet("capture"))window->grabWindow().save(parser.value("capture"));
            check->stop();application.exit(0);
        });check->start();
        QTimer::singleShot(200,&application,[&]{online.selectProvider(parser.value("online-preview-test"));});
        QTimer::singleShot(80000,&application,[&]{qWarning().noquote()<<"Preview QA timed out:"<<online.previewError();application.exit(190);});
    }
    QVariantMap polishMetrics,glassMetrics;
    if(parser.isSet("web-test")){
        window->hide();window->showNormal();
        if(parser.isSet("web-focus-test"))window->setPosition(QCursor::pos()+QPoint(40,40));
        QTimer::singleShot(200,&application,[&]{QMetaObject::invokeMethod(window,"openWebsite",Q_ARG(QVariant,QVariant(parser.value("web-test"))));});
        auto focusPhase=std::make_shared<int>(0);auto viewportHeight=std::make_shared<int>(0);
        auto* check=new QTimer(&application);check->setInterval(500);
        QObject::connect(check,&QTimer::timeout,&application,[&,check,focusPhase,viewportHeight]{
            const auto metrics=window->property("browserMetrics").toMap();
            if(metrics.value("provider").toString()!=parser.value("web-test")||metrics.value("controls").toInt()<3||metrics.value("bodyLength").toInt()<100)return;
            if(parser.isSet("web-focus-test")){
                if(*focusPhase==0){QMetaObject::invokeMethod(window,"prepareWebFocusTest");*focusPhase=1;return;}
                if(*focusPhase==1){if(auto* page=window->findChild<QQuickItem*>("embeddedWebsite"))*viewportHeight=qRound(page->height());QEvent leave(QEvent::Leave);QCoreApplication::sendEvent(window,&leave);QMetaObject::invokeMethod(window,"inspectWebFocusTest");*focusPhase=2;return;}
                if(*focusPhase==2){
                    if(!metrics.value("focusHidden").toBool()||!metrics.value("focusImage").toBool()||metrics.value("chromeVisible").toBool()){qWarning()<<"Web focus did not hide controls and preserve image"<<metrics;application.exit(187);return;}
                    if(metrics.value("viewportTop").toInt()!=0||metrics.value("viewportHeight").toInt()!=metrics.value("workspaceHeight").toInt()||metrics.value("viewportHeight").toInt()<=*viewportHeight){qWarning("Web focus left an empty header area");application.exit(191);return;}
                    QEnterEvent enter(QPointF(10,10),QPointF(10,10),QPointF(10,10));QCoreApplication::sendEvent(window,&enter);QMetaObject::invokeMethod(window,"inspectWebFocusTest");*focusPhase=3;return;
                }
                if(metrics.value("focusHidden").toBool()||!metrics.value("chromeVisible").toBool()||metrics.value("viewportHeight").toInt()!=*viewportHeight){qWarning("Web focus did not restore controls/viewport");application.exit(188);return;}
                qInfo("Web focus QA: header space reclaimed, viewport expanded/restored, image preserved");
            }
            check->stop();qInfo().noquote()<<"Website QA:"<<metrics.value("provider").toString()<<metrics.value("title").toString()<<"controls"<<metrics.value("controls").toInt();
            if(parser.isSet("capture"))window->grabWindow().save(parser.value("capture"));
            QFile report(directory+"/website-check.json");if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument::fromVariant(metrics).toJson());
            application.exit(0);
        });check->start();QTimer::singleShot(90000,&application,[&]{qWarning("Embedded website QA timed out");application.exit(181);});
    }
    if(parser.isSet("saved-flow-test")){
        auto phase=std::make_shared<int>(0);auto folder=std::make_shared<QString>();
        auto* check=new QTimer(&application);check->setInterval(300);
        QObject::connect(check,&QTimer::timeout,&application,[&,check,phase,folder]{
            if(controller.scanning())return;
            if(*phase==0){
                if(downloads.savedSets().isEmpty()){qWarning("Saved flow QA has no saved set");application.exit(192);return;}
                const auto set=downloads.savedSets().first().toMap();*folder=set.value("folder").toString();
                QMetaObject::invokeMethod(window,"openSavedSets");
                auto* drawer=window->findChild<QObject*>("onlineSourcesDrawer");
                if(!drawer){application.exit(193);return;}
                QMetaObject::invokeMethod(drawer,"prepareSession",Q_ARG(QVariant,QVariant(set)));*phase=1;return;
            }
            if(*phase==1){
                auto* start=window->findChild<QQuickItem*>("startSavedSession");
                if(!start||!start->isEnabled())return;
                QMetaObject::invokeMethod(start,"clicked");*phase=2;return;
            }
            if(controller.loading()){window->grabWindow();controller.framePresented();return;}
            if(!controller.active())return;
            int selected=0;for(const auto& value:controller.collections())if(value.toMap().value("selected").toBool())++selected;
            if(QFileInfo(controller.currentPath()).absolutePath()!=QDir(*folder).absolutePath()||selected!=1){qWarning("Saved flow mixed other collections");application.exit(194);return;}
            qInfo("Saved flow QA: persistent set opened through session setup; only this set is selected");check->stop();application.exit(0);
        });check->start();QTimer::singleShot(30000,&application,[&]{application.exit(195);});
    }
    if(parser.isSet("queue-test")){
        auto started=std::make_shared<bool>(false),verified=std::make_shared<bool>(false);
        QObject::connect(&downloads,&DownloadQueue::changed,&application,[&,started,verified]{
            if(!*started||downloads.running()||*verified)return;
            *verified=true;QSet<QString> folders;
            for(const auto& value:downloads.jobs()){
                const auto job=value.toMap();if(job.value("status")!="Completed"||job.value("completed").toInt()!=2){qWarning().noquote()<<"Queue QA failed:"<<job.value("error").toString();application.exit(182);return;}
                folders.insert(job.value("folder").toString());
            }
            if(folders.size()!=3){qWarning("Separate downloads share a folder");application.exit(183);return;}
            const auto folder=downloads.jobs().first().toMap().value("folder").toString();qInfo("Queue download finished; starting folder session");controller.startDownloadedSession(folder);
            auto* verify=new QTimer(&application);verify->setInterval(100);
            QObject::connect(verify,&QTimer::timeout,&application,[&,verify,folder]{
                if(controller.loading()){window->grabWindow();controller.framePresented();return;}
                if(!controller.active())return;
                const auto expected=QDir(folder).absolutePath();
                if(controller.sessionImageCount()!=2||QFileInfo(controller.currentPath()).absolutePath()!=expected){qWarning("Downloaded-folder session uses another collection");application.exit(184);return;}
                verify->stop();qInfo("Queue QA: 3 sets, 6 images, 3 folders; downloaded-folder session started");
                QFile report(directory+"/queue-check.json");if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument::fromVariant(downloads.jobs()).toJson());application.exit(0);
            });verify->start();
        });
        QTimer::singleShot(500,&downloads,[&,started]{
            QDir().mkpath(directory+"/chosen-download-folder");downloads.setDestination(QUrl::fromLocalFile(directory+"/chosen-download-folder"));
            online.selectProvider("quickposes");
            QMetaObject::invokeMethod(window,"openOnlineSources",Q_ARG(QVariant,QVariant("download")));
            auto* drawer=window->findChild<QObject*>("onlineSourcesDrawer");
            auto* count=window->findChild<QObject*>("onlineCountPicker");
            if(!drawer||!count){application.exit(186);return;}
            drawer->setProperty("gender","female");drawer->setProperty("clothing","clothed");
            drawer->setProperty("selectedSets",QVariantList{QString("pose_all_all"),QString("hands_all")});
            count->setProperty("model",QStringList{"2"});count->setProperty("currentIndex",0);
            QVariant added;if(!QMetaObject::invokeMethod(drawer,"addSelection",Q_RETURN_ARG(QVariant,added))||!added.toBool()){qWarning()<<"QML download selection failed"<<online.provider()<<online.sets().size()<<count->property("currentText");application.exit(187);return;}
            QMetaObject::invokeMethod(drawer,"close");
            downloads.add("sketchdaily",{QVariantMap{{"id","FullBodies"},{"name","Full body"}}},2,true,{{"gender","female"},{"clothing","clothed"},{"pose","stationary"},{"angle","front"}});
            *started=true;downloads.start();
        });
        QTimer::singleShot(120000,&application,[&]{qWarning()<<"Download queue QA timed out"<<controller.state()<<controller.loading()<<controller.scanning()<<controller.sessionImageCount()<<controller.currentPath()<<downloads.running();application.exit(185);});
    }

    if(parser.isSet("controls-test"))installControlChecks(window,&controller,&polishMetrics);
    if(parser.isSet("preview-test"))installPreviewChecks(window,&controller,&polishMetrics);
    if(parser.isSet("lobe-test"))installLobeChecks(window,&controller,&polishMetrics);
    if(parser.isSet("interaction-test"))installInteractionChecks(window,&controller,&polishMetrics);
    if(parser.isSet("polish-test"))installPolishChecks(window,&controller,&polishMetrics);
    if(parser.isSet("glass-test"))installGlassChecks(window,&controller,&glassMetrics);
    if(parser.isSet("controls-test")||parser.isSet("preview-test")||(parser.isSet("capture")&&!parser.isSet("web-test")&&!parser.isSet("online-preview-test"))||parser.isSet("studio-test")||parser.isSet("lobe-test")||parser.isSet("interaction-test")||parser.isSet("polish-test")||parser.isSet("glass-test")){
        // Qt 6.8.3 grabOffscreen creates its temporary target with DPR 1.
        // Keep QA readbacks at the window's density; normal rendering has no target.
        QObject::connect(window,&QQuickWindow::beforeSynchronizing,window,[window]{
            auto target=window->renderTarget();
            if(!target.isNull()&&!qFuzzyCompare(target.devicePixelRatio(),window->devicePixelRatio())){
                target.setDevicePixelRatio(window->devicePixelRatio());window->setRenderTarget(target);
            }
        },Qt::DirectConnection);
    }
    if(parser.isSet("size")){const auto size=parser.value("size").split('x');if(size.size()==2)window->resize(size[0].toInt(),size[1].toInt());}
    if(parser.isSet("source"))QTimer::singleShot(0,&controller,[&]{controller.loadSource(parser.value("source"));});
    if(parser.isSet("capture")&&!parser.isSet("web-test")&&!parser.isSet("online-preview-test")){
        QTimer::singleShot(parser.isSet("source")?4000:1000,&controller,[&]{
            const QString page=parser.value("capture-page");
            if(page=="result"){auto s=controller.settings();s["display_time"]=1;s["num_images"]=1;s["unlimited_time"]=false;s["unlimited_images"]=false;s["use_break"]=false;s["timer_volume"]=0;controller.applySettings(s);controller.startSession();}
            if(parser.isSet("capture-session"))controller.startSession();
            if(page=="appearance")QMetaObject::invokeMethod(window,"openAppearance");
            else if(page=="window" || page=="data" || page=="goal")QMetaObject::invokeMethod(window,"openSettingsSection",Q_ARG(QVariant,QVariant(page=="window"?2:page=="goal"?3:4)));
            else if(page=="collections")QMetaObject::invokeMethod(window,"openCollections");
            else if(page=="online")QMetaObject::invokeMethod(window,"openOnlineSources",Q_ARG(QVariant,QVariant("sites")));
            else if(page=="saved-sets"||page=="saved-setup"){
                QMetaObject::invokeMethod(window,"openSavedSets");
                if(page=="saved-setup"&&!downloads.savedSets().isEmpty())if(auto* drawer=window->findChild<QObject*>("onlineSourcesDrawer"))QMetaObject::invokeMethod(drawer,"prepareSession",Q_ARG(QVariant,downloads.savedSets().first()));
            }
            else if(page=="downloader")QMetaObject::invokeMethod(window,"openOnlineSources",Q_ARG(QVariant,QVariant("download")));
            else if(page.startsWith("web-"))QMetaObject::invokeMethod(window,"openWebsite",Q_ARG(QVariant,QVariant(page.mid(4))));
            else if(page=="navigation")QMetaObject::invokeMethod(window,"openNavigation");
            else if(page=="settings")controller.openSettings();
            else if(page=="preview"){controller.setPage("library");if(controller.catalog()->rowCount()>0)controller.openMedia(controller.catalog()->data(controller.catalog()->index(0,0),CatalogModel::PathRole).toString());}
            else if(page.endsWith("-menu"))QMetaObject::invokeMethod(window,"openStudioMenu",Q_ARG(QVariant,QVariant(page)));
            else if(page!="home"||!parser.isSet("capture-session"))controller.setPage(page);
            if(parser.isSet("capture-bw"))controller.setEffect("bw");
            if(parser.isSet("capture-hidden")){QEvent leave(QEvent::Leave);QCoreApplication::sendEvent(window,&leave);}
            QTimer::singleShot(1200,&application,[&]{
                const QString capturePage=parser.value("capture-page");const QString scrollName=capturePage=="home"?"homeScroll":QStringList{"practice","statistics","tracker"}.contains(capturePage)?"practiceScroll":"settingsScroll";
                if(auto* scroll=window->findChild<QQuickItem*>(scrollName)){const auto content=scroll->property("contentItem").value<QQuickItem*>();if(content)content->setProperty("contentY",parser.value("capture-scroll").toInt());}
                // A hidden native window needs a render pass to initialize layer textures.
                window->grabWindow();
                controller.framePresented();
                if(parser.isSet("capture-grid")&&!controller.grid())controller.setEffect("grid");
                if(parser.isSet("capture-flip-h"))controller.setEffect("horizontal");
                if(parser.isSet("capture-flip-v"))controller.setEffect("vertical");
                if(parser.isSet("capture-rotate"))controller.setEffect("rotate");
                if(parser.isSet("capture-zoom"))if(auto* studio=window->findChild<QQuickItem*>("studioPage"))QMetaObject::invokeMethod(studio,"zoomCentered",Q_ARG(QVariant,QVariant(parser.value("capture-zoom").toDouble())));
                if(parser.isSet("capture-hover"))if(auto* button=window->findChild<QQuickItem*>("sessionPlayButton"))QTest::mouseMove(window,button->mapToScene(QPointF(button->width()/2,button->height()/2)).toPoint());
                QTimer::singleShot(capturePage=="result"?1500:parser.isSet("capture-zoom")||parser.isSet("capture-grid")||parser.isSet("capture-hover")||parser.isSet("capture-rotate")?500:150,&application,[&]{
                    const QImage image=window->grabWindow();controller.framePresented();
                    if(image.isNull() || !image.save(parser.value("capture")))QCoreApplication::exit(4);else QCoreApplication::quit();
                });
            });
        });
    }
    int benchmarkSteps=0,smokeSteps=0;
    int uiPhase=0; qreal canvasWidth=0,canvasHeight=0;QRect pinnedGeometry;
    QTimer uiChecks;
    if(parser.isSet("ui-test")){
        auto settings=controller.settings();settings["display_time"]=60;settings["show_timer"]=true;settings["hide_timer_on_leave"]=false;settings["always_on_top"]=false;settings["timer_volume"]=0;controller.applySettings(settings);
        uiChecks.setInterval(400);
        QObject::connect(&uiChecks,&QTimer::timeout,&controller,[&]{
            if(controller.scanning() || controller.imageCount()==0)return;
            if(controller.loading()&&uiPhase>0){window->grabWindow();controller.framePresented();return;}
            auto* canvas=window->findChild<QQuickItem*>("imageCanvas");
            auto* chrome=window->findChild<QQuickItem*>("sessionChrome");
            auto* timer=window->findChild<QQuickItem*>("sessionTimer");
            auto* progress=window->findChild<QQuickItem*>("sessionProgress");
            if(!canvas||!chrome||!timer||!progress){QCoreApplication::exit(20);return;}
            switch(uiPhase++){
            case 0:controller.startSession();break;
            case 1:{
                canvasWidth=canvas->width();canvasHeight=canvas->height();
                if(canvasHeight<window->height()-30 || !timer->isVisible() || (window->width()<340&&timer->height()>28)){QCoreApplication::exit(21);return;}
                QEvent leave(QEvent::Leave);QCoreApplication::sendEvent(window,&leave);break;
            }
            case 2:{
                if(chrome->opacity()>0.01 || chrome->isEnabled() || !timer->isVisible() || !progress->isVisible()
                    || timer->opacity()!=1 || controller.state()!="running" || canvas->width()!=canvasWidth || canvas->height()!=canvasHeight){QCoreApplication::exit(22);return;}
                QEnterEvent enter(QPointF(10,10),QPointF(10,10),QPointF(10,10));QCoreApplication::sendEvent(window,&enter);break;
            }
            case 3:{
                if(chrome->opacity()<0.99 || !chrome->isEnabled() || controller.state()!="running"){QCoreApplication::exit(23);return;}
                auto s=controller.settings();s["hide_timer_on_leave"]=true;controller.applySettings(s);
                QEvent leave(QEvent::Leave);QCoreApplication::sendEvent(window,&leave);break;
            }
            case 4:{
                if(timer->opacity()>0.01 || !progress->isVisible() || controller.state()!="running" || canvas->width()!=canvasWidth || canvas->height()!=canvasHeight){QCoreApplication::exit(25);return;}
                QEnterEvent enter(QPointF(10,10),QPointF(10,10),QPointF(10,10));QCoreApplication::sendEvent(window,&enter);
                pinnedGeometry=window->geometry();controller.togglePin();
                if(window->geometry()!=pinnedGeometry){QCoreApplication::exit(26);return;}
#ifdef Q_OS_WIN
                if(QGuiApplication::platformName()=="windows"){
                    const auto hwnd=reinterpret_cast<HWND>(window->winId());
                    if(!(GetWindowLongPtr(hwnd,GWL_EXSTYLE)&WS_EX_TOPMOST) || !(GetWindowLongPtr(hwnd,GWL_STYLE)&WS_CAPTION)){QCoreApplication::exit(27);return;}
                }
#endif
                window->setPosition(pinnedGeometry.topLeft()+QPoint(12,8));break;
            }
            default:
                if(timer->opacity()<0.99 || window->position()!=pinnedGeometry.topLeft()+QPoint(12,8) || window->size()!=pinnedGeometry.size()){QCoreApplication::exit(28);return;}
                controller.togglePin();QCoreApplication::quit();break;
            }
        });uiChecks.start();QTimer::singleShot(20000,&application,[]{QCoreApplication::exit(24);});
    }
    QTimer collectionChecks;int collectionPhase=0;QString collectionA,collectionB;
    const QString secondSource=QFileInfo(parser.value("second-source")).absoluteFilePath();
    if(parser.isSet("collection-test")){
        auto s=controller.settings();s["timer_volume"]=0;controller.applySettings(s);
        collectionChecks.setInterval(300);
        QObject::connect(&collectionChecks,&QTimer::timeout,&controller,[&]{
            if(controller.scanning()||(controller.imageCount()==0&&collectionPhase<8))return;
            if(controller.loading()&&controller.page()=="studio"){window->grabWindow();controller.framePresented();return;}
            switch(collectionPhase++){
            case 0:controller.loadSource(secondSource);break;
            case 1:{
                const auto sources=controller.collections();if(sources.size()!=2){QCoreApplication::exit(30);return;}
                for(const auto& value:sources){const auto item=value.toMap();if(pathInSources(item.value("path").toString(),{secondSource}))collectionB=item.value("id").toString();else collectionA=item.value("id").toString();}
                if(collectionA.isEmpty()||collectionB.isEmpty()){QCoreApplication::exit(31);return;}
                controller.selectAllCollections(false);controller.selectCollection(collectionB,true);controller.renameCollection(collectionB,"QA renamed collection");
                if(controller.sessionImageCount()!=controller.collections().last().toMap().value("count").toInt()){QCoreApplication::exit(32);return;}
                controller.startSession();break;
            }
            case 2:if(!pathInSources(controller.currentPath(),{secondSource})){QCoreApplication::exit(33);return;}controller.skip();break;
            case 3:if(controller.finishedImageCount()!=0){QCoreApplication::exit(34);return;}controller.next();break;
            case 4:if(controller.finishedImageCount()!=1){QCoreApplication::exit(35);return;}controller.setPage("history");controller.removeCollection(collectionA);break;
            case 5:if(controller.collections().size()!=1 || !controller.canUndoCollectionRemoval()){QCoreApplication::exit(36);return;}controller.undoCollectionRemoval();break;
            case 6:if(controller.collections().size()!=2 || controller.canUndoCollectionRemoval() || controller.finishedImageCount()!=1){QCoreApplication::exit(37);return;}controller.removeCollection(collectionA);break;
            case 7:if(controller.collections().size()!=1 || controller.finishedImageCount()!=1 || !QFileInfo(parser.value("source")).exists()){QCoreApplication::exit(38);return;}if(parser.isSet("collection-test-empty"))controller.removeCollection(collectionB);else QCoreApplication::quit();break;
            default:if(!controller.collections().isEmpty() || controller.imageCount()!=0 || !QFileInfo(secondSource).exists()){QCoreApplication::exit(40);return;}QCoreApplication::quit();break;
            }
        });collectionChecks.start();QTimer::singleShot(30000,&application,[]{QCoreApplication::exit(39);});
    }
    QTimer flowChecks;int flowPhase=0;
    if(parser.isSet("flow-test")){
        auto s=controller.settings();s["timer_volume"]=0;s["display_time"]=1;s["num_images"]=1;s["unlimited_time"]=false;s["unlimited_images"]=false;s["use_break"]=false;controller.applySettings(s);
        flowChecks.setInterval(250);
        QObject::connect(&flowChecks,&QTimer::timeout,&controller,[&]{
            if(controller.scanning()||controller.imageCount()==0)return;
            if(controller.loading()&&controller.page()=="studio"){window->grabWindow();controller.framePresented();return;}
            switch(flowPhase){
            case 0:if(controller.page()!="home"){QCoreApplication::exit(50);return;}controller.setStatsPeriod(7);controller.setPage("statistics");++flowPhase;break;
            case 1:if(controller.practice().isEmpty())return;if(controller.practice().value("totalStudies").toInt()!=0){QCoreApplication::exit(51);return;}controller.setPage("tracker");controller.shiftTrackerMonth(-1);++flowPhase;break;
            case 2:if(controller.practice().value("month").toString().left(7)==QDate::currentDate().toString("yyyy-MM"))return;if(controller.practice().value("calendar").toList().size()!=42){QCoreApplication::exit(52);return;}controller.resetTrackerMonth();++flowPhase;break;
            case 3:{if(controller.practice().value("month").toString().left(7)!=QDate::currentDate().toString("yyyy-MM"))return;auto* day=findVisualItem(window->contentItem(),"calendarDay-"+QDate::currentDate().toString(Qt::ISODate));if(!day||!day->isEnabled()||!QMetaObject::invokeMethod(day,"clicked")){qWarning("Flow calendar target: found=%d enabled=%d",day!=nullptr,day&&day->isEnabled());QCoreApplication::exit(53);return;}controller.setPage("home");++flowPhase;break;}
            case 4:{auto* button=window->findChild<QQuickItem*>("homeStartButton");if(!button||!button->isEnabled()||!QMetaObject::invokeMethod(button,"clicked")){QCoreApplication::exit(54);return;}++flowPhase;break;}
            case 5:if(controller.state()!="completed")return;if(controller.completed()!=1||controller.sessionPracticeMs()!=1000){QCoreApplication::exit(55);return;}controller.setPage("statistics");++flowPhase;break;
            case 6:if(controller.practice().value("totalStudies").toInt()!=1)return;if(controller.practice().value("totalMs").toLongLong()!=1000){QCoreApplication::exit(56);return;}QMetaObject::invokeMethod(window,"openSettingsSection",Q_ARG(QVariant,QVariant(3)));++flowPhase;break;
            case 7:{auto* sheet=window->findChild<QObject*>("settingsDrawer");auto* cancel=window->findChild<QObject*>("settingsCancelButton");if(!sheet||!cancel){QCoreApplication::exit(57);return;}auto draft=controller.settings();draft["daily_goal_minutes"]=7;sheet->setProperty("draft",draft);QMetaObject::invokeMethod(cancel,"clicked");++flowPhase;break;}
            case 8:if(controller.settings().value("daily_goal_minutes").toInt()==7){QCoreApplication::exit(58);return;}QMetaObject::invokeMethod(window,"openSettingsSection",Q_ARG(QVariant,QVariant(3)));++flowPhase;break;
            case 9:{auto* sheet=window->findChild<QObject*>("settingsDrawer");auto* save=window->findChild<QObject*>("settingsSaveButton");auto draft=controller.settings();draft["daily_goal_minutes"]=1;draft["daily_goal_studies"]=1;draft["daily_goal_mode"]="both";if(!sheet||!save){QCoreApplication::exit(59);return;}sheet->setProperty("draft",draft);QMetaObject::invokeMethod(save,"clicked");++flowPhase;break;}
            default:if(controller.practice().value("goalMode").toString()!="both")return;if(controller.settings().value("daily_goal_minutes").toInt()!=1||std::abs(controller.practice().value("goalProgress").toDouble()-1.0/60)>0.0001){QCoreApplication::exit(60);return;}QCoreApplication::quit();break;
            }
        });flowChecks.start();QTimer::singleShot(30000,&application,[]{QCoreApplication::exit(61);});
    }
    QTimer refinementChecks;int refinementPhase=0;QElapsedTimer refinementClock;QVariantList cueTrace;
    qint64 pausedAt=-1,pauseLength=0;
    if(parser.isSet("refinement-test")){
        auto s=controller.settings();s["timer_volume"]=0;s["display_time"]=4;s["num_images"]=2;s["unlimited_time"]=false;s["unlimited_images"]=false;s["use_break"]=false;controller.applySettings(s);
        refinementClock.start();
        QObject::connect(&controller,&Controller::countdownCue,&controller,[&](int seconds){cueTrace.append(QVariantMap{{"beat",seconds},{"atMs",refinementClock.elapsed()},{"ordinal",controller.ordinal()},{"state",controller.state()}});});
        refinementChecks.setInterval(100);
        QObject::connect(&refinementChecks,&QTimer::timeout,&controller,[&]{
            if(controller.scanning()||controller.imageCount()==0)return;
            if(controller.loading()&&controller.page()=="studio"){window->grabWindow();controller.framePresented();return;}
            switch(refinementPhase){
            case 0:if(window->width()<720)QMetaObject::invokeMethod(window,"openNavigation");pausedAt=refinementClock.elapsed();++refinementPhase;break;
            case 1:{
                if(refinementClock.elapsed()-pausedAt<300)return;
                window->grabWindow(); // Polish popup geometry before sending coordinate events to a hidden QA window.
                auto* nav=window->findChild<QObject*>("navigationDrawer");auto* button=window->width()>=720?findVisibleItem(window->contentItem(),"sidebarSettingsButton"):nav?findVisibleItem(nav->property("contentItem").value<QQuickItem*>(),"sidebarSettingsButton"):nullptr;
                if(!button){QCoreApplication::exit(70);return;}
                const auto position=button->mapToScene(QPointF(button->width()*.4,button->height()/2));
                qInfo("Settings text target at %.1f %.1f, button %.1f x %.1f, drawer %.2f",position.x(),position.y(),button->width(),button->height(),nav->property("position").toDouble());
                QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,position.toPoint());++refinementPhase;break;
            }
            case 2:{auto* sheet=window->findChild<QObject*>("settingsDrawer");if(!sheet||!sheet->property("visible").toBool()||sheet->property("width").toDouble()>480){qInfo("Settings visible: %d width: %.1f",sheet&&sheet->property("visible").toBool(),sheet?sheet->property("width").toDouble():0);window->grabWindow().save(QDir(directory).filePath("refinement-failure.png"));QCoreApplication::exit(71);return;}QMetaObject::invokeMethod(window->findChild<QObject*>("settingsCancelButton"),"clicked");controller.setPage("tracker");++refinementPhase;break;}
            case 3:if(controller.page()!="practice"||!findVisualItem(window->contentItem(),"practiceCalendar")||!findVisualItem(window->contentItem(),"practiceChart")){QCoreApplication::exit(72);return;}controller.startSession();++refinementPhase;break;
            case 4:{
                auto* timer=window->findChild<QQuickItem*>("sessionTimer");if(!timer||(window->width()<340&&(timer->height()>28||timer->width()>130))){QCoreApplication::exit(73);return;}
                if(cueTrace.size()==1){controller.togglePause();pausedAt=refinementClock.elapsed();++refinementPhase;}break;
            }
            case 5:if(refinementClock.elapsed()-pausedAt<500)return;if(cueTrace.size()!=1||!controller.paused()){QCoreApplication::exit(74);return;}pauseLength=refinementClock.elapsed()-pausedAt;controller.togglePause();++refinementPhase;break;
            default:{
                if(controller.state()!="completed")return;
                if(cueTrace.size()!=8||controller.completed()!=2){QCoreApplication::exit(75);return;}
                const QList<int> expected{3,2,1,0,3,2,1,0};
                for(int i=0;i<expected.size();++i){const auto cue=cueTrace[i].toMap();if(cue.value("beat").toInt()!=expected[i]||cue.value("state").toString()!="running"){QCoreApplication::exit(76);return;}
                    if(i>0&&i!=4){const auto interval=cue.value("atMs").toLongLong()-cueTrace[i-1].toMap().value("atMs").toLongLong()-(i==1?pauseLength:0);if(std::abs(interval-1000)>120){QCoreApplication::exit(77);return;}}
                }
                QCoreApplication::quit();break;
            }
            }
        });refinementChecks.start();QTimer::singleShot(25000,&application,[]{QCoreApplication::exit(78);});
    }
    QTimer studioChecks;QElapsedTimer studioClock;int studioPhase=0,progressSamples=0,progressChanges=0;QVariantMap studioMetrics;
    qreal pausedProgress=0,lastProgress=0;QImage glassBaseline;int glassMaxError=0,glassSamples=0;
    if(parser.isSet("studio-test")){
        auto s=controller.settings();s["timer_volume"]=0;s["display_time"]=60;s["num_images"]=2;s["unlimited_time"]=false;s["unlimited_images"]=false;s["use_break"]=false;s["reduce_motion"]=false;s["session_blur"]=true;s["session_glass"]=true;s["show_timer"]=true;s["hide_timer_on_leave"]=true;controller.applySettings(s);
        studioChecks.setInterval(16);studioChecks.setTimerType(Qt::PreciseTimer);studioClock.start();
        QObject::connect(&studioChecks,&QTimer::timeout,&controller,[&]{
            if(controller.scanning()||controller.imageCount()==0)return;
            if(controller.loading()&&controller.page()=="studio"){window->grabWindow();controller.framePresented();return;}
            auto* studio=window->findChild<QQuickItem*>("studioPage");auto* art=window->findChild<QQuickItem*>("referenceImage");
            auto* grid=window->findChild<QQuickItem*>("imageGrid");auto* toolbar=window->findChild<QQuickItem*>("sessionToolbar");
            if(!studio||!art||!grid||!toolbar){QCoreApplication::exit(80);return;}
            const QStringList panels{"sessionContext","sessionWindowActions","sessionToolbar","sessionTimer"};
            auto configureGlass=[&]{for(const auto& name:panels)if(auto* panel=window->findChild<QQuickItem*>(name)){panel->setProperty("surfaceOpacity",0.0);panel->setProperty("blurRadius",0.0);}};
            auto compareGlass=[&]{
                const QImage actual=window->grabWindow();const qreal dpr=qreal(actual.width())/window->width();
                studioMetrics["windowDpr"]=window->devicePixelRatio();studioMetrics["effectiveDpr"]=window->effectiveDevicePixelRatio();studioMetrics["grabDpr"]=dpr;
                qInfo()<<"Studio capture"<<studioPhase<<window->size()<<window->contentItem()->size()<<studio->size()<<"DPR"<<window->devicePixelRatio()<<window->effectiveDevicePixelRatio()<<"grab"<<actual.size();
                for(const auto& name:panels){auto* panel=window->findChild<QQuickItem*>(name);if(!panel||!panel->isVisible())continue;
                    for(const QPointF local:{QPointF(4,panel->height()/2),QPointF(panel->width()/2,panel->height()-4),QPointF(panel->width()-4,panel->height()/2)}){
                        const auto point=panel->mapToScene(local)*dpr;const QPoint pixel(qFloor(point.x()),qFloor(point.y()));
                        if(!actual.rect().contains(pixel)||!glassBaseline.rect().contains(pixel)){QCoreApplication::exit(81);return;}
                        const QColor a=actual.pixelColor(pixel),b=glassBaseline.pixelColor(pixel);
                        const int error=std::max({std::abs(a.red()-b.red()),std::abs(a.green()-b.green()),std::abs(a.blue()-b.blue())});
                        if(error>4){qWarning()<<"Glass sample"<<studioPhase<<name<<pixel<<"DPR"<<panel->property("pixelRatio")<<"origin"<<panel->property("origin")<<"region"<<panel->property("sampleRect")<<"actual"<<a<<"expected"<<b;actual.save(QDir(directory).filePath(QString("glass-phase-%1.png").arg(studioPhase)));glassBaseline.save(QDir(directory).filePath(QString("glass-baseline-%1.png").arg(studioPhase)));}
                        glassMaxError=std::max({glassMaxError,std::abs(a.red()-b.red()),std::abs(a.green()-b.green()),std::abs(a.blue()-b.blue())});++glassSamples;
                    }
                    auto* crop=panel->findChild<QQuickItem*>("glassCrop");if(crop){const QSize texture=crop->property("textureSize").toSize();const QRectF region=panel->property("sampleRect").toRectF();if(texture.width()<qFloor(region.width()*window->devicePixelRatio())||texture.height()<qFloor(region.height()*window->devicePixelRatio())){QCoreApplication::exit(82);return;}}
                }
            };
            switch(studioPhase){
            case 0:controller.startSession();++studioPhase;break;
            case 1:if(!controller.grid())controller.setEffect("grid");studioClock.restart();++studioPhase;break;
            case 2:{
                if(studioClock.elapsed()<200)return;
                if(grid->parentItem()!=art->parentItem()||std::abs(grid->width()-art->width())>.01||std::abs(grid->height()-art->height())>.01){QCoreApplication::exit(83);return;}
                if(toolbar->width()>window->width()-16){QCoreApplication::exit(84);return;}
                QMetaObject::invokeMethod(studio,"zoomCentered",Q_ARG(QVariant,QVariant(1.7)));controller.setEffect("rotate");controller.setEffect("horizontal");studioClock.restart();++studioPhase;break;
            }
            case 3:{
                if(studioClock.elapsed()<500)return;window->grabWindow();
                const auto stroke=grid->property("stroke").toDouble(),zoom=studio->property("zoom").toDouble();
                const auto delta=grid->mapToScene(QPointF(stroke,0))-grid->mapToScene(QPointF(0,0));
                const double pixels=std::hypot(delta.x(),delta.y())*window->devicePixelRatio();
                if(std::abs(zoom-1.7)>.001||std::abs(pixels-1)>.01||std::abs(grid->width()-art->width())>.01){QCoreApplication::exit(85);return;}
                auto* line=findVisualItem(grid,"gridVertical-0");if(!line||line->childItems().size()!=2||line->childItems()[0]->property("color").value<QColor>()!=QColor(Qt::black)||line->childItems()[1]->property("color").value<QColor>()!=QColor(Qt::white)){QCoreApplication::exit(93);return;}
                studioMetrics["gridTwoTone"]=true;
                studioMetrics["gridStrokePixels"]=pixels;controller.togglePause();pausedProgress=studio->property("progressValue").toDouble();studioClock.restart();++studioPhase;break;
            }
            case 4:if(studioClock.elapsed()<250)return;if(std::abs(studio->property("progressValue").toDouble()-pausedProgress)>.00001){QCoreApplication::exit(86);return;}controller.togglePause();studioClock.restart();lastProgress=studio->property("progressValue").toDouble();++studioPhase;break;
            case 5:{
                const auto progress=studio->property("progressValue").toDouble();++progressSamples;if(progress-lastProgress>.000001)++progressChanges;
                if(progress<lastProgress-.00001){QCoreApplication::exit(87);return;}lastProgress=progress;
                if(studioClock.elapsed()<650)return;
                studioMetrics["progressSamples"]=progressSamples;studioMetrics["progressChanges"]=progressChanges;
                if(progressChanges<12){QCoreApplication::exit(88);return;}
                controller.togglePause();controller.resetEffects();controller.notify({});QMetaObject::invokeMethod(studio,"resetView");configureGlass();
                QEvent leave(QEvent::Leave);QCoreApplication::sendEvent(window,&leave);studioClock.restart();++studioPhase;break;
            }
            case 6:if(studioClock.elapsed()<300)return;glassBaseline=window->grabWindow();{QEnterEvent enter(QPointF(1,1),QPointF(1,1),QPointF(1,1));QCoreApplication::sendEvent(window,&enter);}studioClock.restart();++studioPhase;break;
            case 7:if(studioClock.elapsed()<350)return;compareGlass();window->resize(900,640);QMetaObject::invokeMethod(studio,"zoomCentered",Q_ARG(QVariant,QVariant(1.6)));studio->setProperty("targetPanX",73.0);studio->setProperty("targetPanY",-41.0);controller.setEffect("rotate");{QEvent leave(QEvent::Leave);QCoreApplication::sendEvent(window,&leave);}studioClock.restart();++studioPhase;break;
            case 8:if(studioClock.elapsed()<500)return;glassBaseline=window->grabWindow();{QEnterEvent enter(QPointF(1,1),QPointF(1,1),QPointF(1,1));QCoreApplication::sendEvent(window,&enter);}studioClock.restart();++studioPhase;break;
            case 9:{
                if(studioClock.elapsed()<350)return;compareGlass();
                studioMetrics["glassSamples"]=glassSamples;studioMetrics["glassMaxChannelError"]=glassMaxError;
                if(glassMaxError>4){qWarning("Glass alignment error: %d in %d samples",glassMaxError,glassSamples);window->grabWindow().save(QDir(directory).filePath("glass-failure.png"));glassBaseline.save(QDir(directory).filePath("glass-baseline.png"));QCoreApplication::exit(89);return;}
                auto* play=window->findChild<QQuickItem*>("sessionPlayButton");if(!play){QCoreApplication::exit(94);return;}const auto buttonSize=play->size();QTest::mouseMove(window,play->mapToScene(QPointF(play->width()/2,play->height()/2)).toPoint());if(!play->property("hovered").toBool()||play->size()!=buttonSize){QCoreApplication::exit(95);return;}studioMetrics["hoverGeometryStable"]=true;
                const auto size=art->size();controller.next();if(art->size()!=size||controller.imagePixelSize().isEmpty()){QCoreApplication::exit(90);return;}++studioPhase;break;
            }
            default:if(studio->property("progressValue").toDouble()>.01){QCoreApplication::exit(91);return;}QCoreApplication::quit();break;
            }
        });studioChecks.start();QTimer::singleShot(30000,&application,[&]{studioMetrics["timeoutPhase"]=studioPhase;if(auto* art=window->findChild<QQuickItem*>("referenceImage")){studioMetrics["imageStatus"]=art->property("status");studioMetrics["imageSourceSize"]=art->property("sourceSize");}QCoreApplication::exit(92);});
    }
    int timedPhase=0;
    QTimer timedNavigation;
    if(parser.isSet("timed-test")){
        auto testSettings=controller.settings();
        testSettings["display_time"]=1;testSettings["num_images"]=2;testSettings["use_break"]=true;
        testSettings["break_duration"]=1;testSettings["unlimited_time"]=false;
        testSettings["unlimited_images"]=false;testSettings["unlimited_break"]=false;testSettings["timer_volume"]=0;
        controller.applySettings(testSettings);
        timedNavigation.setInterval(100);
        QObject::connect(&timedNavigation,&QTimer::timeout,&controller,[&]{
            if(controller.scanning() || controller.imageCount()==0)return;
            if(controller.loading()){window->grabWindow();controller.framePresented();}
            if(timedPhase==0){controller.startSession();if(controller.active())timedPhase=1;return;}
            if(timedPhase==1 && controller.state()=="break"){
                if(controller.completed()!=2){QCoreApplication::exit(12);return;}timedPhase=2;
            }else if(timedPhase==2 && controller.state()=="running"){
                if(controller.completed()!=0)QCoreApplication::exit(13);else QCoreApplication::quit();
            }
        });
        timedNavigation.start();QTimer::singleShot(20000,&application,[]{QCoreApplication::exit(14);});
    }
    QTimer navigation;
    navigation.setInterval(parser.isSet("benchmark")?120:500);
    if(parser.isSet("smoke-test") || parser.isSet("benchmark")){
        QObject::connect(&navigation,&QTimer::timeout,&controller,[&]{
            if(controller.scanning())return;
            if(controller.imageCount()==0){if(++smokeSteps>30)QCoreApplication::exit(5);return;}
            if(!controller.active()){controller.startSession(true);return;}
            if(controller.loading()){window->grabWindow();controller.framePresented();return;}
            if(parser.isSet("benchmark")){
                controller.next();if(++benchmarkSteps>=100)QCoreApplication::quit();return;
            }
            switch(smokeSteps++){
            case 0:controller.togglePause();break;
            case 1:controller.openSettings();break;
            case 2:controller.closeSettings();if(!controller.paused())QCoreApplication::exit(10);break;
            case 3:controller.togglePause();controller.next();break;
            case 4:controller.previous();break;
            case 5:controller.next();break;
            case 6:controller.setEffect("bw");controller.setEffect("rotate");break;
            case 7:controller.setEffect("grid");controller.resetEffects();break;
            case 8:controller.skip();break;
            case 9:controller.setPage("history");break;
            default:QCoreApplication::quit();break;
            }
        });navigation.start();
        QTimer::singleShot(parser.isSet("benchmark")?90000:30000,&application,[]{QCoreApplication::exit(11);});
    }
    QObject::connect(&application,&QCoreApplication::aboutToQuit,&controller,[&]{
        navigation.stop();
        timedNavigation.stop();
        uiChecks.stop();
        collectionChecks.stop();
        flowChecks.stop();
        refinementChecks.stop();
        studioChecks.stop();
        QThreadPool::globalInstance()->waitForDone();
        if(parser.isSet("diagnostics")){
            auto diagnostic=controller.diagnostics();if(parser.isSet("refinement-test")){diagnostic["countdownTrace"]=cueTrace;diagnostic["countdownPauseMs"]=pauseLength;}
            if(parser.isSet("studio-test"))diagnostic["studioChecks"]=studioMetrics;
            if(parser.isSet("polish-test"))diagnostic["polishChecks"]=polishMetrics;
            if(parser.isSet("interaction-test"))diagnostic["interactionChecks"]=polishMetrics;
            if(parser.isSet("controls-test"))diagnostic["controlChecks"]=polishMetrics;
            if(parser.isSet("preview-test"))diagnostic["previewChecks"]=polishMetrics;
            if(parser.isSet("lobe-test"))diagnostic["lobeChecks"]=polishMetrics;
            if(parser.isSet("glass-test"))diagnostic["glassChecks"]=glassMetrics;
            QFile f(parser.value("diagnostics"));if(f.open(QIODevice::WriteOnly))f.write(QJsonDocument(QJsonObject::fromVariantMap(diagnostic)).toJson());
        }
        controller.stopSession();
    });
    return application.exec();
}
