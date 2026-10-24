import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Popup {
    id:root;objectName:"sessionPopover";popupType:Popup.Item
    signal manageCollections()
    padding:16;margins:8;closePolicy:Popup.CloseOnEscape|Popup.CloseOnPressOutside
    readonly property bool browsing:app.active&&app.freeView
    readonly property real durationMs:app.active?app.sessionDurationMs:app.settings.unlimited_time?0:(app.settings.display_time||30)*1000
    readonly property int studyLimit:app.active?app.sessionLimit:app.settings.unlimited_images?0:app.settings.num_images||10
    height:Math.min(body.implicitHeight+32,Overlay.overlay?Overlay.overlay.height-16:400)
    enter:Transition {NumberAnimation {property:"opacity";from:0;to:1;duration:app.settings.reduce_motion?0:120}}
    exit:Transition {NumberAnimation {property:"opacity";from:1;to:0;duration:app.settings.reduce_motion?0:100}}
    background:Rectangle {radius:16;color:Theme.panel;border.color:Theme.border}
    contentItem:Flickable {
        clip:true;contentWidth:width;contentHeight:body.implicitHeight;boundsBehavior:Flickable.StopAtBounds
        ScrollBar.vertical:ScrollBar {policy:ScrollBar.AsNeeded}
        ColumnLayout {id:body;width:parent.width;spacing:16
            RowLayout {Layout.fillWidth:true;spacing:8
                Text {text:Theme.t(root.browsing?"Free view":"Session");color:Theme.text;font.pixelSize:18;font.weight:Font.DemiBold;Layout.fillWidth:true}
                Rectangle {visible:app.active;implicitWidth:status.implicitWidth+16;implicitHeight:24;radius:8;color:Theme.accentSoft
                    Text {id:status;anchors.centerIn:parent;text:Theme.t(app.paused||app.state==="review"?"Pause":app.state==="break"?"Break":"In progress");color:Theme.accentText;font.pixelSize:12}
                }
            }
            RowLayout {Layout.fillWidth:true;spacing:16
                ColumnLayout {Layout.fillWidth:true;spacing:4
                    Text {text:Theme.t("Per reference");color:Theme.muted;font.pixelSize:12}
                    Text {text:root.durationMs===0?"∞":Theme.duration(root.durationMs);color:Theme.text;font.pixelSize:20;font.weight:Font.Medium}
                }
                ColumnLayout {Layout.fillWidth:true;spacing:4
                    Text {text:Theme.t("Studies");color:Theme.muted;font.pixelSize:12}
                    Text {text:root.studyLimit===0?"∞":root.studyLimit;color:Theme.text;font.pixelSize:20;font.weight:Font.Medium}
                }
            }
            Rectangle {Layout.fillWidth:true;height:1;color:Theme.border}
            Button {objectName:"sessionSourcesButton";Layout.fillWidth:true;implicitHeight:56;padding:8;hoverEnabled:true;Accessible.name:Theme.t("Session folders")
                background:Rectangle {radius:8;color:parent.down?Theme.hover:parent.hovered?Theme.raised:"transparent";border.width:parent.visualFocus?2:0;border.color:Theme.accent}
                contentItem:RowLayout {spacing:8
                    ArtIcon {name:"folder";tint:Theme.muted;width:20;height:20}
                    ColumnLayout {Layout.fillWidth:true;spacing:4
                        Text {text:Theme.t("Session folders");color:Theme.text;font.pixelSize:13}
                        Text {text:Theme.count(app.sessionImageCount,"image","images_few","images");color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;elide:Text.ElideRight}
                    }
                    ArtIcon {name:"chevron-right";width:16;height:16}
                }
                onClicked:{root.close();root.manageCollections()}
            }
            LobeButton {objectName:"sessionModeButton";Layout.fillWidth:true;iconName:root.browsing?"timer":"scan-eye";text:Theme.t(root.browsing?"Timed practice":"Free view");font.pixelSize:13;enabled:app.sessionImageCount>0&&!app.scanning;onClicked:{root.close();app.startSession(!root.browsing)}}
            LobeButton {objectName:"sessionStopButton";visible:app.active;Layout.fillWidth:true;quiet:true;iconName:"square";text:Theme.t("End session");font.pixelSize:13;onClicked:{root.close();app.stopSession()}}
        }
    }
}
