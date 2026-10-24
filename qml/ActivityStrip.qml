import QtQuick
import QtQuick.Layouts
GridLayout {
    id:root;objectName:"activityStrip"
    columns:3;columnSpacing:8;rowSpacing:8
    Repeater {model:[{icon:"timer",value:Theme.duration(app.practice.todayMs||0),label:Theme.t("Today")},{icon:"circle-check",value:String(app.practice.todayStudies||0),label:Theme.t("Studies today")},{icon:"flame",value:String(app.practice.currentStreak||0),label:Theme.t("Day streak")}]
        delegate:Rectangle {required property var modelData;Layout.fillWidth:true;implicitHeight:root.width<340?64:80;radius:12;color:Qt.rgba(Theme.raised.r,Theme.raised.g,Theme.raised.b,.55)
            ColumnLayout {anchors.fill:parent;anchors.margins:root.width<340?8:12;spacing:4
                RowLayout {Layout.fillWidth:true;spacing:4
                    ArtIcon {visible:root.width>=340;name:modelData.icon;width:14;height:14;tint:Theme.muted}
                    Text {text:modelData.value;color:Theme.text;font.pixelSize:root.width<340?16:20;font.weight:Font.Medium;Layout.fillWidth:true;elide:Text.ElideRight}
                }
                Text {text:modelData.label;color:Theme.muted;font.pixelSize:11;Layout.fillWidth:true;elide:Text.ElideRight}
            }
        }
    }
}
