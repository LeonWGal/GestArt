import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
    id:root;objectName:"practiceScroll";contentWidth:availableWidth;clip:true
    signal editGoal()
    signal dataSettings()
    readonly property int inset:width<600?16:32
    ColumnLayout {width:Math.max(100,Math.min(1120,root.availableWidth-2*root.inset));x:(root.availableWidth-width)/2;spacing:24
        Item {height:root.height<400?0:8}
        ColumnLayout {Layout.fillWidth:true;spacing:12
            Text {visible:root.height>=400;Layout.fillWidth:true;text:Theme.t("Practice overview");color:Theme.text;font.pixelSize:24;font.weight:Font.DemiBold}
            LobeSegmented {Layout.fillWidth:true;accessibleLabel:Theme.t("Practice period");value:app.statsPeriod
                options:[7,30,90,0].map(n=>({value:n,label:n===0?Theme.t("All time"):n+" "+Theme.t("days")}));onSelected:n=>app.statsPeriod=n
            }
        }
        GridLayout {Layout.fillWidth:true;columns:root.width>=900?4:2;rowSpacing:8;columnSpacing:8
            MetricCard {Layout.fillWidth:true;Layout.preferredWidth:1;dense:true;label:Theme.t("Practice time");value:Theme.duration(app.practice.totalMs||0);iconName:"timer"}
            MetricCard {Layout.fillWidth:true;Layout.preferredWidth:1;dense:true;label:Theme.t("Completed studies");value:String(app.practice.totalStudies||0);iconName:"circle-check"}
            MetricCard {Layout.fillWidth:true;Layout.preferredWidth:1;dense:true;label:Theme.t("Active days");value:String(app.practice.activeDays||0);iconName:"calendar"}
            MetricCard {Layout.fillWidth:true;Layout.preferredWidth:1;dense:true;label:Theme.t("Average study");value:Theme.timeCode(app.practice.averageMs||0);iconName:"brush"}
        }
        Rectangle {visible:(app.practice.totalStudies||0)===0;Layout.fillWidth:true;implicitHeight:emptyContent.implicitHeight+32;radius:16;color:Theme.raised;border.color:Theme.border
            ColumnLayout {id:emptyContent;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:16;spacing:12
                Text {Layout.fillWidth:true;text:Theme.t("Your practice starts with one study.");color:Theme.text;font.pixelSize:16;wrapMode:Text.WordWrap}
                Text {Layout.fillWidth:true;text:Theme.t("Only completed studies contribute time. Free view and skipped images are excluded.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                LobeButton {text:Theme.t("Home");iconName:"home";primary:true;onClicked:app.page="home"}
            }
        }
        GridLayout {Layout.fillWidth:true;columns:root.width>=900?2:1;columnSpacing:16;rowSpacing:16
            GoalCard {Layout.fillWidth:true;Layout.preferredWidth:1;onEditGoal:root.editGoal()}
            Rectangle {Layout.fillWidth:true;Layout.preferredWidth:1;implicitHeight:Math.max(156,weekContent.implicitHeight+32);radius:16;color:Theme.raised;border.color:Theme.border
                ColumnLayout {id:weekContent;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:16;spacing:16
                    RowLayout {Layout.fillWidth:true
                        Text {Layout.fillWidth:true;text:Theme.t("This week");color:Theme.text;font.pixelSize:16;font.weight:Font.Medium}
                        ArtIcon {name:"flame";width:18;height:18;tint:Theme.accentText}
                    }
                    WeekStrip {Layout.fillWidth:true}
                    Text {Layout.fillWidth:true;text:Theme.t("Current streak")+": "+(app.practice.currentStreak||0)+" · "+Theme.t("Best streak")+": "+(app.practice.bestStreak||0);color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                }
            }
        }
        GridLayout {Layout.fillWidth:true;columns:root.width>=1000?2:1;rowSpacing:16;columnSpacing:16
            PracticeChart {objectName:"practiceChart";Layout.fillWidth:true;Layout.preferredWidth:1;Layout.alignment:Qt.AlignTop;implicitHeight:280}
            PracticeCalendar {Layout.fillWidth:true;Layout.preferredWidth:1;Layout.alignment:Qt.AlignTop}
        }
        RowLayout {Layout.fillWidth:true
            Text {Layout.fillWidth:true;text:Theme.t("Only completed studies contribute time. Free view and skipped images are excluded.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
            IconButton {iconName:"history";hint:Theme.t("History");onClicked:app.page="history"}
        }
        Text {visible:(app.practice.legacyCount||0)>0;Layout.fillWidth:true;text:Theme.t("Imported history without dates is kept in History and excluded from charts.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
        LobeNotice {visible:!app.settings.save_history||(!app.historyLoading&&!app.historyHealthy);Layout.fillWidth:true;warning:app.settings.save_history&&!app.historyHealthy;text:Theme.t(!app.settings.save_history?"History is off. New studies will not be added to statistics.":"Practice is available. History cannot be saved right now.");actionText:Theme.t("Data");onActionRequested:root.dataSettings()}
        Item {height:16}
    }
}
