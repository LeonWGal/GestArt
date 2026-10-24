import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
    id:root;objectName:"practiceCalendar"
    property var selectedDay:null
    implicitHeight:calendarColumn.implicitHeight+32;radius:16;color:Theme.raised;border.color:Theme.border
    Connections {target:app;function onPracticeChanged(){if(root.selectedDay){const days=app.practice.calendar||[];root.selectedDay=days.find(day=>day.date===root.selectedDay.date)||null}}}
    ColumnLayout {id:calendarColumn;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:16;spacing:16
        RowLayout {Layout.fillWidth:true;spacing:4
            Text {text:Theme.monthLabel(app.practice.month||app.practice.today);color:Theme.text;font.pixelSize:root.width<280?14:20;font.weight:Font.Medium;Layout.fillWidth:true;elide:Text.ElideRight}
            IconButton {iconName:"chevron-left";hint:Theme.t("Previous month");width:28;height:28;onClicked:{root.selectedDay=null;app.shiftTrackerMonth(-1)}}
            IconButton {iconName:"chevron-right";hint:Theme.t("Next month");width:28;height:28;enabled:!!app.practice.month&&(app.practice.month.slice(0,7)<app.practice.today.slice(0,7));onClicked:{root.selectedDay=null;app.shiftTrackerMonth(1)}}
        }
        CalendarGrid {Layout.fillWidth:true;selectedDate:root.selectedDay?root.selectedDay.date:"";onSelected:day=>root.selectedDay=day}
        Text {text:root.selectedDay?Theme.dateLabel(root.selectedDay.date,"d MMMM")+" · "+Theme.duration(root.selectedDay.durationMs)+" · "+Theme.count(root.selectedDay.studies,"study","studies_few","studies"):Theme.t("Choose a day to see your practice.");color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap}
        RowLayout {Layout.fillWidth:true;spacing:8
            Rectangle {width:12;height:12;radius:4;color:Theme.accentSoft;border.color:Theme.accent}
            Text {text:Theme.t("Practiced");color:Theme.muted;font.pixelSize:11;Layout.fillWidth:true}
            Rectangle {width:12;height:12;radius:4;color:Theme.accent}
            Text {text:Theme.t("Goal reached");color:Theme.muted;font.pixelSize:11;Layout.fillWidth:true}
        }
    }
}
