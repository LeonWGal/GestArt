import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id:root
    signal manageCollections()
    property bool compact:false
    property string namePrefix:"setup"
    property bool showDuration:true
    property bool showFolders:true
    property bool foldersExpanded:false
    readonly property int selectedFolders:app.collections.filter(c=>c.selected).length
    spacing:8
    Text {visible:root.showDuration;text:Theme.t("Time per image");color:Theme.muted;font.pixelSize:12}
    LobeSegmented {visible:root.showDuration;Layout.fillWidth:true;accessibleLabel:Theme.t("Time per image");value:app.settings.unlimited_time?null:app.settings.display_time
        options:[30,60,120,300].map(n=>({value:n,label:n<60?n+" "+Theme.t("sec"):n/60+" "+Theme.t("min")}));onSelected:n=>app.setPresetDuration(n)
    }
    RowLayout {Layout.fillWidth:true
        Text {text:Theme.t("Number of images");color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true}
        Text {text:app.settings.unlimited_images?"∞":app.settings.num_images;color:Theme.text;font.pixelSize:12}
    }
    LobeSegmented {Layout.fillWidth:true;accessibleLabel:Theme.t("Number of images");value:app.settings.unlimited_images?null:app.settings.num_images
        options:[5,10,20,30].map(n=>({value:n,label:String(n),objectName:root.namePrefix+"CountPreset"+n}));onSelected:n=>app.setPresetCount(n)
    }
    LobeButton {visible:root.showFolders;objectName:root.namePrefix+"FoldersButton";Layout.fillWidth:true;implicitHeight:root.compact?32:36;leftPadding:8;rightPadding:8;font.pixelSize:12;iconName:"folder";text:Theme.t("Session folders")+" · "+root.selectedFolders;onClicked:root.foldersExpanded=!root.foldersExpanded}
    ColumnLayout {visible:root.showFolders&&root.foldersExpanded;Layout.fillWidth:true;spacing:4
        Text {visible:app.active;Layout.fillWidth:true;text:Theme.t("For the next session");color:Theme.muted;font.pixelSize:11}
        ListView {objectName:root.namePrefix+"FolderList";Layout.fillWidth:true;Layout.preferredHeight:Math.min(160,app.collections.length*36);clip:true;spacing:4;model:app.collections
            ScrollBar.vertical:ScrollBar {policy:ScrollBar.AsNeeded}
            delegate:Button {required property var modelData;objectName:root.namePrefix+"Folder"+modelData.id;width:ListView.view.width;height:32;hoverEnabled:true
                Accessible.name:Theme.t("Use in session")+" · "+modelData.name;Accessible.checkable:true;Accessible.checked:modelData.selected
                onClicked:app.selectCollection(modelData.id,!modelData.selected)
                background:Rectangle {radius:8;color:parent.down?Theme.hover:parent.hovered?Theme.raised:"transparent";border.width:parent.visualFocus?2:0;border.color:Theme.accent}
                contentItem:RowLayout {spacing:8
                    ArtIcon {name:modelData.selected?"circle-check":"circle";tint:modelData.selected?Theme.accentText:Theme.muted;width:16;height:16}
                    Text {text:modelData.name;color:Theme.text;font.pixelSize:12;Layout.fillWidth:true;elide:Text.ElideMiddle}
                    Text {text:modelData.count;color:Theme.muted;font.pixelSize:11}
                }
            }
        }
        LobeButton {Layout.fillWidth:true;quiet:true;implicitHeight:28;font.pixelSize:12;text:Theme.t("Manage collections");iconName:"folder-plus";onClicked:root.manageCollections()}
    }
}
