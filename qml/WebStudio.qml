import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine
Item {
    id:root;objectName:"websiteWorkspace"
    property string provider:"quickposes"
    property var lastPages:({})
    property string activeProvider:""
    readonly property string currentTitle:page.title||page.url.toString()
    readonly property string currentPreview:{
        const match=page.url.toString().match(/[?&]v=([A-Za-z0-9_-]{11})(?:&|$)/)
        return match?"https://i.ytimg.com/vi/"+match[1]+"/hqdefault.jpg":""
    }
    property bool windowVideo:false
    function restoreProvider(){
        if(!page)return
        if(activeProvider&&safe(page.url)&&page.url.toString()!=="about:blank")lastPages[activeProvider]=page.url.toString()
        activeProvider=provider
        page.url=lastPages[provider]||online.siteUrl(provider)
    }
    onProviderChanged:restoreProvider()
    Component.onCompleted:{restoreProvider();uiReady=true}
    function toggleWindowVideo(){
        if(page.isFullScreen){leaveFullScreen();return}
        const wasExpanded=root.windowVideo
        page.runJavaScript("window.gestartToggleVideo ? window.gestartToggleVideo() : false",result=>{
            root.windowVideo=!!result
            if(!result&&!wasExpanded)app.notify(Theme.t("Open a video first"))
        })
    }
    property var metrics:({})
    property string loadError:""
    readonly property bool videoSite:root.provider.indexOf("youtube")===0
    function openYouTube(input){
        const value=input.trim().replace(/\\/g, "")
        if(!value)return
        const link=value.replace(/^https?:\/\//i, "")
        if(/^(?:(?:www|m|music)\.)?youtube\.com(?:[/?#]|$)/i.test(link)||/^youtu\.be(?:[/?#]|$)/i.test(link))
            page.url="https://"+link
        else
            page.url="https://www.youtube.com/results?search_query="+encodeURIComponent(value)
        addressExpanded=false
        page.forceActiveFocus()
    }
    property bool uiReady:false
    property bool controlsRequested:false
    property bool addressExpanded:false
    function collapseChrome(){
        controlsRequested=false
        addressExpanded=false
        websitePicker.popup.close()
        websiteMenu.close()
        youtubeAddress.focus=false
        page.forceActiveFocus()
    }
    onWidthChanged:if(uiReady)collapseChrome()
    onHeightChanged:if(uiReady)collapseChrome()
    Connections {target:app;function onPointerInsideChanged(){if(!app.pointerInsideWindow&&!youtubeAddress.activeFocus&&!websiteMenu.opened&&!websitePicker.popup.visible)root.controlsRequested=false}}

    function showAddress(){controlsRequested=true;addressExpanded=true;youtubeAddress.forceActiveFocus();youtubeAddress.selectAll()}
    readonly property bool chromeVisible:(controlsRequested&&app.pointerInsideWindow)||websiteMenu.opened||youtubeAddress.activeFocus||websitePicker.popup.visible
    readonly property bool compact:true
    signal providerRequested(string provider)
    signal navigation()
    signal downloader()
    signal settings()
    readonly property var sites:[{id:"quickposes",name:"Quickposes"},{id:"proko",name:"Proko"},{id:"poselibrary",name:"Pose Library"},{id:"sketchdaily",name:"SketchDaily"},{id:"youtube",name:"YouTube",description:"Videos and search",video:true},{id:"youtube-gesdrawparty",name:"GESDRAWPARTY",description:"YouTube channel",video:true},{id:"youtube-playlist-1",name:"Free YouTube Figure Drawing Sessions",description:"YouTube playlist",video:true},{id:"youtube-playlist-2",name:"Daily Life Drawing Sessions - Model Sessions",description:"YouTube playlist",video:true}]
    function prepareVideoTest(){
        page.runJavaScript("(()=>{const v=document.createElement('video');v.id='gestart-video-test';v.muted=true;v.controls=true;v.style.cssText='position:fixed;left:20px;top:20px;width:320px;height:180px;z-index:2147483647';document.body.appendChild(v);const canvas=document.createElement('canvas');canvas.width=640;canvas.height=360;const ctx=canvas.getContext('2d');ctx.fillStyle='#285b96';ctx.fillRect(0,0,640,360);v.srcObject=canvas.captureStream(5);v.play();window.gestartToggleVideo();return true})()",()=>videoCheck.restart())
    }
    Timer {
        id:videoCheck;interval:600
        onTriggered:page.runJavaScript("(()=>{const v=document.getElementById('gestart-video-test');const r=v.getBoundingClientRect();return {expanded:window.gestartVideoExpanded(),width:r.width,height:r.height,visible:getComputedStyle(v).visibility==='visible',playing:!v.paused&&v.readyState>=2}})()",result=>{
            root.metrics=Object.assign({},root.metrics,{videoTest:result})
        })
    }
    function prepareResizeTest(){windowVideo=!windowVideo;controlsRequested=true;addressExpanded=true;websiteMenu.open()}
    function prepareFocusTest(){page.forceActiveFocus();root.controlsRequested=true}
    function inspectFocusTest(){focusCheck.restart()}
    Timer {
        id:focusCheck;interval:250
        onTriggered:root.metrics=Object.assign({},root.metrics,{
            chromeVisible:root.chromeVisible,chromeOpacity:chromePanel.opacity,chromeHeight:chromePanel.height,
            viewportHeight:page.height,viewportTop:page.y,workspaceHeight:root.height})
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
    IconButton {
        objectName:"websiteShowControls";z:11
        x:root.width-width-4
        y:root.windowVideo||page.isFullScreen?4:root.height-height-4
        width:28;height:28;visible:!root.chromeVisible&&app.pointerInsideWindow
        iconName:"sliders-horizontal";hint:Theme.t("Browser controls")
        background:Rectangle {color:Theme.panel;radius:8;border.color:Theme.border;opacity:0.85}
        onClicked:root.controlsRequested=true
    }
    Rectangle {
        id:chromePanel;objectName:"websiteControlsPanel";z:10
        x:2;width:Math.max(0,root.width-4);height:chromeLayout.implicitHeight
        y:root.windowVideo||page.isFullScreen?2:root.height-height-2
        radius:12;color:Theme.panel;border.color:Theme.border
        opacity:root.chromeVisible?1:0;visible:opacity>0;enabled:root.chromeVisible
        transform:Translate {y:root.chromeVisible?0:8;Behavior on y {NumberAnimation {duration:app.settings.reduce_motion?0:180;easing.type:Easing.OutCubic}}}
        Behavior on opacity {NumberAnimation {duration:app.settings.reduce_motion?0:180;easing.type:Easing.OutCubic}}
        MouseArea {anchors.fill:parent}
        ColumnLayout {
        id:chromeLayout;anchors.left:parent.left;anchors.right:parent.right;spacing:0
        RowLayout {
            Layout.fillWidth:true;Layout.margins:root.compact?4:8;spacing:4
            IconButton {objectName:"websiteHome";Layout.preferredWidth:root.compact?28:36;Layout.preferredHeight:root.compact?28:36;iconName:"panel-left";hint:Theme.t("Home");onClicked:root.navigation()}
            LobeComboBox {
                id:websitePicker;Layout.preferredHeight:root.compact?28:36;Layout.minimumWidth:50;popup.y:root.windowVideo||page.isFullScreen?height+4:-popup.height-4;popup.height:Math.min(popup.implicitHeight,root.height-48);objectName:"websitePicker";Layout.fillWidth:true;Layout.maximumWidth:root.compact?10000:240
                model:root.sites.map(s=>s.name);currentIndex:Math.max(0,root.sites.findIndex(s=>s.id===root.provider))
                onActivated:root.providerRequested(root.sites[currentIndex].id)
                Accessible.name:Theme.t("Websites")
            }
            IconButton {visible:!root.compact;iconName:page.loading?"x":"rotate-cw";hint:Theme.t(page.loading?"Cancel":"Reload");onClicked:{if(page.loading)page.stop();else page.reload()}}
            LobeButton {objectName:"websiteDownloader";visible:!root.compact&&!root.videoSite;implicitHeight:36;text:Theme.t("Download references");iconName:"download";onClicked:root.downloader()}
            IconButton {Layout.preferredWidth:root.compact?28:36;Layout.preferredHeight:root.compact?28:36;objectName:"websiteWindowVideo";visible:root.videoSite;iconName:"maximize";hint:Theme.t("Video fills window");toggle:true;selected:root.windowVideo||page.isFullScreen;onClicked:root.toggleWindowVideo()}
            IconButton {Layout.preferredWidth:root.compact?28:36;Layout.preferredHeight:root.compact?28:36;objectName:"websitePin";iconName:"pin";hint:Theme.t("Always on top");toggle:true;selected:app.settings.always_on_top;onClicked:app.togglePin()}
            IconButton {visible:root.compact&&root.videoSite;Layout.preferredWidth:28;Layout.preferredHeight:28;iconName:"search";hint:Theme.t("Search YouTube");onClicked:root.showAddress()}
            IconButton {Layout.preferredWidth:28;Layout.preferredHeight:28;iconName:"x";hint:Theme.t("Close");onClicked:root.collapseChrome()}
            IconButton {id:more;Layout.preferredWidth:root.compact?28:36;Layout.preferredHeight:root.compact?28:36;iconName:"ellipsis";hint:Theme.t("Settings");onClicked:websiteMenu.open()
                LobeMenu {id:websiteMenu;y:root.windowVideo||page.isFullScreen?more.height+4:-height-4;height:Math.min(implicitHeight,root.height-48);x:more.width-width;width:228
                    LobeMenuItem {visible:root.compact;text:Theme.t("Reload");onTriggered:page.reload()}
                    LobeMenuItem {visible:root.compact&&!root.videoSite;text:Theme.t("Download references");onTriggered:root.downloader()}
                    LobeMenuItem {text:Theme.t("Back");enabled:page.canGoBack;onTriggered:page.goBack()}
                    LobeMenuItem {text:Theme.t("Open website");onTriggered:Qt.openUrlExternally(page.url)}
                    LobeMenuItem {text:"GestArt · "+Theme.t("Settings");onTriggered:root.settings()}
                    LobeMenuItem {text:Theme.t("Fullscreen");onTriggered:app.toggleFullscreen()}
                }
            }
        }
        RowLayout {
            visible:root.videoSite&&((!root.compact&&!page.isFullScreen&&!root.windowVideo)||root.addressExpanded)
            Layout.fillWidth:true;Layout.leftMargin:8;Layout.rightMargin:8;Layout.bottomMargin:8;spacing:4
            IconButton {iconName:"chevron-left";hint:Theme.t("Back");enabled:page.canGoBack;onClicked:page.goBack()}
            IconButton {iconName:"chevron-right";hint:Theme.t("Forward");enabled:page.canGoForward;onClicked:page.goForward()}
            LobeSearch {
                id:youtubeAddress;objectName:"youtubeAddress";Layout.fillWidth:true
                placeholderText:Theme.t("Search YouTube or paste a link")
                onAccepted:root.openYouTube(text)
                Keys.onEscapePressed:{root.addressExpanded=false;page.forceActiveFocus()}
            }
            IconButton {iconName:"search";hint:Theme.t("Search YouTube");onClicked:root.openYouTube(youtubeAddress.text)}
        }
        Text {visible:!page.isFullScreen&&!root.windowVideo&&!root.compact;Layout.fillWidth:true;Layout.leftMargin:16;Layout.rightMargin:16;Layout.bottomMargin:8;text:page.url.toString().split("/")[2]||"";color:Theme.muted;font.pixelSize:12;elide:Text.ElideMiddle}
        ProgressBar {visible:page.loading;Layout.fillWidth:true;Layout.preferredHeight:2;from:0;to:100;value:page.loadProgress;opacity:page.loading&&root.chromeVisible?1:0}
        LobeNotice {visible:root.loadError!=="";Layout.fillWidth:true;Layout.margins:8;text:Theme.t(root.loadError);actionText:Theme.t("Reload");onActionRequested:page.reload()}
        }
    }
    WebEngineView {
            id:page;objectName:"embeddedWebsite";anchors.fill:parent
            profile:siteProfile;backgroundColor:Theme.canvas;audioMuted:!root.visible
            userScripts.collection:[{
                name:"GestArt window video",injectionPoint:WebEngineScript.DocumentReady,
                worldId:WebEngineScript.MainWorld,runsOnSubFrames:false,
                sourceUrl:"qrc:/assets/web-video.js"
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
            onFullScreenRequested:request=>request.accept()
            onLoadingChanged:request=>{
                if(request.status===WebEngineView.LoadStartedStatus){root.loadError="";root.metrics=({});root.windowVideo=false}
                else if(request.status===WebEngineView.LoadFailedStatus)root.loadError="Could not load the website. Check your connection or open it in your browser."
                else if(request.status===WebEngineView.LoadSucceededStatus){if(webQaEnabled)inspectDelay.restart()}
            }
    }
    Timer {
        interval:400;repeat:true;running:root.visible&&root.videoSite
        onTriggered:page.runJavaScript("!!(window.gestartVideoExpanded && window.gestartVideoExpanded())",result=>root.windowVideo=!!result)
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
                    profile:siteProfile;audioMuted:!root.visible
                    settings.localContentCanAccessFileUrls:false;settings.localContentCanAccessRemoteUrls:false
                    onNavigationRequested:request=>{if(request.isMainFrame&&!root.safe(request.url))request.action=WebEngineView.IgnoreRequest}
                    onPermissionRequested:permission=>permission.deny()
                    onCertificateError:error=>error.rejectCertificate()
                    onWindowCloseRequested:sitePopup.close()
                }
            }
        }
    }
    Shortcut {sequence:"Ctrl+L";enabled:root.visible&&root.videoSite&&!page.isFullScreen;onActivated:{youtubeAddress.text=page.url.toString();root.showAddress()}}
    Shortcut {sequence:"F11";enabled:root.visible;onActivated:{if(page.isFullScreen)root.leaveFullScreen();else app.toggleFullscreen()}}
    Shortcut {sequence:"Escape";enabled:root.visible&&(page.isFullScreen||root.windowVideo);onActivated:{if(page.isFullScreen)root.leaveFullScreen();else root.toggleWindowVideo()}}
}
