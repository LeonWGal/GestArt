import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
    id:root;objectName:"homeScroll";contentWidth:availableWidth;clip:true
    signal onlineSources()
    signal downloader()
    signal savedSets()
    signal addFolder()
    signal manageCollections()
    signal editSession()
    signal editGoal()
    signal dataSettings()
    readonly property bool small:width<600
    readonly property bool shortWindow:height<400
    readonly property int inset:small?16:32
    readonly property int selectedFolders:app.collections.filter(c=>c.selected).length
    ColumnLayout {
        width:Math.max(100,Math.min(1040,root.availableWidth-2*root.inset));x:(root.availableWidth-width)/2;spacing:root.shortWindow?16:24
        Item {height:root.shortWindow?0:8}
        ColumnLayout {visible:!root.shortWindow;Layout.fillWidth:true;spacing:8
            Text {Layout.fillWidth:true;text:Theme.t(app.active?"Continue your practice":"Make time to draw");color:Theme.text;font.pixelSize:root.small?24:30;font.weight:Font.DemiBold;wrapMode:Text.WordWrap}
            Text {Layout.fillWidth:true;text:Theme.t("Choose a rhythm. The rest can wait.");color:Theme.muted;font.pixelSize:14;wrapMode:Text.WordWrap}
        }
        GridLayout {Layout.fillWidth:true;columns:root.width>=900?2:1;columnSpacing:24;rowSpacing:24
            Rectangle {Layout.fillWidth:true;Layout.preferredWidth:root.width>=900?620:1;Layout.alignment:Qt.AlignTop;implicitHeight:sessionContent.implicitHeight+32;radius:16;color:Theme.raised;border.color:Theme.border
                ColumnLayout {id:sessionContent;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:16;spacing:16
                    RowLayout {Layout.fillWidth:true
                        Text {Layout.fillWidth:true;text:Theme.t(app.active?"Session in progress":app.collections.length===0?"Your first session":"New session");color:Theme.text;font.pixelSize:20;font.weight:Font.Medium}
                        IconButton {width:32;height:32;iconName:"settings-2";hint:Theme.t("Session settings");onClicked:root.editSession()}
                    }
                    Text {Layout.fillWidth:true;text:app.scanning?Theme.t("Scanning")+" · "+app.imageCount:app.collections.length===0?Theme.t("Add a folder of references to get started."):app.sessionImageCount===0?Theme.t("Choose folders for the session"):app.active?Theme.count(app.completed,"study","studies_few","studies")+" · "+Theme.duration(app.sessionPracticeMs):Theme.count(app.sessionImageCount,"image","images_few","images")+" · "+Theme.count(root.selectedFolders,"collection","collections_few","collections");color:Theme.muted;font.pixelSize:13;wrapMode:Text.WordWrap}
                    RowLayout {Layout.fillWidth:true;spacing:8
                        LobeButton {objectName:"homeOnlineSources";Layout.fillWidth:true;Layout.preferredWidth:1;text:Theme.t(root.width<420?"Websites":"Online viewing");font.pixelSize:12;iconName:"globe";quiet:true;onClicked:root.onlineSources()}
                        LobeButton {objectName:"homeDownloader";Layout.fillWidth:true;Layout.preferredWidth:1;text:Theme.t("Download references");font.pixelSize:12;iconName:"download";quiet:true;onClicked:root.downloader()}
                    }
                    LobeButton {objectName:"homeSavedSets";Layout.fillWidth:true;text:Theme.t("Saved sets")+" · "+downloads.savedSets.length;iconName:"folder-open";quiet:true;onClicked:root.savedSets()}
                    RowLayout {Layout.fillWidth:true;spacing:8
                        LobeButton {objectName:"homeStartButton";Layout.fillWidth:true;primary:true;iconName:app.collections.length===0?"folder-plus":"play";text:app.collections.length===0?Theme.t("Add folder"):app.active?Theme.t("Return to session"):Theme.t("Start session");font.pixelSize:13;enabled:!app.scanning&&(app.collections.length===0||app.active||app.sessionImageCount>0);onClicked:{if(app.collections.length===0)root.addFolder();else if(app.active)app.page="studio";else app.startSession()}}
                        IconButton {visible:!app.active&&app.collections.length>0;iconName:"scan-eye";hint:Theme.t("Free view");enabled:app.sessionImageCount>0&&!app.scanning;onClicked:app.startSession(true)}
                    }
                    Rectangle {visible:app.collections.length>0;Layout.fillWidth:true;height:1;color:Theme.border}
                    SessionSetup {namePrefix:"home";visible:app.collections.length>0;Layout.fillWidth:true;compact:root.small;showDuration:true;onManageCollections:root.manageCollections()}
                    Text {visible:app.active;Layout.fillWidth:true;text:Theme.t("For the next session");color:Theme.muted;font.pixelSize:12}
                }
            }
            ColumnLayout {Layout.fillWidth:true;Layout.preferredWidth:root.width>=900?360:1;Layout.alignment:Qt.AlignTop;spacing:16
                RowLayout {Layout.fillWidth:true
                    Text {Layout.fillWidth:true;text:Theme.t("Today");color:Theme.text;font.pixelSize:18;font.weight:Font.Medium}
                    LobeButton {text:Theme.t("Practice");quiet:true;iconName:"arrow-up-right";font.pixelSize:12;implicitHeight:32;onClicked:app.page="practice"}
                }
                ActivityStrip {Layout.fillWidth:true}
                GoalCard {Layout.fillWidth:true;onEditGoal:root.editGoal()}
                Rectangle {Layout.fillWidth:true;implicitHeight:weekContent.implicitHeight+32;radius:16;color:Theme.raised;border.color:Theme.border
                    ColumnLayout {id:weekContent;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:16;spacing:16
                        Text {text:Theme.t("This week");color:Theme.text;font.pixelSize:16;font.weight:Font.Medium}
                        WeekStrip {Layout.fillWidth:true}
                    }
                }
            }
        }
        LobeNotice {objectName:"homeStorageNotice";visible:!app.settings.save_history||(!app.historyLoading&&!app.historyHealthy);Layout.fillWidth:true;warning:app.settings.save_history&&!app.historyHealthy;text:Theme.t(!app.settings.save_history?"History is off. New studies will not be added to statistics.":"Practice is available. History cannot be saved right now.");actionText:Theme.t("Data");onActionRequested:root.dataSettings()}
        Item {height:16}
    }
}
