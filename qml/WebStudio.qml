import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine
Item {
    id:root;objectName:"websiteWorkspace"
    property string provider:"quickposes"
    property var metrics:({})
    property string loadError:""
    readonly property bool chromeVisible:app.pointerInsideWindow||websiteMenu.opened||sitePopup.opened
    function updateFocus(){
        page.runJavaScript("window.gestartFocus && window.gestartFocus("+(!root.chromeVisible&&root.visible)+")")
    }
    onChromeVisibleChanged:updateFocus()
    onVisibleChanged:updateFocus()
    Connections {target:app;function onPointerInsideChanged(){root.updateFocus()}}
    readonly property bool compact:width<560||height<520
    signal providerRequested(string provider)
    signal navigation()
    signal downloader()
    signal settings()
    signal fullScreenRequested(bool enabled)
    readonly property var sites:[{id:"quickposes",name:"Quickposes"},{id:"proko",name:"Proko"},{id:"poselibrary",name:"Pose Library"},{id:"sketchdaily",name:"SketchDaily"}]
    function prepareFocusTest(){
        page.runJavaScript("(()=>{const c=document.createElement('canvas');c.width=300;c.height=400;const x=c.getContext('2d');x.fillStyle='tomato';x.fillRect(0,0,300,400);const i=document.createElement('img');i.src=c.toDataURL();i.style.cssText='position:fixed;left:0;top:0;width:300px;height:400px;z-index:2147483646';document.body.appendChild(i)})()")
    }
    function inspectFocusTest(){
        page.runJavaScript("(()=>{const f=document.getElementById('gestart-focus');return {hidden:document.documentElement.hasAttribute('data-gestart-hidden'),image:f&&getComputedStyle(f).display==='flex'}})()",result=>{
            if(result)root.metrics=Object.assign({},root.metrics,{focusHidden:result.hidden,focusImage:result.image,chromeVisible:root.chromeVisible,viewportHeight:page.height,viewportTop:page.y,workspaceHeight:root.height})
        })
    }
    function reload(){page.reload()}
    function safe(url){return url.toString()==="about:blank"||/^https:\/\//i.test(url.toString())}
    function leaveFullScreen(){page.triggerWebAction(WebEngineView.ExitFullScreen)}
    WebEngineProfile {
        id:siteProfile;storageName:"reference-sites";offTheRecord:storageName.length===0
        persistentStoragePath:webStoragePath+"/storage";cachePath:webStoragePath+"/cache"
        httpCacheMaximumSize:67108864
        persistentCookiesPolicy:WebEngineProfile.AllowPersistentCookies
        onDownloadRequested:download=>{download.cancel();app.notify(Theme.t("Use the reference downloader to save images."))}
    }
    ColumnLayout {
        anchors.fill:parent;spacing:0
        RowLayout {
            visible:!page.isFullScreen&&root.chromeVisible;enabled:root.chromeVisible;Layout.fillWidth:true;Layout.margins:root.compact?4:8;spacing:4
            IconButton {objectName:"websiteHome";width:root.compact?28:36;height:width;iconName:"panel-left";hint:Theme.t("Home");onClicked:root.navigation()}
            LobeComboBox {
                objectName:"websitePicker";Layout.fillWidth:true;Layout.maximumWidth:root.compact?10000:240
                model:root.sites.map(s=>s.name);currentIndex:Math.max(0,root.sites.findIndex(s=>s.id===root.provider))
                onActivated:root.providerRequested(root.sites[currentIndex].id)
                Accessible.name:Theme.t("Websites")
            }
            IconButton {visible:!root.compact;iconName:page.loading?"x":"rotate-cw";hint:Theme.t(page.loading?"Cancel":"Reload");onClicked:{if(page.loading)page.stop();else page.reload()}}
            LobeButton {objectName:"websiteDownloader";visible:!root.compact;implicitHeight:36;text:Theme.t("Download references");iconName:"download";onClicked:root.downloader()}
            IconButton {id:more;width:root.compact?28:36;height:width;iconName:"ellipsis";hint:Theme.t("Settings");onClicked:websiteMenu.open()
                LobeMenu {id:websiteMenu;y:more.height+4;x:more.width-width;width:228
                    LobeMenuItem {visible:root.compact;text:Theme.t("Reload");onTriggered:page.reload()}
                    LobeMenuItem {visible:root.compact;text:Theme.t("Download references");onTriggered:root.downloader()}
                    LobeMenuItem {text:Theme.t("Back");enabled:page.canGoBack;onTriggered:page.goBack()}
                    LobeMenuItem {text:Theme.t("Open website");onTriggered:Qt.openUrlExternally(page.url)}
                    LobeMenuItem {text:"GestArt · "+Theme.t("Settings");onTriggered:root.settings()}
                    LobeMenuItem {text:Theme.t("Fullscreen");onTriggered:app.toggleFullscreen()}
                }
            }
        }
        Text {visible:root.chromeVisible&&!page.isFullScreen&&!root.compact;Layout.fillWidth:true;Layout.leftMargin:16;Layout.rightMargin:16;Layout.bottomMargin:8;text:page.url.toString().split("/")[2]||"";color:Theme.muted;font.pixelSize:11;elide:Text.ElideMiddle}
        ProgressBar {visible:root.chromeVisible&&!page.isFullScreen&&page.loading;Layout.fillWidth:true;Layout.preferredHeight:2;from:0;to:100;value:page.loadProgress;opacity:page.loading&&root.chromeVisible?1:0}
        LobeNotice {visible:root.loadError!==""&&root.chromeVisible;Layout.fillWidth:true;Layout.margins:8;text:Theme.t(root.loadError);actionText:Theme.t("Reload");onActionRequested:page.reload()}
        WebEngineView {
            id:page;objectName:"embeddedWebsite";Layout.fillWidth:true;Layout.fillHeight:true
            profile:siteProfile;url:online.siteUrl(root.provider);backgroundColor:Theme.canvas;audioMuted:!root.visible
            userScripts.collection:[{
                name:"GestArt focus",injectionPoint:WebEngineScript.DocumentReady,
                worldId:WebEngineScript.MainWorld,runsOnSubFrames:false,
                sourceUrl:"qrc:/assets/web-focus.js"
            }]
            settings.fullScreenSupportEnabled:true
            settings.localContentCanAccessFileUrls:false;settings.localContentCanAccessRemoteUrls:false
            settings.errorPageEnabled:false;settings.playbackRequiresUserGesture:true
            onNavigationRequested:request=>{if(request.isMainFrame&&!root.safe(request.url))request.action=WebEngineView.IgnoreRequest}
            onPermissionRequested:permission=>permission.deny()
            onCertificateError:error=>error.rejectCertificate()
            onNewWindowRequested:request=>{
                if(!request.userInitiated)return;
                popupLoader.active=true;sitePopup.open();request.openIn(popupLoader.item)
            }
            onFullScreenRequested:request=>{request.accept();root.fullScreenRequested(request.toggleOn)}
            onLoadingChanged:request=>{
                if(request.status===WebEngineView.LoadStartedStatus){root.loadError="";root.metrics=({})}
                else if(request.status===WebEngineView.LoadFailedStatus)root.loadError="Could not load the website. Check your connection or open it in your browser."
                else if(request.status===WebEngineView.LoadSucceededStatus){root.updateFocus();if(webQaEnabled)inspectDelay.restart()}
            }
        }
    }
    Timer {
        id:inspectDelay;interval:1800;repeat:webQaEnabled
        onTriggered:{
            page.runJavaScript("({title:document.title,controls:document.querySelectorAll('input,select,button').length,bodyLength:document.body?document.body.innerText.length:0})",result=>{
                if(result)root.metrics=Object.assign({},root.metrics,{provider:root.provider,title:result.title,controls:result.controls,bodyLength:result.bodyLength,url:page.url.toString()})
            })
        }
    }
    Popup {
        id:sitePopup;parent:Overlay.overlay;anchors.centerIn:parent
        width:Math.min(640,root.Window.window?root.Window.window.width:root.width);height:Math.min(720,root.Window.window?root.Window.window.height:root.height);modal:true;padding:8
        background:Rectangle {color:Theme.panel;radius:12;border.color:Theme.border}
        onClosed:popupLoader.active=false
        contentItem:ColumnLayout {
            RowLayout {
                Layout.fillWidth:true
                Text {Layout.fillWidth:true;text:popupLoader.item?popupLoader.item.url.toString().split("/")[2]||Theme.t("Websites"):Theme.t("Websites");color:Theme.text;font.pixelSize:12;elide:Text.ElideRight}
                IconButton {iconName:"x";hint:Theme.t("Close");onClicked:sitePopup.close()}
            }
            Loader {
                id:popupLoader;Layout.fillWidth:true;Layout.fillHeight:true;active:false
                sourceComponent:WebEngineView {
                    profile:siteProfile
                    settings.localContentCanAccessFileUrls:false;settings.localContentCanAccessRemoteUrls:false
                    onNavigationRequested:request=>{if(request.isMainFrame&&!root.safe(request.url))request.action=WebEngineView.IgnoreRequest}
                    onPermissionRequested:permission=>permission.deny()
                    onCertificateError:error=>error.rejectCertificate()
                    onWindowCloseRequested:sitePopup.close()
                }
            }
        }
    }
    Shortcut {sequence:"F11";enabled:root.visible;onActivated:{if(page.isFullScreen)root.leaveFullScreen();else app.toggleFullscreen()}}
    Shortcut {sequence:"Escape";enabled:root.visible&&page.isFullScreen;onActivated:root.leaveFullScreen()}
}
