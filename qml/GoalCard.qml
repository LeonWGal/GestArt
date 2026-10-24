import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
    id:root
    signal editGoal()
    implicitHeight:Math.max(width<380?176:156,goalContent.implicitHeight+32);radius:16;color:Theme.raised;border.color:Theme.border
    ColumnLayout {id:goalContent;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:16;spacing:16
        RowLayout {Layout.fillWidth:true
            Text {text:Theme.t("Today's goal");color:Theme.text;font.pixelSize:16;font.weight:Font.Medium;Layout.fillWidth:true}
            IconButton {iconName:"settings-2";hint:Theme.t("Edit practice goal");width:28;height:28;onClicked:root.editGoal()}
        }
        Text {text:Theme.goalSummary(app.practice);color:Theme.text;font.pixelSize:root.width<380?18:22;font.weight:Font.Medium;Layout.fillWidth:true;wrapMode:Text.WordWrap}
        ProgressBar {Layout.fillWidth:true;from:0;to:1;value:app.practice.goalProgress||0;implicitHeight:8
            background:Rectangle {implicitHeight:8;radius:4;color:Theme.canvas}
            contentItem:Item {Rectangle {width:parent.width*parent.parent.visualPosition;height:parent.height;radius:4;color:Theme.accent}}
        }
        Text {text:app.practice.goalReached?Theme.t("Goal reached. A good day of practice."):Theme.t("Every completed study counts.");color:app.practice.goalReached?Theme.accentText:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap}
    }
}
