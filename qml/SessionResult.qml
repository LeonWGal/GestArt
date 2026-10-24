import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
    id:root;objectName:"sessionResult"
    property Item scene:null
    property bool opened:false
    opacity:opened?1:0
    visible:opacity>0
    enabled:opened
    Behavior on opacity {NumberAnimation {duration:app.settings.reduce_motion?0:180;easing.type:Easing.InOutQuad}}
    signal hideRequested()
    signal manageCollections()
    readonly property bool shortView:height<360
    readonly property bool narrow:width<400
    // The photo remains live behind this local frosted surface.
    GlassPanel {
        id:card;objectName:"resultCard";scene:root.scene;radius:20;blurRadius:18
        width:Math.min(480,root.width-16);height:Math.min(body.implicitHeight+32,root.height-(root.shortView?16:128))
        anchors.centerIn:parent
        MouseArea {anchors.fill:parent}
        Flickable {
            id:scroll;objectName:"resultScroll";anchors.fill:parent;anchors.margins:root.shortView?12:16
            contentWidth:width;contentHeight:body.implicitHeight;clip:true;boundsBehavior:Flickable.StopAtBounds
            ScrollBar.vertical:ScrollBar {policy:ScrollBar.AsNeeded}
            ColumnLayout {
                id:body;width:scroll.width;spacing:root.shortView?8:12
                RowLayout {Layout.fillWidth:true;spacing:8
                    ArtIcon {visible:!root.shortView;name:"circle-check";width:28;height:width;tint:Theme.accentText}
                    Text {objectName:"resultHeading";text:Theme.t("Session completed");color:Theme.text;font.pixelSize:root.shortView?14:24;font.weight:Font.DemiBold;Layout.fillWidth:true;wrapMode:Text.WordWrap}
                    IconButton {objectName:"resultHideButton";width:28;height:28;iconName:"x";hint:Theme.t("Hide results")+" · Esc";onClicked:root.hideRequested()}
                }
                RowLayout {Layout.fillWidth:true;spacing:8
                    ColumnLayout {Layout.fillWidth:true;Layout.preferredWidth:1;spacing:2
                        Text {Layout.fillWidth:true;text:app.completed;color:Theme.text;font.pixelSize:root.shortView?20:28;font.weight:Font.Medium}
                        Text {Layout.fillWidth:true;text:Theme.t("Studies");color:Theme.muted;font.pixelSize:12;elide:Text.ElideRight}
                    }
                    ColumnLayout {Layout.fillWidth:true;Layout.preferredWidth:1;spacing:2
                        Text {Layout.fillWidth:true;text:Theme.duration(app.sessionPracticeMs);color:Theme.text;font.pixelSize:root.shortView?20:28;font.weight:Font.Medium;elide:Text.ElideRight}
                        Text {text:Theme.t("Drawing time");color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;elide:Text.ElideRight}
                    }
                }
                LobeButton {objectName:"sessionReplayButton";Layout.fillWidth:true;implicitHeight:root.shortView?32:40;leftPadding:8;rightPadding:8;primary:true;iconName:"play";text:Theme.t("Another session");font.pixelSize:root.shortView?12:14;enabled:app.sessionImageCount>0&&!app.scanning;onClicked:app.startSession()}
                RowLayout {Layout.fillWidth:true;spacing:4
                    LobeButton {objectName:"sessionResultHomeButton";Layout.fillWidth:true;implicitWidth:1;implicitHeight:32;leftPadding:4;rightPadding:4;font.pixelSize:12;iconName:root.narrow?"":"home";text:Theme.t("Home");quiet:true;onClicked:app.page="home"}
                    LobeButton {objectName:"sessionResultPracticeButton";Layout.fillWidth:true;implicitWidth:1;implicitHeight:32;leftPadding:4;rightPadding:4;font.pixelSize:12;iconName:root.narrow?"":"chart";text:Theme.t("Practice");quiet:true;onClicked:app.page="practice"}
                }
                Rectangle {Layout.fillWidth:true;height:1;color:Theme.border}
                Text {text:Theme.t("Next session");color:Theme.text;font.pixelSize:14;font.weight:Font.Medium}
                SessionSetup {namePrefix:"result";Layout.fillWidth:true;compact:root.shortView;showDuration:false;onManageCollections:root.manageCollections()}
                ActivityStrip {Layout.fillWidth:true}
                Text {Layout.fillWidth:true;text:Theme.t(!app.settings.save_history?"Practice history is off":app.historyHealthy?"Saved to Practice":"History needs attention");color:app.settings.save_history&&!app.historyHealthy?Theme.danger:Theme.muted;font.pixelSize:11;wrapMode:Text.WordWrap}
            }
        }
    }
}
