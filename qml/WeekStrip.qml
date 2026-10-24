import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
RowLayout {
    id:root;spacing:4;implicitHeight:64
    Repeater {model:app.practice.week||[]
        delegate:ColumnLayout {required property var modelData;Layout.fillWidth:true;spacing:8
            Text {text:Theme.dateLabel(modelData.date,"ddd");color:Theme.muted;font.pixelSize:11;Layout.alignment:Qt.AlignHCenter}
            Rectangle {Layout.alignment:Qt.AlignHCenter;width:root.width<340?24:32;height:width;radius:height/2;color:modelData.goalReached?Theme.accent:modelData.studies>0?Theme.accentSoft:Theme.canvas;border.color:modelData.today?Theme.accent:Theme.border
                Text {anchors.centerIn:parent;text:modelData.date.slice(-2).replace(/^0/,"");color:modelData.goalReached?Theme.primaryText:Theme.text;font.pixelSize:12;opacity:modelData.future?.45:1}
                HoverHandler {id:hover}
                ToolTip.visible:hover.hovered;ToolTip.text:Theme.dateLabel(modelData.date,"d MMMM")+" · "+Theme.duration(modelData.durationMs)+" · "+Theme.count(modelData.studies,"study","studies_few","studies")
                Accessible.role:Accessible.StaticText;Accessible.name:ToolTip.text
            }
        }
    }
}
