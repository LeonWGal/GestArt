import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Popup {
    id:root;objectName:"mediaPreview"
    anchors.centerIn:parent
    width:parent.width;height:parent.height
    padding:0;modal:true;focus:true
    closePolicy:Popup.CloseOnEscape
    property real zoom:1
    property real panX:0
    property real panY:0
    property int angle:0
    property real decodeZoom:1
    function resetView(){zoom=1;panX=0;panY=0;angle=0;decodeZoom=1}
    function zoomAt(factor,x,y){
        const next=Math.max(.1,Math.min(12,zoom*factor));
        panX=(panX-(x-viewport.width/2))*next/zoom+(x-viewport.width/2);
        panY=(panY-(y-viewport.height/2))*next/zoom+(y-viewport.height/2);
        zoom=next;decodeDelay.restart()
    }
    Timer {id:decodeDelay;interval:160;onTriggered:root.decodeZoom=root.zoom}
    Connections {target:app;function onPreviewChanged(){if(app.previewPath!==""){root.resetView();if(!root.opened)root.open()}else root.close()}}
    onOpened:contentItem.forceActiveFocus()
    onAboutToHide:app.closePreview()
    background:Rectangle {color:Theme.canvas}
    enter:Transition {NumberAnimation {property:"opacity";from:0;to:1;duration:app.settings.reduce_motion?0:120}}
    exit:Transition {NumberAnimation {property:"opacity";from:1;to:0;duration:app.settings.reduce_motion?0:100}}
    contentItem:ColumnLayout {spacing:0;focus:true
        Keys.onEscapePressed:event=>{root.close();event.accepted=true}
        Rectangle {Layout.fillWidth:true;implicitHeight:60;color:Theme.panel
            RowLayout {anchors.fill:parent;anchors.margins:12;spacing:8
                IconButton {objectName:"closePreviewButton";iconName:"arrow-left";hint:Theme.t("Close preview")+" · Esc";onClicked:root.close()}
                ColumnLayout {Layout.fillWidth:true;spacing:3
                    Text {text:Theme.t("Preview");color:Theme.muted;font.pixelSize:11}
                    Text {Layout.fillWidth:true;text:app.previewPath.split(/[\\/]/).pop();elide:Text.ElideMiddle;color:Theme.text;font.pixelSize:13;font.weight:Font.Medium}
                }
                Text {text:(app.previewIndex+1)+" / "+app.previewCount;color:Theme.muted;font.pixelSize:12}
                IconButton {iconName:"x";hint:Theme.t("Close preview");onClicked:root.close()}
            }
        }
        Item {id:viewport;Layout.fillWidth:true;Layout.fillHeight:true;clip:true
            Item {x:(viewport.width-width)/2+root.panX;y:(viewport.height-height)/2+root.panY;width:root.angle%180===0?viewport.width:viewport.height;height:root.angle%180===0?viewport.height:viewport.width
                scale:root.zoom;rotation:root.angle
                Image {id:art;objectName:"previewImage";anchors.fill:parent;source:root.visible?app.previewImage:"";asynchronous:true;cache:false;fillMode:Image.PreserveAspectFit
                    onStatusChanged:if(status===Image.Ready)app.previewPresented(source.toString())
                    sourceSize:Qt.size(Math.min(8192,Math.max(64,parent.width*Screen.devicePixelRatio*Math.max(1,root.decodeZoom))),Math.min(8192,Math.max(64,parent.height*Screen.devicePixelRatio*Math.max(1,root.decodeZoom))));smooth:true
                }
            }
            MouseArea {anchors.fill:parent;property real lastX;property real lastY;cursorShape:pressed?Qt.ClosedHandCursor:Qt.OpenHandCursor
                onPressed:mouse=>{lastX=mouse.x;lastY=mouse.y}
                onPositionChanged:mouse=>{if(pressed){root.panX+=mouse.x-lastX;root.panY+=mouse.y-lastY;lastX=mouse.x;lastY=mouse.y}}
                onDoubleClicked:root.resetView()
                onWheel:wheel=>{const d=wheel.pixelDelta.y?wheel.pixelDelta.y/60:wheel.angleDelta.y/120;root.zoomAt(Math.pow(1.12,d),wheel.x,wheel.y);wheel.accepted=true}
            }
            BusyIndicator {anchors.centerIn:parent;running:art.status===Image.Loading&&root.visible;width:40;height:40}
            LobeEmpty {anchors.centerIn:parent;width:Math.min(320,parent.width-24);visible:art.status===Image.Error;iconName:"image";title:Theme.t("Preview unavailable");description:app.previewPath}
        }
        Rectangle {Layout.fillWidth:true;implicitHeight:root.width<520?62:82;color:Theme.panel
            ColumnLayout {anchors.centerIn:parent;spacing:6
                RowLayout {Layout.alignment:Qt.AlignHCenter;spacing:8
                    IconButton {objectName:"previewPrevious";iconName:"chevron-left";hint:Theme.t("Previous");enabled:app.previewIndex>0;onClicked:app.stepPreview(-1)}
                    IconButton {iconName:"zoom-out";hint:Theme.t("Zoom out");onClicked:root.zoomAt(1/1.2,viewport.width/2,viewport.height/2)}
                    IconButton {iconName:"scan";hint:Theme.t("Fit");onClicked:root.resetView()}
                    IconButton {iconName:"zoom-in";hint:Theme.t("Zoom in");onClicked:root.zoomAt(1.2,viewport.width/2,viewport.height/2)}
                    IconButton {iconName:"rotate-cw";hint:Theme.t("Rotate 90°");onClicked:{root.angle=(root.angle+90)%360;root.panX=0;root.panY=0}}
                    IconButton {objectName:"previewNext";iconName:"chevron-right";hint:Theme.t("Next");enabled:app.previewIndex+1<app.previewCount;onClicked:app.stepPreview(1)}
                }
                Text {visible:root.width>=520;text:Theme.t("Preview controls");color:Theme.muted;font.pixelSize:11;Layout.alignment:Qt.AlignHCenter}
            }
        }
    }
    Shortcut {sequence:"Left";enabled:root.opened;onActivated:app.stepPreview(-1)}
    Shortcut {sequence:"Right";enabled:root.opened;onActivated:app.stepPreview(1)}
    Shortcut {sequences:["Ctrl++","Ctrl+="];enabled:root.opened;onActivated:root.zoomAt(1.1,viewport.width/2,viewport.height/2)}
    Shortcut {sequence:"Ctrl+-";enabled:root.opened;onActivated:root.zoomAt(1/1.1,viewport.width/2,viewport.height/2)}
    Shortcut {sequence:"Ctrl+0";enabled:root.opened;onActivated:root.resetView()}
}

