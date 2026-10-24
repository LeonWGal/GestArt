import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
    id:root;implicitHeight:256;radius:16;color:Theme.raised;border.color:Theme.border
    ColumnLayout {anchors.fill:parent;anchors.margins:16;spacing:16
        RowLayout {Layout.fillWidth:true
            Text {text:Theme.t("Practice rhythm");color:Theme.text;font.pixelSize:16;font.weight:Font.Medium;Layout.fillWidth:true}
            Text {text:Theme.t("Last")+" "+(app.practice.chartDays||30)+" "+Theme.t("days");color:Theme.muted;font.pixelSize:12}
        }
        Flickable {id:chartView;Layout.fillWidth:true;Layout.fillHeight:true;clip:true;contentWidth:Math.max(width,(app.practice.chartDays||30)*8);contentHeight:height;interactive:contentWidth>width
            ScrollBar.horizontal:ScrollBar {policy:ScrollBar.AsNeeded}
            Row {width:chartView.contentWidth;height:chartView.height-12;spacing:app.practice.chartDays===7?12:4
            Repeater {model:app.practice.chart||[]
                delegate:Item {required property var modelData;width:(parent.width-parent.spacing*Math.max(0,(app.practice.chartDays||30)-1))/(app.practice.chartDays||30);height:parent.height
                    Rectangle {anchors.bottom:parent.bottom;width:parent.width;height:modelData.durationMs>0?Math.max(8,parent.height*modelData.durationMs/Math.max(1,app.practice.chartMaxMs)):2;radius:Math.min(4,width/2);color:modelData.durationMs>0?Theme.accentText:Theme.border}
                    HoverHandler {id:hover}
                    ToolTip.visible:hover.hovered;ToolTip.text:Theme.dateLabel(modelData.date,"d MMMM")+" · "+Theme.duration(modelData.durationMs)+" · "+Theme.count(modelData.studies,"study","studies_few","studies")
                    Accessible.role:Accessible.StaticText;Accessible.name:ToolTip.text
                }
            }
            }
        }
        RowLayout {Layout.fillWidth:true
            Text {text:app.practice.chart&&app.practice.chart.length?Theme.dateLabel(app.practice.chart[0].date,"d MMM"):"";color:Theme.muted;font.pixelSize:11}
            Item {Layout.fillWidth:true}
            Text {text:Theme.t("Today");color:Theme.muted;font.pixelSize:11}
        }
    }
}
