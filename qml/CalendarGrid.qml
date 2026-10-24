import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GridLayout {
    id:root;columns:7;columnSpacing:8;rowSpacing:8
    signal selected(var day)
    property string selectedDate:""
    Repeater {model:["Mon","Tue","Wed","Thu","Fri","Sat","Sun"]
        delegate:Text {required property string modelData;text:Theme.t(modelData);color:Theme.muted;font.pixelSize:12;horizontalAlignment:Text.AlignHCenter;Layout.fillWidth:true}
    }
    Repeater {model:app.practice.calendar||[]
        delegate:Button {id:day;required property var modelData;objectName:"calendarDay-"+modelData.date;Layout.fillWidth:true;Layout.preferredHeight:root.width<360?32:48;implicitWidth:1;padding:0;hoverEnabled:true
            enabled:modelData.inMonth&&!modelData.future;opacity:modelData.inMonth?(modelData.future?.42:1):.2
            onClicked:root.selected(modelData)
            Accessible.name:Theme.dateLabel(modelData.date,"d MMMM yyyy")+" · "+Theme.count(modelData.studies,"study","studies_few","studies")+" · "+Theme.duration(modelData.durationMs)
            Accessible.description:modelData.goalReached?Theme.t("Goal reached"):modelData.studies>0?Theme.t("Practiced"):Theme.t("No practice")
            ToolTip.visible:hovered;ToolTip.text:Accessible.name
            background:Rectangle {radius:8;color:day.modelData.goalReached?Theme.accent:day.modelData.studies>0?Theme.accentSoft:day.hovered?Theme.hover:Theme.canvas;border.color:day.activeFocus||day.modelData.today||root.selectedDate===day.modelData.date?Theme.accent:Theme.border;border.width:day.modelData.today||root.selectedDate===day.modelData.date?2:1}
            contentItem:Text {text:day.modelData.date.slice(-2).replace(/^0/,"");color:day.modelData.goalReached?Theme.primaryText:Theme.text;font.pixelSize:12;horizontalAlignment:Text.AlignHCenter;verticalAlignment:Text.AlignVCenter}
        }
    }
}
