#include "polish_checks.h"
#include "app/controller.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQuickItem>
#include <QEnterEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QtTest/QTest>
#include <memory>
#include <cmath>

namespace {
QQuickItem* visualItem(QQuickItem* root,const QString& name){
    if(root->objectName()==name)return root;
    for(auto* child:root->childItems())if(auto* found=visualItem(child,name))return found;
    return nullptr;
}
QQuickItem* item(QQuickWindow* window,const char* name){
    if(auto* found=window->findChild<QQuickItem*>(QString::fromLatin1(name)))return found;
    return visualItem(window->contentItem(),QString::fromLatin1(name));
}
void enter(QQuickWindow* window){QEnterEvent event({1,1},{1,1},{1,1});QCoreApplication::sendEvent(window,&event);}
void leave(QQuickWindow* window){QEvent event(QEvent::Leave);QCoreApplication::sendEvent(window,&event);}
bool click(QQuickWindow* window,const char* name){
    auto* button=item(window,name);if(!button||!button->isVisible()||!button->isEnabled())return false;
    const QPoint point=button->mapToScene({button->width()/2,button->height()/2}).toPoint();
    if(!QRect(QPoint(),window->size()).contains(point))return false;
    QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,point);return true;
}
void fail(QVariantMap* metrics,int code,const char* message){(*metrics)["failure"]=message;qWarning("%s",message);QCoreApplication::exit(code);}
}

void installInteractionChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics){
    struct State {int phase=0;QElapsedTimer clock;double frozen=0;int progressFrames=0;};
    auto state=std::make_shared<State>();state->clock.start();
    auto settings=controller->settings();settings["display_time"]=1;settings["num_images"]=1;settings["use_break"]=true;settings["break_duration"]=3;settings["timer_volume"]=0;settings["reduce_motion"]=false;controller->applySettings(settings);
    QObject::connect(controller,&Controller::visualProgressChanged,controller,[=]{if(controller->state()=="break"&&!controller->paused())++state->progressFrames;});
    auto* timer=new QTimer(controller);timer->setInterval(50);
    QObject::connect(timer,&QTimer::timeout,controller,[=]{
        if(controller->scanning()||controller->imageCount()==0)return;
        (*metrics)["phase"]=state->phase;
        if(controller->loading()&&controller->page()=="studio"){window->grabWindow();controller->framePresented();return;}
        auto* studio=item(window,"studioPage");if(!studio){fail(metrics,111,"Missing studio");return;}
        switch(state->phase){
        case 0:
            enter(window);controller->startSession(true);++state->phase;break;
        case 1:{
            QMetaObject::invokeMethod(studio,"resetView");
            QMetaObject::invokeMethod(studio,"zoomCentered",Q_ARG(QVariant,QVariant(1.1)));
            QMetaObject::invokeMethod(studio,"zoomCentered",Q_ARG(QVariant,QVariant(1.1)));
            if(std::abs(studio->property("targetZoom").toDouble()-1.21)>.001){fail(metrics,112,"Wheel burst loses steps");return;}
            state->clock.restart();++state->phase;break;
        }
        case 2:{
            if(state->clock.elapsed()<250)return;
            if(std::abs(studio->property("zoom").toDouble()-1.21)>.001){fail(metrics,112,"Zoom does not converge to target");return;}
            const QPointF point(window->width()/2.0,window->height()/2.0);
            QWheelEvent horizontal(point,window->mapToGlobal(point.toPoint()),{},QPoint(120,0),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);
            QCoreApplication::sendEvent(window,&horizontal);
            if(std::abs(studio->property("targetZoom").toDouble()-1.21)>.001){fail(metrics,112,"Horizontal wheel changes zoom");return;}
            (*metrics)["burstZoom"]=1.21;(*metrics)["horizontalWheelIgnored"]=true;
            controller->openSettings();leave(window);state->clock.restart();++state->phase;break;
        }
        case 3:{
            if(state->clock.elapsed()<250)return;
            auto* drawer=window->findChild<QObject*>("settingsDrawer");
            if(!drawer||!drawer->property("opened").toBool()||drawer->property("opacity").toDouble()<.99){fail(metrics,113,"Active settings fade away");return;}
            if(auto* category=item(window,"settingsCategory");category&&category->isVisible()){
                category->forceActiveFocus();QTest::keyClick(window,Qt::Key_Space);QTest::keyClick(window,Qt::Key_Down);QTest::keyClick(window,Qt::Key_Return);
                if(drawer->property("section").toInt()!=1){fail(metrics,113,"Settings category keyboard navigation fails");return;}
                (*metrics)["settingsCategoryKeyboard"]=true;
            }
            (*metrics)["settingsRemainVisibleOutside"]=true;QMetaObject::invokeMethod(drawer,"close");enter(window);
            controller->stopSession();controller->startSession();++state->phase;break;
        }
        case 4:
            if(controller->state()!="break")return;controller->togglePause();state->frozen=controller->progress();state->clock.restart();++state->phase;break;
        case 5:
            if(state->clock.elapsed()<600)return;
            if(controller->state()!="break"||!controller->paused()||std::abs(state->frozen-controller->progress())>.001){fail(metrics,114,"Break pause advances timer");return;}
            controller->togglePause();state->clock.restart();state->progressFrames=0;++state->phase;break;
        case 6:
            if(state->clock.elapsed()<600)return;
            if(state->progressFrames<20){fail(metrics,114,"Break progress is not smooth");return;}
            (*metrics)["breakProgressSignals600ms"]=state->progressFrames;(*metrics)["breakPausePreservesTime"]=true;
            controller->skip();state->clock.restart();++state->phase;break;
        default:
            if(controller->state()!="running")return;(*metrics)["skipBreakRestarts"]=true;QCoreApplication::quit();break;
        }
    });timer->start();QTimer::singleShot(15000,controller,[=]{fail(metrics,115,"Interaction checks timed out");});
}

void installLobeChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics){
    struct State {int phase=0;QElapsedTimer clock;};auto state=std::make_shared<State>();state->clock.start();
    auto* timer=new QTimer(controller);timer->setInterval(100);
    QObject::connect(timer,&QTimer::timeout,controller,[=]{
        if(controller->scanning()||controller->imageCount()==0)return;
        if(controller->loading()&&controller->page()=="studio"){window->grabWindow();controller->framePresented();return;}
        (*metrics)["phase"]=state->phase;
        switch(state->phase){
        case 0:{
            enter(window);controller->setPage("home");controller->setPresetCount(10);
            auto* preset=item(window,"homeCountPreset10");if(!preset){fail(metrics,121,"Missing segmented preset");return;}
            preset->forceActiveFocus();QTest::keyClick(window,QGuiApplication::layoutDirection()==Qt::RightToLeft?Qt::Key_Left:Qt::Key_Right);
            if(controller->settings().value("num_images").toInt()!=20){fail(metrics,122,"Segment arrow navigation fails");return;}
            QTest::keyClick(window,Qt::Key_Home);if(controller->settings().value("num_images").toInt()!=5){fail(metrics,122,"Segment Home navigation fails");return;}
            (*metrics)["segmentedKeyboard"]=true;controller->startSession(true);++state->phase;break;
        }
        case 1:controller->stopSession();controller->setPage("history");++state->phase;break;
        case 2:{
            if(controller->history().isEmpty())return;
            auto* history=item(window,"historyPage");auto* list=item(window,"historyList");
            if(!history||!list||list->property("count").toInt()==0){fail(metrics,123,"History list empty after browsing");return;}
            history->setProperty("kind","completed");state->clock.restart();++state->phase;break;
        }
        case 3:{
            auto* history=item(window,"historyPage");auto* list=item(window,"historyList");
            if(list->property("count").toInt()!=0){fail(metrics,123,"History kind filter fails");return;}
            history->setProperty("kind","viewed");auto* search=item(window,"historySearch");search->setProperty("text","no-such-reference-qa");state->clock.restart();++state->phase;break;
        }
        case 4:{
            auto* list=item(window,"historyList");auto* search=item(window,"historySearch");
            if(list->property("count").toInt()!=0){fail(metrics,124,"History search fails");return;}
            search->forceActiveFocus();QTest::keyClick(window,Qt::Key_Escape);++state->phase;break;
        }
        case 5:{
            auto* history=item(window,"historyPage");auto* list=item(window,"historyList");
            if(list->property("count").toInt()==0||item(window,"historySearch")->property("text").toString()!=""){fail(metrics,124,"Search Escape does not clear filters");return;}
            QVariant label;QVariantMap legacy{{"kind","legacy"},{"at",QVariant{}},{"path","legacy.png"}};
            if(!QMetaObject::invokeMethod(history,"eventDate",Q_RETURN_ARG(QVariant,label),Q_ARG(QVariant,QVariant(legacy)))||label.toString().isEmpty()){fail(metrics,125,"Imported dates lack a label");return;}
            (*metrics)["historyKindAndSearch"]=true;(*metrics)["legacyDateLabel"]=label;
            auto settings=controller->settings();settings["save_history"]=false;controller->applySettings(settings);controller->setPage("home");++state->phase;break;
        }
        case 6:{
            auto* notice=item(window,"homeStorageNotice");auto* action=notice?notice->findChild<QQuickItem*>("noticeAction"):nullptr;
            if(!notice||!notice->isVisible()||!action||!QMetaObject::invokeMethod(action,"clicked")){fail(metrics,126,"Storage notice has no working action");return;}
            state->clock.restart();++state->phase;break;
        }
        default:{
            if(state->clock.elapsed()<200)return;auto* drawer=window->findChild<QObject*>("settingsDrawer");
            if(!drawer||!drawer->property("opened").toBool()||drawer->property("section").toInt()!=4){fail(metrics,126,"Storage notice opens the wrong settings");return;}
            (*metrics)["storageAction"]=true;QCoreApplication::quit();break;
        }
        }
    });timer->start();QTimer::singleShot(20000,controller,[=]{fail(metrics,127,"Lobe UI test timed out");});
}

void installPolishChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics){
    struct State {int phase=0,combination=0;QElapsedTimer clock;};
    auto state=std::make_shared<State>();state->clock.start();
    auto settings=controller->settings();settings["display_time"]=60;settings["num_images"]=1;settings["unlimited_time"]=false;settings["unlimited_images"]=false;settings["use_break"]=false;settings["timer_volume"]=0;settings["reduce_motion"]=true;settings["grid_h_lines"]=1;settings["grid_v_lines"]=3;controller->applySettings(settings);
    auto* timer=new QTimer(controller);timer->setInterval(100);
    QObject::connect(timer,&QTimer::timeout,controller,[=]{
        if(controller->scanning()||controller->imageCount()==0)return;
        if(controller->loading()&&controller->page()=="studio"){window->grabWindow();controller->framePresented();return;}
        auto* studio=item(window,"studioPage");auto* art=item(window,"referenceImage");auto* grid=item(window,"imageGrid");
        auto* toolbar=item(window,"sessionToolbar");auto* result=item(window,"sessionResult");
        if(!studio||!art||!grid||!toolbar||!result){fail(metrics,101,"Missing session objects");return;}
        switch(state->phase){
        case 0:enter(window);controller->startSession();if(controller->active())++state->phase;break;
        case 1:{
            if(state->combination==16){(*metrics)["transformCombinations"]=16;++state->phase;break;}
            const int combination=state->combination;controller->resetEffects();
            for(int r=0;r<combination/4;++r)controller->setEffect("rotate");
            if(combination&1)controller->setEffect("horizontal");if(combination&2)controller->setEffect("vertical");
            if(!controller->grid())controller->setEffect("grid");QMetaObject::invokeMethod(studio,"resetView");
            studio->setProperty("targetZoom",1.6);studio->setProperty("targetPanX",23.0);studio->setProperty("targetPanY",-17.0);
            window->grabWindow();
            for(const QPointF uv:{QPointF(0,0),QPointF(1,0),QPointF(0,1),QPointF(1,1),QPointF(.25,.5)}){
                const auto delta=grid->mapToScene({uv.x()*grid->width(),uv.y()*grid->height()})-art->mapToScene({uv.x()*art->width(),uv.y()*art->height()});
                if(std::hypot(delta.x(),delta.y())>.001){fail(metrics,102,"Grid leaves image after reflection/rotation");return;}
            }
            const auto d=grid->mapToScene({grid->property("stroke").toDouble(),0})-grid->mapToScene({0,0});
            if(std::abs(std::hypot(d.x(),d.y())*window->devicePixelRatio()-1)>.01){fail(metrics,102,"Grid stroke changes with transform");return;}
            ++state->combination;break;
        }
        case 2:{
            const QStringList buttons{"sessionBwButton","sessionFlipHButton","sessionFlipVButton","sessionRotateButton","sessionGridButton","sessionResetButton","sessionPreviousButton","sessionPlayButton","sessionNextButton","sessionSkipButton","sessionFitButton","sessionSettingsButton","sessionCopyButton","sessionOpenButton","sessionDeleteButton","sessionFullscreenButton"};
            for(const auto& name:buttons){auto* button=window->findChild<QQuickItem*>(name);if(!button||!button->isVisible()||button->width()<24||button->height()<24){fail(metrics,103,"Session command is hidden or too small");return;}
                const auto bounds=button->mapRectToScene(QRectF(QPointF(),button->size()));if(!QRectF(QPointF(),window->size()).contains(bounds)){fail(metrics,103,"Exposed toolbar leaves window");return;}}
            (*metrics)["exposedCommands"]=buttons.size();(*metrics)["toolbarRows"]=toolbar->property("singleRow").toBool()?1:toolbar->property("doubleRow").toBool()?2:3;
            auto upcoming=controller->settings();upcoming["display_time"]=30;controller->applySettings(upcoming);controller->notify({});
            if(!click(window,"sessionMenuButton")){fail(metrics,104,"Cannot open session popover");return;}state->clock.restart();++state->phase;break;
        }
        case 3:{
            if(state->clock.elapsed()<200)return;auto* popup=window->findChild<QObject*>("sessionPopover");
            if(!popup||!popup->property("opened").toBool()||popup->property("durationMs").toLongLong()!=60000){fail(metrics,104,"Popover shows the wrong active rhythm");return;}
            auto* content=popup->property("contentItem").value<QQuickItem*>();if(!content||!QRectF(QPointF(),window->size()).contains(content->mapRectToScene(QRectF(QPointF(),content->size())))){fail(metrics,104,"Popover leaves window");return;}
            QMetaObject::invokeMethod(popup,"close");(*metrics)["activeRhythmPreserved"]=true;controller->next();state->clock.restart();++state->phase;break;
        }
        case 4:{
            if(state->clock.elapsed()<200)return;window->grabWindow();
            if(controller->state()!="completed"||!result->isVisible()||toolbar->isVisible()||!item(window,"sessionContext")->isVisible()||item(window,"sessionTimer")->isVisible()){fail(metrics,105,"Completion repeats active-session controls");return;}
            leave(window);state->clock.restart();++state->phase;break;
        }
        case 5:{
            if(state->clock.elapsed()<300)return;if(!result->isVisible()||result->opacity()<.99){fail(metrics,105,"Result disappears when pointer leaves");return;}
            enter(window);
            if(!click(window,"resultHideButton")||result->isVisible()||!toolbar->isVisible()){fail(metrics,106,"Cannot hide session results");return;}
            if(!click(window,"sessionMenuButton")||!result->isVisible()){fail(metrics,106,"Cannot reopen session results");return;}
            (*metrics)["hideAndReopenResults"]=true;
            if(!QMetaObject::invokeMethod(item(window,"resultCountPreset5"),"clicked")||controller->settings().value("num_images").toInt()!=5){fail(metrics,106,"Result count presets failed");return;}
            (*metrics)["resultCountPresets"]=true;
            auto nextSettings=controller->settings();nextSettings["num_images"]=1;controller->applySettings(nextSettings);controller->notify({});
            if(!click(window,"sessionReplayButton")){fail(metrics,106,"Replay is outside compact result viewport");return;}++state->phase;break;
        }
        case 6:
            if(controller->state()!="running"||controller->sessionDurationMs()!=30000){fail(metrics,106,"Replay does not apply the next rhythm");return;}
            (*metrics)["replayAppliedNextRhythm"]=true;
            if(!controller->canPrevious()){fail(metrics,106,"History was reset by replay");return;}
            controller->previous();if(controller->state()!="review"){fail(metrics,106,"Shared history does not pause the session");return;}
            window->grabWindow();controller->ready(controller->requestId());controller->framePresented();controller->next();
            window->grabWindow();controller->ready(controller->requestId());controller->framePresented();
            if(controller->state()!="running"||controller->completed()!=0){fail(metrics,106,"Shared history corrupts completion or resume");return;}
            (*metrics)["sharedHistoryAcrossSessions"]=true;controller->next();state->clock.restart();++state->phase;break;
        case 7:
            if(state->clock.elapsed()<200)return;if(!QMetaObject::invokeMethod(item(window,"sessionResultPracticeButton"),"clicked")||controller->page()!="practice"){fail(metrics,105,"Result practice action failed");return;}
            controller->setPage("studio");if(!QMetaObject::invokeMethod(item(window,"sessionResultHomeButton"),"clicked")||controller->page()!="home"){fail(metrics,105,"Result home action failed");return;}
            (*metrics)["completionNavigation"]=true;enter(window);controller->startSession();++state->phase;break;
        case 8:
            QMetaObject::invokeMethod(window->findChild<QObject*>("sessionPopover"),"open");++state->phase;break;
        case 9:
            if(!QMetaObject::invokeMethod(item(window,"sessionModeButton"),"clicked")){fail(metrics,107,"Free view action missing");return;}++state->phase;break;
        case 10:
            if(!controller->freeView()){fail(metrics,107,"Free view action does not switch modes");return;}QMetaObject::invokeMethod(window->findChild<QObject*>("sessionPopover"),"open");++state->phase;break;
        case 11:QMetaObject::invokeMethod(item(window,"sessionModeButton"),"clicked");++state->phase;break;
        case 12:
            if(controller->freeView()){fail(metrics,107,"Timed practice action does not switch modes");return;}QMetaObject::invokeMethod(window->findChild<QObject*>("sessionPopover"),"open");++state->phase;break;
        case 13:
            QMetaObject::invokeMethod(item(window,"sessionStopButton"),"clicked");if(controller->active()){fail(metrics,107,"End session action failed");return;}
            (*metrics)["popoverModesAndStop"]=true;controller->setPage("home");
            if(!QMetaObject::invokeMethod(item(window,"homeCountPreset20"),"clicked")||controller->settings().value("num_images").toInt()!=20){fail(metrics,107,"Home count preset failed");return;}
            QMetaObject::invokeMethod(item(window,"homeFoldersButton"),"clicked");
            if(auto* home=item(window,"homeScroll"))if(auto* flick=home->property("contentItem").value<QQuickItem*>())flick->setProperty("contentY",240);
            ++state->phase;break;
        case 14:{
            const QString name="homeFolder"+controller->collections().first().toMap().value("id").toString();
            auto* folder=item(window,name.toUtf8().constData());if(!folder)return;
            QMetaObject::invokeMethod(folder,"clicked");if(controller->sessionImageCount()!=0){fail(metrics,107,"Home folder deselection failed");return;}
            ++state->phase;break;
        }
        default:{
            const QString name="homeFolder"+controller->collections().first().toMap().value("id").toString();
            auto* folder=item(window,name.toUtf8().constData());if(!folder)return;
            QMetaObject::invokeMethod(folder,"clicked");if(controller->sessionImageCount()==0){fail(metrics,107,"Home folder selection failed");return;}
            (*metrics)["homeCountPresetsAndFolders"]=true;QCoreApplication::quit();break;
        }
        }
    });timer->start();QTimer::singleShot(60000,controller,[=]{fail(metrics,108,"Session polish test timed out");});
}

void installGlassChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics){
    struct State {int phase=0;QElapsedTimer clock;QImage baseline;};auto state=std::make_shared<State>();state->clock.start();
    auto settings=controller->settings();settings["display_time"]=60;settings["num_images"]=2;settings["timer_volume"]=0;settings["unlimited_time"]=false;settings["unlimited_images"]=false;settings["use_break"]=false;settings["reduce_motion"]=true;settings["session_glass"]=true;settings["session_blur"]=true;settings["hide_timer_on_leave"]=true;controller->applySettings(settings);
    auto* timer=new QTimer(controller);timer->setInterval(100);
    QObject::connect(timer,&QTimer::timeout,controller,[=]{
        if(controller->scanning()||controller->imageCount()==0)return;
        if(controller->loading()&&controller->page()=="studio"){window->grabWindow();controller->framePresented();return;}
        switch(state->phase){
        case 0:controller->startSession();enter(window);if(controller->active())++state->phase;break;
        case 1:
            if(controller->softwareRenderer()){fail(metrics,111,"Native blur test requires GPU rendering");return;}
            controller->togglePause();controller->notify({});if(controller->grid())controller->setEffect("grid");
            for(const char* name:{"sessionContextContent","sessionWindowContent","sessionToolbarContent","sessionTimerContent"})item(window,name)->setOpacity(0);
            for(const char* name:{"sessionContext","sessionWindowActions","sessionToolbar","sessionTimer"}){auto* panel=item(window,name);panel->setProperty("surfaceOpacity",0.0);panel->setProperty("blurRadius",12.0);}
            leave(window);state->clock.restart();++state->phase;break;
        case 2:if(state->clock.elapsed()<300)return;state->baseline=window->grabWindow();enter(window);state->clock.restart();++state->phase;break;
        default:{
            if(state->clock.elapsed()<300)return;const QImage actual=window->grabWindow();const qreal dpr=qreal(actual.width())/window->width();
            QVariantList panels;
            for(const char* name:{"sessionContext","sessionWindowActions","sessionToolbar","sessionTimer"}){
                auto* panel=item(window,name);const QRect bounds=panel->mapRectToScene(QRectF(QPointF(),panel->size())).toAlignedRect();
                const QRect area(qRound((bounds.x()+8)*dpr),qRound((bounds.y()+8)*dpr),qRound((bounds.width()-16)*dpr),qRound((bounds.height()-16)*dpr));
                double raw=0,blurred=0;int pairs=0;
                for(int y=area.top();y<area.bottom();++y)for(int x=area.left();x<area.right();++x){
                    for(const QPoint delta:{QPoint(1,0),QPoint(0,1)}){
                        const QPoint a(x,y),b=a+delta;raw+=std::abs(qGray(state->baseline.pixel(a))-qGray(state->baseline.pixel(b)));blurred+=std::abs(qGray(actual.pixel(a))-qGray(actual.pixel(b)));++pairs;
                    }
                }
                if(pairs==0||raw/pairs<20){fail(metrics,112,"Checker pattern does not cover the glass sample");return;}
                const double ratio=blurred/raw;panels.append(QVariantMap{{"panel",name},{"edgeVariationRatio",ratio}});
                if(ratio>.05){(*metrics)["panels"]=panels;fail(metrics,113,"Blur leaves high-frequency pixel grain");return;}
            }
            (*metrics)["panels"]=panels;(*metrics)["windowDpr"]=window->devicePixelRatio();(*metrics)["densePatternSmoothed"]=true;QCoreApplication::quit();break;
        }
        }
    });timer->start();QTimer::singleShot(30000,controller,[=]{fail(metrics,114,"Native blur test timed out");});
}

void installPreviewChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics){
    struct State {int phase=0;QString path,source;quint64 request=0;qint64 remaining=0;int ordinal=0;QElapsedTimer wait;};
    auto state=std::make_shared<State>();auto* timer=new QTimer(controller);timer->setInterval(100);
    auto settings=controller->settings();settings["display_time"]=60;settings["num_images"]=5;settings["unlimited_time"]=false;settings["unlimited_images"]=false;settings["use_break"]=false;settings["timer_volume"]=0;controller->applySettings(settings);
    QObject::connect(timer,&QTimer::timeout,controller,[=]{
        if(controller->scanning()||controller->imageCount()==0||controller->historyLoading())return;
        if(controller->loading()&&controller->page()=="studio"){window->grabWindow();controller->framePresented();return;}
        window->grabWindow(); // Flush layouts when native QA starts the window hidden.
        (*metrics)["phase"]=state->phase;
        switch(state->phase){
        case 0:controller->startSession();++state->phase;break;
        case 1:
            if(!controller->active()){fail(metrics,131,"Session did not start");return;}
            state->path=controller->currentPath();state->source=controller->currentImage();state->request=controller->requestId();state->ordinal=controller->ordinal();
            controller->setEffect("horizontal");controller->setPage("library");state->remaining=controller->remainingMs();controller->openMedia(state->path);controller->stepPreview(-1);state->wait.start();++state->phase;break;
        case 2:{
            if(state->wait.elapsed()<250)return;
            auto* image=item(window,"previewImage");
            if(!image||image->property("status").toInt()==2)return;
            if(image->property("status").toInt()!=1){fail(metrics,132,"Preview image failed to load");return;}
            if(controller->page()!="library"||!controller->active()||controller->currentPath()!=state->path||controller->currentImage()!=state->source||controller->requestId()!=state->request||!controller->flipH()){
                fail(metrics,133,"Collection preview changed the session");return;}
            (*metrics)["collectionPreviewIsolated"]=true;
            if(controller->previewCount()>1){const auto index=controller->previewIndex();if(!click(window,"previewNext")){auto* button=item(window,"previewNext");(*metrics)["previewIndex"]=controller->previewIndex();(*metrics)["previewCount"]=controller->previewCount();if(button){(*metrics)["buttonVisible"]=button->isVisible();(*metrics)["buttonEnabled"]=button->isEnabled();const auto pos=button->mapToScene({0,0});(*metrics)["buttonPosition"]=QVariantList{pos.x(),pos.y()};(*metrics)["buttonSize"]=QVariantList{button->width(),button->height()};(*metrics)["windowSize"]=QVariantList{window->width(),window->height()};}else (*metrics)["missingButton"]=true;window->grabWindow().save(QCoreApplication::applicationDirPath()+"/failure.png");fail(metrics,134,"Preview next button unavailable");return;}if(controller->previewIndex()!=index+1){fail(metrics,134,"Preview navigation failed");return;}controller->stepPreview(-1);}
            image->forceActiveFocus();QTest::keyClick(window,Qt::Key_Escape);state->wait.start();++state->phase;break;
        }
        case 3:
            if(state->wait.elapsed()<200)return;
            if(!controller->previewPath().isEmpty()||controller->page()!="library"){fail(metrics,135,"Escape did not return to collection");return;}
            if(std::abs(controller->remainingMs()-state->remaining)>100){fail(metrics,136,"Session timer ran while browsing");return;}
            controller->setPage("history");controller->openMedia(state->path);++state->phase;break;
        case 4:
            if(controller->page()!="history"||!controller->active()||controller->ordinal()!=state->ordinal){fail(metrics,137,"History preview changed the session");return;}
            if(!click(window,"closePreviewButton")){fail(metrics,138,"Close preview button unavailable");return;}
            (*metrics)["historyPreviewIsolated"]=true;controller->setPage("studio");state->wait.restart();++state->phase;break;
        case 5:
            if(state->wait.elapsed()<350)return;
            if(controller->currentPath()!=state->path||controller->requestId()!=state->request||controller->remainingMs()>=state->remaining||!controller->flipH()){
                fail(metrics,139,"Returning to session did not restore and resume");return;}
            (*metrics)["sessionResumed"]=true;(*metrics)["escapeReturnsToOrigin"]=true;
            controller->togglePause();state->remaining=controller->remainingMs();controller->setPage("library");controller->openMedia(state->path);state->wait.restart();++state->phase;break;
        case 6:
            if(state->wait.elapsed()<250)return;
            controller->closePreview();controller->setPage("studio");state->wait.restart();++state->phase;break;
        default:
            if(state->wait.elapsed()<250)return;
            if(!controller->paused()||std::abs(controller->remainingMs()-state->remaining)>100){fail(metrics,141,"Browsing discarded the manual pause");return;}
            (*metrics)["manualPausePreserved"]=true;QCoreApplication::quit();break;
        }
    });timer->start();QTimer::singleShot(30000,controller,[=]{fail(metrics,140,"Preview isolation test timed out");});
}

void installControlChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics){
    struct State {int phase=0,position=0,fitStep=0;QElapsedTimer wait;};auto state=std::make_shared<State>();
    auto settings=controller->settings();settings["display_time"]=60;settings["num_images"]=20;settings["unlimited_time"]=false;settings["unlimited_images"]=false;settings["use_break"]=false;settings["timer_volume"]=0;settings["reduce_motion"]=true;controller->applySettings(settings);
    auto* timer=new QTimer(controller);timer->setInterval(100);
    QObject::connect(timer,&QTimer::timeout,controller,[=]{
        if(controller->scanning()||controller->imageCount()==0||controller->historyLoading())return;
        window->grabWindow();controller->framePresented();
        auto* studio=item(window,"studioPage");auto* fit=item(window,"sessionFitButton");auto* badge=item(window,"sessionTimer");
        (*metrics)["phase"]=state->phase;
        switch(state->phase){
        case 0:window->showNormal();window->requestActivate();if(!QTest::qWaitForWindowActive(window,3000)){fail(metrics,164,"Keyboard test window could not activate");return;}controller->startSession();++state->phase;break;
        case 1:{
            if(controller->loading())return;
            double last=-1;
            for(const char* name:{"sessionPreviousButton","sessionPlayButton","sessionSkipButton","sessionNextButton"}){
                auto* button=item(window,name);const auto point=button->mapToScene({0,0});
                if(point.x()<=last||!button->property("quiet").toBool()||button->property("primary").toBool()){fail(metrics,151,"Navigation order or transparency differs");return;}last=point.x();
            }
            if(item(window,"sessionResetButton")->x()>=fit->x()){fail(metrics,152,"Reset was not restored before Fit");return;}
            if(fit->property("selected").toBool()){fail(metrics,153,"Default fitted image must have a neutral button");return;}
            (*metrics)["toolbarOrderAndTransparentButtons"]=true;if(!click(window,"sessionFitButton")){fail(metrics,154,"Cannot click actual-size button");return;}++state->phase;break;
        }
        case 2:{
            auto* toolbar=item(window,"sessionToolbar");
            if(state->fitStep==0||state->fitStep==1||state->fitStep==3){
                if(!fit->property("selected").toBool()||toolbar->property("viewPercent").toInt()!=100){fail(metrics,154,"Actual-size command must highlight the button and show 100 percent");return;}
            }
            if(state->fitStep==0){window->resize(520,600);++state->fitStep;break;}
            if(state->fitStep==1){if(!click(window,"sessionFitButton")){fail(metrics,154,"Cannot toggle back to fit");return;}++state->fitStep;break;}
            if(state->fitStep==2){
                if(fit->property("selected").toBool()||!studio->property("fitMode").toBool()){fail(metrics,154,"Second click must restore a neutral fitted view");return;}
                studio->forceActiveFocus();QTest::keyClick(window,Qt::Key_1,Qt::ControlModifier);++state->fitStep;break;
            }
            if(state->fitStep==3){QMetaObject::invokeMethod(studio,"zoomCentered",Q_ARG(QVariant,QVariant(1.1)));++state->fitStep;break;}
            if(fit->property("selected").toBool()){fail(metrics,154,"Manual zoom leaves actual-size button highlighted");return;}
            studio->forceActiveFocus();QTest::keyClick(window,Qt::Key_0,Qt::ControlModifier);++state->phase;break;
        }
        case 3:
            if(fit->property("selected").toBool()||!studio->property("fitMode").toBool()){fail(metrics,155,"Ctrl+0 must restore a neutral fitted view");return;}
            studio->forceActiveFocus();QTest::keyClick(window,Qt::Key_B);QTest::keyClick(window,Qt::Key_H);QTest::keyClick(window,Qt::Key_V);QTest::keyClick(window,Qt::Key_R);QTest::keyClick(window,Qt::Key_G);++state->phase;break;
        case 4:
            if(!controller->bw()||!controller->flipH()||!controller->flipV()||controller->rotation()!=90||!controller->grid()){fail(metrics,156,"Effect shortcuts do not match commands");return;}
            QTest::keyClick(window,Qt::Key_Backspace);++state->phase;break;
        case 5:
            if(controller->bw()||controller->flipH()||controller->flipV()||controller->rotation()!=0||!controller->grid()){fail(metrics,157,"Backspace must reset the image and preserve the grid guide");return;}
            QTest::keyClick(window,Qt::Key_Equal,Qt::ControlModifier);++state->phase;break;
        case 6:
            if(studio->property("targetZoom").toDouble()<=1){fail(metrics,158,"Ctrl+= zoom shortcut failed");return;}
            QTest::keyClick(window,Qt::Key_0,Qt::ControlModifier);QTest::keyClick(window,Qt::Key_Space);++state->phase;break;
        case 7:
            if(!controller->paused()){fail(metrics,159,"Space does not pause");return;}
            QTest::keyClick(window,Qt::Key_Space);++state->phase;break;
        case 8:
            if(controller->paused()){fail(metrics,159,"Space does not resume");return;}
            QTest::keyClick(window,Qt::Key_Comma,Qt::ControlModifier);++state->phase;break;
        case 9:
            QTest::keyClick(window,Qt::Key_B);++state->phase;break;
        case 10:
            if(controller->bw()){fail(metrics,160,"Session shortcut fires in settings");return;}
            if(auto* sheet=window->findChild<QObject*>("settingsDrawer"))QMetaObject::invokeMethod(sheet,"close");
            QTest::keyClick(window,Qt::Key_Escape);controller->closeSettings();(*metrics)["keyboardCommandsAndModalScope"]=true;++state->phase;break;
        case 11:{
            auto s=controller->settings();s["timer_position"]=QStringList{"Left","Center","Right"}[state->position/2];controller->applySettings(s);
            window->resize(state->position%2?800:360,440);state->wait.restart();++state->phase;break;
        }
        default:{
            if(state->wait.elapsed()<150)return;
            const auto p=badge->mapToScene({0,0});const auto position=controller->settings().value("timer_position").toString();
            const double expected=position=="Left"?16:position=="Right"?window->width()-badge->width()-16:(window->width()-badge->width())/2;
            if(std::abs(p.y()-16)>1||std::abs(p.x()-expected)>1){fail(metrics,161,"Timer anchor shifts on resize");return;}
            for(const char* name:{"sessionContext","sessionWindowActions"}){
                auto* panel=item(window,name);if(badge->mapRectToScene(QRectF(QPointF(),badge->size())).intersects(panel->mapRectToScene(QRectF(QPointF(),panel->size())))){fail(metrics,162,"Timer overlaps top controls");return;}}
            if(++state->position<6){state->phase=11;break;}
            (*metrics)["sixTimerAnchorsWithoutOverlap"]=true;(*metrics)["neutralFitAndActualSizeToggle"]=true;QCoreApplication::quit();break;
        }
        }
    });timer->start();QTimer::singleShot(30000,controller,[=]{fail(metrics,163,"Controls check timed out");});
}
