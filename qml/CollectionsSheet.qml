import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Drawer {
    id:sheet;edge:Qt.RightEdge;modal:true;dim:true
    signal onlineSources()
    signal addFolder()
    signal addFiles()
    background:Rectangle {color:Theme.panel;border.color:Theme.border}
    ColumnLayout {anchors.fill:parent;anchors.margins:sheet.height<320?8:16;spacing:sheet.height<320?8:16
        RowLayout {Layout.fillWidth:true
            Text {text:Theme.t("Session folders");color:Theme.text;font.pixelSize:sheet.width<320?16:20;font.weight:Font.DemiBold;Layout.fillWidth:true;elide:Text.ElideRight}
            IconButton {iconName:"x";width:28;height:28;hint:Theme.t("Close");onClicked:sheet.close()}
        }
        Text {visible:sheet.height>=320;text:Theme.t("Check the collections for your next session.");color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap}
        RowLayout {visible:sheet.height>=280;Layout.fillWidth:true;spacing:8
            LobeButton {text:Theme.t("Select all");quiet:true;implicitHeight:28;implicitWidth:1;Layout.fillWidth:true;onClicked:app.selectAllCollections(true)}
            LobeButton {text:Theme.t("Clear selection");quiet:true;implicitHeight:28;implicitWidth:1;Layout.fillWidth:true;onClicked:app.selectAllCollections(false)}
        }
        CollectionList {Layout.fillWidth:true;Layout.fillHeight:true}
        RowLayout {visible:app.canUndoCollectionRemoval&&sheet.height>=320;Layout.fillWidth:true
            Text {text:Theme.t("Files stay on disk");color:Theme.muted;font.pixelSize:11;Layout.fillWidth:true;elide:Text.ElideRight}
            LobeButton {text:Theme.t("Undo");quiet:true;implicitHeight:28;onClicked:app.undoCollectionRemoval()}
        }
        RowLayout {Layout.fillWidth:true;spacing:8
            LobeButton {text:Theme.t("Add folder");font.pixelSize:12;leftPadding:8;rightPadding:8;Accessible.name:Theme.t("Add folder");iconName:"folder-plus";onClicked:sheet.addFolder()}
            IconButton {objectName:"collectionOnlineSources";iconName:"globe";hint:Theme.t("Online references");onClicked:{sheet.close();sheet.onlineSources()}}
            IconButton {iconName:"image-plus";hint:Theme.t("Select images");onClicked:sheet.addFiles()}
            IconButton {visible:app.canUndoCollectionRemoval&&sheet.height<320;iconName:"undo-2";implicitWidth:28;implicitHeight:32;hint:Theme.t("Undo removal");onClicked:app.undoCollectionRemoval()}
            LobeButton {text:sheet.width<340?"":Theme.t("Start session");Accessible.name:Theme.t("Start session");primary:true;iconName:"play";Layout.fillWidth:true;enabled:app.sessionImageCount>0&&!app.scanning;onClicked:{app.startSession();sheet.close()}}
        }
    }
}
