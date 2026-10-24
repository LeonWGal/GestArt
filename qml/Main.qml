import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
ApplicationWindow {
    id: window
    width: 1280; height: 860; minimumWidth: 360; minimumHeight: 440
    visible: true; title: "GestArt Ultimate"
    color: Theme.canvas
    font.family: Theme.font
    LayoutMirroring.enabled: i18n.rtl
    LayoutMirroring.childrenInherit: true
    property string webProvider:""
    property bool webWasWindowed:false
    readonly property var browserMetrics:websiteLoader.item?websiteLoader.item.metrics:({})
    property bool sidebarVisible: true
    readonly property bool sidebarCompact:app.settings.sidebar_compact||false
    readonly property bool immersive: (app.page === "studio" && app.currentImage !== "")||app.page==="web"
    readonly property bool chromeVisible: !immersive || app.pointerInsideWindow || studio.dragging || sheet.visible || collectionSheet.visible || trashDialog.visible || navigation.visible
    property bool shortcutsEnabled: app.page!=="web" && !onlineSheet.visible && !preview.visible && !sheet.visible && !collectionSheet.visible && !trashDialog.visible && !navigation.visible && !studio.menuOpen && !(activeFocusItem && (activeFocusItem instanceof TextInput || activeFocusItem instanceof TextEdit))
    property bool mediaShortcutsEnabled: shortcutsEnabled && app.page === "studio" && !studio.resultOpen
    palette.window: Theme.panel; palette.windowText: Theme.text; palette.text: Theme.text
    palette.base: Theme.raised; palette.button: Theme.raised; palette.buttonText: Theme.text
    palette.highlight: Theme.accent; palette.highlightedText: Theme.primaryText
    FolderDialog { id: folderDialog; title: Theme.t("Add folder"); onAccepted: {app.addFolder(selectedFolder);if(app.page==="studio")app.page="home"} }
    FileDialog { id: filesDialog; title: Theme.t("Select images"); fileMode: FileDialog.OpenFiles; nameFilters: [app.imageFilter, Theme.t("All files")+" (*)"]; onAccepted: {app.loadFiles(selectedFiles);if(app.page==="studio")app.page="home"} }
    RowLayout {
        anchors.fill: parent; spacing: 0
        Sidebar {
            visible:!window.immersive&&window.sidebarVisible&&window.width>=720
            allowToggle:window.width>=1000;compact: window.width < 1000 || window.sidebarCompact; Layout.preferredWidth: compact ? 64 : 200;
            onToggleMode:app.setSidebarCompact(!window.sidebarCompact); Layout.fillHeight: true
            onAddFolder: folderDialog.open(); onAddFiles: filesDialog.open(); onSettings: app.openSettings(); onSavedSets:window.openSavedSets(); onManageCollections: window.openCollections()
        }
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.margins: window.immersive || window.visibility === Window.FullScreen ? 0 : window.width<520?0:16
            Layout.leftMargin: window.immersive || window.visibility === Window.FullScreen || window.width<520 || (window.sidebarVisible&&window.width>=720) ? 0 : 16
            color: Theme.panel; radius: window.immersive || window.visibility === Window.FullScreen ? 0 : 16
            border.width: window.immersive ? 0 : 1; border.color: window.chromeVisible ? Theme.border : "transparent"; clip: true
            ColumnLayout {
                anchors.fill: parent; spacing: 0
                Rectangle {
                    visible:!window.immersive
                    Layout.fillWidth: true; implicitHeight: window.height < 400 ? 40 : 64; color: "transparent"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: window.width<520?8:24; anchors.rightMargin: window.width<520?8:24; spacing: 8
                        IconButton {width:window.width<520?28:36;height:width;iconName:"panel-left";hint:Theme.t("Navigation");onClicked:{if(window.width<1000)navigation.open();else app.setSidebarCompact(!window.sidebarCompact)}}
                        ColumnLayout { spacing: 4; Layout.fillWidth: true
                            Text {Layout.fillWidth:true;elide:Text.ElideRight;text:Theme.t(({home:"Home",studio:"Your drawing space",library:"Collections",practice:"Practice",history:"History",sites:"Online viewing"})[app.page]);color:Theme.text;font.pixelSize:window.width<520?14:18;font.weight:Font.DemiBold}
                            Text {visible:window.height>=500;text:app.page==="sites"?Theme.t("Use each website with its own settings and timer inside GestArt."):app.page==="home"?Theme.dateLabel(app.practice.today,"dddd, d MMMM"):app.page==="practice"?Theme.t("Your progress, calendar and daily goal"):app.page==="history"?Theme.t("Your recent references"):app.currentSource!==""?app.currentSource:Theme.t("A little practice. A sharper eye.");color:Theme.muted;font.pixelSize:12;elide:Text.ElideMiddle;Layout.fillWidth:true}
                        }
                        Rectangle { visible: app.scanning&&window.width>=600; radius: 8; color: Theme.raised; implicitWidth: scanRow.implicitWidth + 20; height: 32
                            Row { id: scanRow; anchors.centerIn: parent; spacing: 8; Text { text: Theme.t("Scanning") + " · " + app.imageCount; color: Theme.muted; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter } IconButton { iconName: "x"; width: 24; height: 24; hint: Theme.t("Cancel"); onClicked: app.cancelScan() } }
                        }
                        IconButton {visible:window.webProvider!=="";iconName:"globe";hint:Theme.t("Return to website");onClicked:app.page="web"}
                        IconButton {objectName:"headerReturnSession";visible:app.currentImage!=="";iconName:"play";hint:Theme.t("Return to session");onClicked:app.page="studio"}
                        IconButton {width:window.width<520?28:36;height:width;iconName:app.dark?"sun":"moon";hint:Theme.t("Appearance");onClicked:window.openAppearance()}
                        LobeButton {visible:false;text:Theme.t("Add folder");iconName:"folder-plus";onClicked:folderDialog.open()}
                    }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.border }
                }
                Item {
                    Layout.fillWidth: true; Layout.fillHeight: true
                StackLayout {
                    anchors.fill:parent;visible:app.page!=="studio"&&app.page!=="web"
                    currentIndex:Math.max(0,["home","library","practice","history","sites"].indexOf(app.page))
                    Home {onSavedSets:window.openSavedSets();onDownloader:window.openOnlineSources("download");onOnlineSources:window.openOnlineSources();onDataSettings:window.openSettingsSection(4);onAddFolder:folderDialog.open();onManageCollections:window.openCollections();onEditSession:app.openSettings();onEditGoal:window.openSettingsSection(3)}
                    Library {onSavedSets:window.openSavedSets();onDownloader:window.openOnlineSources("download");onOnlineSources:window.openOnlineSources();onAddFolder:folderDialog.open();onAddFiles:filesDialog.open();onManageCollections:window.openCollections()}
                    Practice {onDataSettings:window.openSettingsSection(4);onEditGoal:window.openSettingsSection(3)}
                    History {onDataSettings:window.openSettingsSection(4)}
                    OnlineSites {onSavedSets:window.openSavedSets();onOpenSite:provider=>window.openWebsite(provider);onPractice:provider=>window.openProviderDownloads(provider)}
                }
                    Loader {
                        id:websiteLoader;anchors.fill:parent;active:window.webProvider!=="";visible:app.page==="web"
                        sourceComponent:WebStudio {
                            provider:window.webProvider
                            onProviderRequested:provider=>window.webProvider=provider
                            onNavigation:app.page="sites"
                            onDownloader:window.openProviderDownloads(window.webProvider)
                            onSettings:app.openSettings()
                            onFullScreenRequested:enabled=>{
                                if(enabled&&window.visibility!==Window.FullScreen){window.webWasWindowed=true;app.toggleFullscreen()}
                                else if(!enabled&&window.webWasWindowed){window.webWasWindowed=false;if(window.visibility===Window.FullScreen)app.toggleFullscreen()}
                            }
                        }
                    }
                    Studio { id: studio; anchors.fill:parent; visible:app.page==="studio"; chromeVisible: window.chromeVisible; onNavigation: app.page="home"; onAppearance: window.openAppearance(); onManageCollections:window.openCollections(); onAddFolder: folderDialog.open(); onAddFiles: filesDialog.open(); onDeleteRequest: window.requestTrash() }
                }
            }
        }
    }
    MediaPreview {id:preview;parent:Overlay.overlay}
    DropArea { anchors.fill: parent; onDropped: drop => {if(drop.hasUrls){app.dropUrls(drop.urls);if(app.page!=="library")app.page="home";drop.acceptProposedAction();}} }
    SettingsSheet { id: sheet; parent: Overlay.overlay; opacity: window.chromeVisible ? 1 : 0; width: Math.min(480, window.width); height: window.height; onClosed: app.closeSettings()
        Overlay.modal: Rectangle {color:window.chromeVisible?"#66000000":"transparent"}
    }
    property int nextSettingsSection: 0
    function openAppearance() {nextSettingsSection=1;app.openSettings()}
    function openSettingsSection(section) {nextSettingsSection=section;app.openSettings()}
    function openNavigation() {navigation.open()}
    function openStudioMenu(name) {studio.openMenu(name)}
    function prepareWebFocusTest(){if(websiteLoader.item)websiteLoader.item.prepareFocusTest()}
    function inspectWebFocusTest(){if(websiteLoader.item)websiteLoader.item.inspectFocusTest()}
    function openWebsite(provider){if(online.siteUrl(provider).toString()!==""){window.webProvider=provider;app.page="web"}}
    function openProviderDownloads(provider){
        if(!online.busy&&online.provider!==provider){onlineSheet.selectedSets=[];online.selectProvider(provider)}
        onlineSheet.section="download";onlineSheet.open()
    }
    Connections {target:downloads;function onPracticeReady(folder){
        if(!app.active){onlineSheet.close();app.startDownloadedSession(folder)}
    }}
    function openSavedSets(){onlineSheet.section="saved";onlineSheet.open()}
    function openOnlineSources(mode){if(mode==="download"){onlineSheet.section="download";onlineSheet.open()}else app.page="sites"}
    OnlineSourcesSheet {id:onlineSheet;onStartSession:folder=>app.startDownloadedSession(folder);parent:Overlay.overlay;width:Math.min(680,window.width);height:window.height}
    function openCollections() {if(app.page==="studio")app.page="library";collectionSheet.open()}
    CollectionsSheet {id:collectionSheet;parent:Overlay.overlay;width:Math.min(420,window.width);height:window.height;onOnlineSources:window.openOnlineSources();onAddFolder:folderDialog.open();onAddFiles:filesDialog.open()}
    Drawer { id: navigation; objectName:"navigationDrawer"; parent: Overlay.overlay; opacity: window.chromeVisible ? 1 : 0; edge: Qt.LeftEdge; width: Math.min(248,window.width); height: window.height; modal: true
        Overlay.modal: Rectangle {color:window.chromeVisible?"#66000000":"transparent"}
        background: Rectangle { color: Theme.canvas }
        Sidebar { anchors.fill: parent
            onSavedSets:{navigation.close();window.openSavedSets()}
            onAddFolder: {navigation.close();folderDialog.open()}
            onAddFiles: {navigation.close();filesDialog.open()}
            onSettings: {navigation.close();Qt.callLater(()=>app.openSettings())}
            onManageCollections: {navigation.close();Qt.callLater(()=>window.openCollections())}
        }
        Connections { target: app; function onNavigationChanged(){navigation.close()} }
    }
    Connections { target: app; function onSettingsRequested() {sheet.section=window.nextSettingsSection;window.nextSettingsSection=0;sheet.open()} }
    function requestTrash() {if(app.currentPath === "")return;if(app.settings.confirm_delete)trashDialog.open();else app.trashCurrent()}
    Dialog {
        id: trashDialog; parent: Overlay.overlay; anchors.centerIn: parent; modal: true; width: Math.min(420, window.width - 40)
        title: Theme.t("Move to trash"); implicitHeight:260;height:Math.min(implicitHeight,window.height-16);opacity: window.chromeVisible ? 1 : 0
        Overlay.modal: Rectangle {color:window.chromeVisible?"#66000000":"transparent"}
        onAccepted: app.trashCurrent()
        contentItem:ScrollView {clip:true;contentWidth:availableWidth
            Text {width:trashDialog.availableWidth;text:Theme.t("Are you sure you want to move the file to trash?")+"\n\n"+app.fileName;color:Theme.text;wrapMode:Text.Wrap;font.pixelSize:13}
        }
        footer:RowLayout {spacing:8
            LobeButton {text:window.width<320?"":Theme.t("Cancel");Accessible.name:Theme.t("Cancel");iconName:window.width<320?"x":"";onClicked:trashDialog.reject()}
            LobeButton {text:Theme.t("Move to trash");iconName:"trash-2";danger:true;Layout.fillWidth:true;font.pixelSize:12;leftPadding:8;rightPadding:8;onClicked:trashDialog.accept()}
        }
    }
    Rectangle {
        parent:Overlay.overlay
        visible: app.toast !== ""; opacity: 1; z: 10000; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin:sheet.visible||collectionSheet.visible?72:window.immersive ? (studio.resultOpen?16:120) : 16
        width: Math.min(toastText.implicitWidth + 40, window.width - 64); height: toastText.implicitHeight + 28
        radius: 12; color: Theme.raised; border.color: Theme.border
        Text { id: toastText; anchors.centerIn: parent; width: Math.min(implicitWidth, window.width - 104); text: app.toast; wrapMode: Text.WordWrap; color: Theme.text; font.pixelSize: 13 }
        Accessible.role: Accessible.AlertMessage; Accessible.name: app.toast
    }
    Shortcut { sequence: "Space"; enabled: window.mediaShortcutsEnabled; onActivated: app.togglePause() }
    Shortcut { sequence: "Left"; enabled: window.mediaShortcutsEnabled; onActivated: {studio.resultOpen=false;app.previous()} }
    Shortcut { sequence: "Right"; enabled: window.mediaShortcutsEnabled; onActivated: {studio.resultOpen=false;app.next()} }
    Shortcut { sequence: "S"; enabled: window.mediaShortcutsEnabled; onActivated: app.skip() }
    Shortcut { sequence: "B"; enabled: window.mediaShortcutsEnabled; onActivated: app.setEffect("bw") }
    Shortcut { sequence: "H"; enabled: window.mediaShortcutsEnabled; onActivated: app.setEffect("horizontal") }
    Shortcut { sequence: "V"; enabled: window.mediaShortcutsEnabled; onActivated: app.setEffect("vertical") }
    Shortcut { sequence: "R"; enabled: window.mediaShortcutsEnabled; onActivated: app.setEffect("rotate") }
    Shortcut { sequence: "G"; enabled: window.mediaShortcutsEnabled; onActivated: app.setEffect("grid") }
    Shortcut { sequence: "Backspace"; enabled: window.mediaShortcutsEnabled; onActivated: {app.resetEffects();studio.resetView()} }
    Shortcut { sequence: "O"; enabled: window.mediaShortcutsEnabled; onActivated: app.openCurrentFile() }
    Shortcut { sequences: ["Ctrl+C","C"]; enabled: window.mediaShortcutsEnabled; onActivated: app.copyImage() }
    Shortcut { sequence: "Delete"; enabled: window.mediaShortcutsEnabled; onActivated: window.requestTrash() }
    Shortcut { sequence: "F11"; enabled:app.page!=="web"; onActivated: app.toggleFullscreen() }
    Shortcut { sequence: "Escape"; enabled: window.shortcutsEnabled && (studio.resultOpen || window.visibility === Window.FullScreen); onActivated: {if(studio.resultOpen)studio.resultOpen=false;else app.toggleFullscreen()} }
    Shortcut { sequence: "Ctrl+,"; enabled: window.shortcutsEnabled; onActivated: app.openSettings() }
    Shortcut {sequence:"Ctrl+B";enabled:window.shortcutsEnabled;onActivated:{if(window.immersive||window.width<1000)navigation.open();else app.setSidebarCompact(!window.sidebarCompact)}}
    Shortcut { sequences: ["Ctrl++","Ctrl+="]; enabled: window.mediaShortcutsEnabled; onActivated: studio.zoomCentered(1.1) }
    Shortcut { sequence: "Ctrl+-"; enabled: window.mediaShortcutsEnabled; onActivated: studio.zoomCentered(1/1.1) }
    Shortcut { sequence: "Ctrl+0"; enabled: window.mediaShortcutsEnabled; onActivated: studio.resetView() }
    Shortcut { sequence: "Ctrl+1"; enabled: window.mediaShortcutsEnabled; onActivated: studio.pixelView() }
    Shortcut { sequence: "A"; enabled: window.mediaShortcutsEnabled; onActivated: app.togglePin() }
    Shortcut { sequence: "T"; enabled: window.mediaShortcutsEnabled; onActivated: {let s=Object.assign({},app.settings);s.show_timer=!s.show_timer;app.applySettings(s)} }
}

