import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
    id: root;objectName:"studioPage"
    signal addFolder()
    signal addFiles()
    signal deleteRequest()
    signal navigation()
    signal appearance()
    signal manageCollections()
    property bool chromeVisible: true
    property bool resultOpen:false
    property string previousState:"idle"
    readonly property bool compact:width<520
    readonly property bool micro:width<340
    readonly property int inset:compact?8:16
    readonly property bool dragging:panMouse.pressed
    readonly property bool menuOpen: sessionMenu.opened
    property real targetZoom:1
    property real targetPanX:0
    property real targetPanY:0
    property real zoom: targetZoom
    property real panX: targetPanX
    property real panY: targetPanY
    readonly property bool fitMode:Math.abs(targetZoom-1)<0.001&&Math.abs(targetPanX)<0.5&&Math.abs(targetPanY)<0.5
    property bool actualSizeRequested:false
    readonly property real pixelZoom:app.imagePixelSize.width>0?app.imagePixelSize.width/Math.max(1,transformed.width*Screen.devicePixelRatio):1
    readonly property bool actualSizeMode:actualSizeRequested&&Math.abs(targetZoom-pixelZoom)<0.001
    property real displayAspect:1
    property var displayRequestId:0
    property real decodeZoom:1
    property bool resettingView:false
    readonly property real progressValue:app.visualProgress
    property bool loadingIndicator:false
    Behavior on zoom {enabled:!root.resettingView&&!app.settings.reduce_motion;NumberAnimation {duration:140;easing.type:Easing.OutCubic}}
    Behavior on panX {enabled:!root.resettingView&&!panMouse.pressed&&!app.settings.reduce_motion;NumberAnimation {duration:140;easing.type:Easing.OutCubic}}
    Behavior on panY {enabled:!root.resettingView&&!panMouse.pressed&&!app.settings.reduce_motion;NumberAnimation {duration:140;easing.type:Easing.OutCubic}}
    onZoomChanged:decodeDelay.restart()
    Timer {id:decodeDelay;interval:160;onTriggered:root.decodeZoom=root.zoom}
    Timer {id:loadingDelay;interval:180;onTriggered:root.loadingIndicator=app.loading}
    readonly property string sessionSummary:Theme.sessionSummary(app.settings)
    function pixelView() {
        if(app.imagePixelSize.width<=0)return;
        targetZoom=Qt.binding(()=>root.pixelZoom);
        actualSizeRequested=true;
        targetPanX=0;targetPanY=0;
    }
    function resetView() {actualSizeRequested=false;resettingView=true;targetZoom=1;targetPanX=0;targetPanY=0;decodeZoom=1;resettingView=false}
    function zoomCentered(factor) {zoomAt(factor,canvas.width/2,canvas.height/2)}
    function zoomAt(factor,x,y) {
        actualSizeRequested=false;
        const old=targetZoom;const next=Math.max(0.1,Math.min(20,old*factor));
        targetPanX=(targetPanX-(x-canvas.width/2))*next/old+(x-canvas.width/2);
        targetPanY=(targetPanY-(y-canvas.height/2))*next/old+(y-canvas.height/2);
        targetZoom=next;
    }
    Connections {target:app
        function onImageChanged(){if(app.loading){root.loadingIndicator=false;loadingDelay.restart()}else {loadingDelay.stop();root.loadingIndicator=false}}
        function onSessionChanged(){if(app.state==="completed"){sessionMenu.close();if(root.previousState!=="completed")root.resultOpen=true}else root.resultOpen=false;root.previousState=app.state}
        function onImageMetadataChanged(){if(root.displayRequestId===app.requestId&&app.imagePixelSize.height>0)root.displayAspect=app.imagePixelSize.width/app.imagePixelSize.height}
    }
    Connections {target:app;function onPointerInsideChanged(){if(!app.pointerInsideWindow)sessionMenu.close()}}
    function openMenu(name) {if(name==="session-menu")sessionMenu.open()}
    Rectangle {
        id:canvas;objectName:"imageCanvas";anchors.fill:parent;color:Theme.canvas;clip:true
        Item {id:referenceScene;anchors.fill:parent
        Rectangle {anchors.fill:parent;color:Theme.canvas}
        Item {
            id:transformed;visible:app.currentImage!==""
            property real aspect:root.displayAspect
            width:app.rotation%180===0?Math.min(canvas.width,canvas.height*aspect):Math.min(canvas.height,canvas.width*aspect)
            height:width/aspect;x:(canvas.width-width)/2+root.panX;y:(canvas.height-height)/2+root.panY
            scale:root.zoom;rotation:app.rotation
            Item {id:imagePlane;objectName:"imagePlane";anchors.fill:parent
                transform:Scale {origin.x:imagePlane.width/2;origin.y:imagePlane.height/2;xScale:app.flipH?-1:1;yScale:app.flipV?-1:1}
            Image {
                id:art;objectName:"referenceImage";anchors.fill:parent
                source:app.currentImage+(app.bw&&app.softwareRenderer?"/bw":"")
                fillMode:Image.PreserveAspectFit;asynchronous:true;cache:false;retainWhileLoading:true
                readonly property real decodeScale:Math.max(1,root.decodeZoom)*Screen.devicePixelRatio
                sourceSize:Qt.size(Math.max(64,Math.min(8192,Math.ceil((app.rotation%180===0?canvas.width:canvas.height)*decodeScale))),Math.max(64,Math.min(8192,Math.ceil((app.rotation%180===0?canvas.height:canvas.width)*decodeScale))));smooth:true
                layer.enabled:app.bw&&!app.softwareRenderer
                layer.effect:ShaderEffect {property var source;property real strength:1;fragmentShader:"qrc:/shaders/grayscale.frag.qsb"}
                onStatusChanged:{if(status===Image.Ready){if(root.displayRequestId!==app.requestId){root.displayRequestId=app.requestId;root.displayAspect=implicitWidth/Math.max(1,implicitHeight);app.ready(app.requestId);root.resetView()}else app.ready(app.requestId)}else if(status===Image.Error)app.loadError(app.requestId)}
            }
            ImageGrid {anchors.fill:parent;visible:app.grid;horizontalLines:app.settings.grid_h_lines;verticalLines:app.settings.grid_v_lines;imageScale:root.zoom;pixelRatio:Screen.devicePixelRatio}
            }
        }
        }
        MouseArea {
            id:panMouse
            anchors.fill:parent;hoverEnabled:true
            cursorShape:(app.settings.always_on_top&&app.settings.drag_when_pinned&&root.fitMode)?Qt.SizeAllCursor:pressed?Qt.ClosedHandCursor:app.currentImage!==""?Qt.OpenHandCursor:Qt.ArrowCursor
            property real lastX;property real lastY
            property bool movingWindow:false
            onPressed:mouse=>{lastX=mouse.x;lastY=mouse.y;movingWindow=(mouse.modifiers&Qt.AltModifier)!==0||(app.settings.always_on_top&&app.settings.drag_when_pinned&&root.fitMode);if(movingWindow)movingWindow=app.beginWindowMove()}
            onPositionChanged:mouse=>{if(pressed&&!movingWindow){root.targetPanX+=mouse.x-lastX;root.targetPanY+=mouse.y-lastY;lastX=mouse.x;lastY=mouse.y}}
            onReleased:movingWindow=false
            onCanceled:movingWindow=false
            onDoubleClicked:root.resetView()
            onWheel:wheel=>{
                if(app.currentImage===""){wheel.accepted=false;return;}
                const delta=wheel.pixelDelta.y!==0?wheel.pixelDelta.y/60:wheel.angleDelta.y/120;
                if(delta===0){wheel.accepted=false;return;}
                root.zoomAt(Math.pow(1.1,Math.max(-8,Math.min(8,delta))),wheel.x,wheel.y);wheel.accepted=true;
            }
        }
        ColumnLayout {
            visible:app.currentImage===""&&root.height>=340;anchors.centerIn:parent;width:Math.min(440,canvas.width-48);spacing:16
            Rectangle {Layout.alignment:Qt.AlignHCenter;width:72;height:72;radius:24;color:Theme.raised;border.color:Theme.border
                ArtIcon {anchors.centerIn:parent;name:"scan-eye";width:32;height:32;tint:Theme.accent}
            }
            Text {text:Theme.t("Make time to draw");color:Theme.text;font.pixelSize:root.compact?20:28;font.weight:Font.DemiBold;Layout.fillWidth:true;horizontalAlignment:Text.AlignHCenter;wrapMode:Text.WordWrap}
            Text {text:Theme.t("Choose your references. Set a rhythm.\nFocus on what you see.");color:Theme.muted;font.pixelSize:14;lineHeight:1.4;Layout.fillWidth:true;horizontalAlignment:Text.AlignHCenter;wrapMode:Text.WordWrap}
            RowLayout {Layout.alignment:Qt.AlignHCenter;Layout.topMargin:8;spacing:8
                LobeButton {primary:true;text:Theme.t("Select folder");iconName:"folder-plus";onClicked:root.addFolder()}
                LobeButton {visible:!root.compact;text:Theme.t("Select images");iconName:"image-plus";onClicked:root.addFiles()}
            }
            Text {text:Theme.t("Or drag a folder or images here");color:Theme.subtle;font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap;horizontalAlignment:Text.AlignHCenter}
        }
        LobeButton {visible:app.currentImage===""&&root.height<340;anchors.centerIn:parent;text:root.width<320?"":Theme.t("Add folder");Accessible.name:Theme.t("Add folder");primary:true;iconName:"folder-plus";onClicked:root.addFolder()}
        // Fade chrome only; geometry and the session clock stay untouched.
        Item {
            id:chrome;objectName:"sessionChrome";z:3;anchors.fill:parent;visible:app.currentImage!==""
            opacity:root.chromeVisible?1:0;enabled:root.chromeVisible
            Behavior on opacity {NumberAnimation {duration:app.settings.reduce_motion?0:220;easing.type:Easing.InOutQuad}}
            GlassPanel {scene:referenceScene;captureEnabled:chrome.opacity>0.001;objectName:"sessionContext"
                id:contextBar;anchors.left:parent.left;anchors.top:parent.top;anchors.leftMargin:root.inset
                anchors.topMargin:app.active&&app.settings.show_timer&&timerBadge.x<root.inset+width+8?60:root.inset
                width:contextRow.implicitWidth+16;height:root.compact?36:44;radius:12
                RowLayout {id:contextRow;objectName:"sessionContextContent";anchors.centerIn:parent;spacing:4
                    IconButton {width:root.compact?28:36;height:width;objectName:"leaveSessionButton";iconName:"arrow-left";hint:Theme.t("Back to app");onClicked:root.navigation()}
                    Button {id:modeButton;objectName:"sessionMenuButton";implicitHeight:root.compact?28:36;implicitWidth:root.micro?42:Math.min(200,modeLabel.implicitWidth+40);hoverEnabled:true
                        Accessible.name:Theme.t(app.state==="completed"?"Session results":"Session menu");onClicked:{if(app.state==="completed")root.resultOpen=!root.resultOpen;else if(sessionMenu.opened)sessionMenu.close();else sessionMenu.open()}
                        background:Rectangle {radius:8;color:Theme.text;opacity:sessionMenu.opened?.12:parent.down?.14:parent.hovered?.08:0;Behavior on opacity {NumberAnimation {duration:app.settings.reduce_motion?0:100}}}
                        contentItem:Item {implicitHeight:18
                            Row {anchors.centerIn:parent;spacing:4
                                ArtIcon {visible:root.micro;name:app.state==="completed"?"circle-check":"timer";width:18;height:18;tint:Theme.text}
                                Text {id:modeLabel;visible:!root.micro;text:Theme.t(app.state==="completed"?"Session results":app.state==="review"?"History":app.state==="break"?"Break":app.active?(app.freeView?"Free view":"Session"):"Preview");color:Theme.text;font.pixelSize:12;anchors.verticalCenter:parent.verticalCenter}
                                ArtIcon {name:"chevron-right";width:12;height:12;tint:Theme.muted;rotation:sessionMenu.opened?270:90;anchors.verticalCenter:parent.verticalCenter;Behavior on rotation {NumberAnimation {duration:app.settings.reduce_motion?0:120;easing.type:Easing.OutCubic}}}
                            }
                        }
                        ToolTip.visible:hovered;ToolTip.delay:650;ToolTip.text:root.sessionSummary
                        SessionPopover {id:sessionMenu;x:-modeButton.x-8;y:modeButton.height+8;width:Math.min(340,root.width-2*root.inset);onManageCollections:root.manageCollections()}
                    }
                }
            }
            GlassPanel {scene:referenceScene;captureEnabled:chrome.opacity>0.001;id:windowActions;objectName:"sessionWindowActions";anchors.right:parent.right;anchors.top:parent.top;anchors.rightMargin:root.inset
                anchors.topMargin:app.active&&app.settings.show_timer&&timerBadge.x+timerBadge.width>root.width-root.inset-width-8?60:root.inset
                width:topActions.implicitWidth+16;height:root.compact?36:44;radius:12
                Row {id:topActions;objectName:"sessionWindowContent";anchors.centerIn:parent;spacing:4
                    IconButton {objectName:"sessionSettingsButton";width:root.compact?28:36;height:width;iconName:"settings-2";hint:Theme.t("Session settings")+" · Ctrl+,";onClicked:app.openSettings()}
                    IconButton {width:root.compact?28:36;height:width;iconName:"pin";hint:Theme.t("Always on top")+" · A";toggle:true;selected:app.settings.always_on_top;onClicked:app.togglePin()}
                    IconButton {objectName:"sessionFullscreenButton";width:root.compact?28:36;height:width;iconName:"maximize";hint:Theme.t("Fullscreen")+" · F11";onClicked:app.toggleFullscreen()}
                }
            }
            SessionToolbar {id:sessionBar;scene:referenceScene;captureEnabled:chrome.opacity>0.001;visible:!root.resultOpen
                anchors.horizontalCenter:parent.horizontalCenter;anchors.bottom:parent.bottom;anchors.bottomMargin:root.inset
                availableWidth:root.width-2*root.inset;actualSize:root.actualSizeMode;viewPercent:Math.round(root.zoom*transformed.width*Screen.devicePixelRatio/Math.max(1,app.imagePixelSize.width)*100)
                onFitRequested:{if(root.actualSizeMode)root.resetView();else root.pixelView()}
                onResetRequested:root.resetView()
                onDeleteRequest:root.deleteRequest()
            }
            GlassPanel {scene:referenceScene;captureEnabled:chrome.opacity>0.001;visible:app.state==="review";anchors.horizontalCenter:parent.horizontalCenter;anchors.bottom:sessionBar.top;anchors.bottomMargin:16
                width:Math.min(stateText.implicitWidth+32,root.width-16);height:36;radius:12
                Text {id:stateText;anchors.centerIn:parent;width:Math.min(implicitWidth,parent.width-24);elide:Text.ElideRight;text:Theme.t("Review · timer paused");color:Theme.text;font.pixelSize:12}
            }
        }
        GlassPanel {scene:referenceScene
            id:timerBadge;objectName:"sessionTimer";visible:app.active&&app.settings.show_timer
            anchors.top:parent.top;anchors.topMargin:16
            x:app.settings.timer_position==="Left"?16:app.settings.timer_position==="Right"?parent.width-width-16:(parent.width-width)/2
            opacity:root.chromeVisible||!app.settings.hide_timer_on_leave?1:0
            Behavior on opacity {NumberAnimation {duration:app.settings.reduce_motion?0:220;easing.type:Easing.InOutQuad}}
            width:timerRow.implicitWidth+24;height:36;radius:12
            Accessible.role:Accessible.StaticText
            Accessible.name:app.timerText+" · "+(app.sessionLimit===0?"#"+app.ordinal:app.ordinal+" / "+app.sessionLimit)+" · "+Theme.t(app.paused?"Pause":app.state==="break"?"Break":"Session")
            HoverHandler {id:timerHover}
            ToolTip.visible:timerHover.hovered;ToolTip.delay:600
            ToolTip.text:Theme.t(app.paused?"Pause":app.state==="break"?"Break":"Session")
            TextMetrics {id:timeMeasure;font:timeDigits.font;text:"00:00"}
            TextMetrics {id:countMeasure;font:countDigits.font;text:app.sessionLimit===0?"#000":app.sessionLimit+" / "+app.sessionLimit}
            Row {id:timerRow;objectName:"sessionTimerContent";anchors.centerIn:parent;spacing:10
                ArtIcon {name:app.paused?"pause":"timer";width:14;height:14;anchors.verticalCenter:parent.verticalCenter;tint:Theme.accent}
                Text {id:timeDigits;objectName:"sessionTimeDigits";text:app.timerText;width:Math.max(timeMeasure.width,implicitWidth);horizontalAlignment:Text.AlignHCenter;font.pixelSize:18;font.family:"Consolas";font.weight:Font.Medium;color:Theme.text;anchors.verticalCenter:parent.verticalCenter}
                Rectangle {width:1;height:14;color:Theme.border;anchors.verticalCenter:parent.verticalCenter}
                Text {id:countDigits;objectName:"sessionCountDigits";text:app.sessionLimit===0?"#"+app.ordinal:app.ordinal+" / "+app.sessionLimit;width:Math.max(30,countMeasure.width,implicitWidth);horizontalAlignment:Text.AlignHCenter;color:Theme.muted;font.pixelSize:12;font.family:"Consolas";anchors.verticalCenter:parent.verticalCenter}
            }
        }

        Rectangle {objectName:"sessionProgress";visible:app.active;anchors.bottom:parent.bottom;height:3;width:parent.width*root.progressValue;color:Theme.accent}
        SessionResult {opened:app.state==="completed"&&root.resultOpen;anchors.fill:parent;z:4;scene:referenceScene;onHideRequested:root.resultOpen=false;onManageCollections:root.manageCollections()}
        BusyIndicator {anchors.centerIn:parent;running:root.loadingIndicator&&root.visible;width:48;height:48;palette.dark:Theme.accent}
    }
}
