import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
    id:root
    signal addFolder()
    signal addFiles()
    signal settings()
    signal savedSets()
    signal manageCollections()
    signal toggleMode()
    property bool compact:false
    property bool allowToggle:false
    readonly property bool hasReference:app.currentImage!==""
    color:Theme.canvas

    component NavigationButton: Button {
        id:control
        property string label:""
        property string iconName:"home"
        property bool current:false
        Layout.fillWidth:true
        implicitHeight:40;padding:0;hoverEnabled:true
        Accessible.name:label;Accessible.checkable:current;Accessible.checked:current
        HoverHandler {cursorShape:Qt.PointingHandCursor}
        background:Rectangle {
            radius:10;color:control.current?Theme.accentSoft:"transparent"
            border.width:control.visualFocus?2:0;border.color:Theme.accent
            Rectangle {
                anchors.fill:parent;radius:parent.radius;color:Theme.text
                opacity:control.down?.10:control.hovered?.05:0
                Behavior on opacity {NumberAnimation {duration:app.settings.reduce_motion?0:control.down?50:120}}
            }
        }
        contentItem:RowLayout {
            spacing:12
            Item {
                Layout.leftMargin:root.compact?0:12
                Layout.preferredWidth:20;Layout.fillWidth:root.compact;implicitHeight:20
                ArtIcon {anchors.centerIn:parent;name:control.iconName;width:20;height:20;tint:control.current?Theme.accentText:Theme.muted}
            }
            Text {
                visible:!root.compact;Layout.fillWidth:true;Layout.rightMargin:12
                text:control.label;color:control.current?Theme.text:Theme.muted
                font.pixelSize:13;font.weight:control.current?Font.Medium:Font.Normal;elide:Text.ElideRight
            }
        }
        ToolTip.visible:(hovered||visualFocus)&&(root.compact||labelTextMetrics.advanceWidth>width-56)
        ToolTip.text:label;ToolTip.delay:400
        TextMetrics {id:labelTextMetrics;font.pixelSize:13;text:control.label}
    }
    ColumnLayout {
        anchors.fill:parent;anchors.margins:root.compact?8:12;spacing:0
        RowLayout {
            Layout.fillWidth:true;Layout.preferredHeight:40;Layout.bottomMargin:16;spacing:8
            Item {
                Layout.alignment:Qt.AlignHCenter;Layout.preferredWidth:32;Layout.preferredHeight:32
                Rectangle {
                    anchors.fill:parent;visible:!root.compact||!root.allowToggle
                    radius:10;color:Theme.accentSoft
                    ArtIcon {anchors.centerIn:parent;name:"brush";tint:Theme.accentText;width:20;height:20}
                }
                IconButton {
                    objectName:"sidebarExpandButton";anchors.fill:parent;visible:root.allowToggle&&root.compact
                    iconName:"panel-left";hint:Theme.t("Expand navigation");onClicked:root.toggleMode()
                }
            }
            Text {visible:!root.compact;Layout.fillWidth:true;text:"GestArt";color:Theme.text;font.pixelSize:16;font.weight:Font.DemiBold}
            IconButton {
                objectName:"sidebarCollapseButton";visible:root.allowToggle&&!root.compact
                Layout.preferredWidth:28;Layout.preferredHeight:32;iconName:"panel-left"
                hint:Theme.t("Collapse navigation");onClicked:root.toggleMode()
            }
        }
        ColumnLayout {
            Layout.fillWidth:true;spacing:4
            Repeater {
                model:[{page:"home",icon:"home",label:"Home"},{page:"library",icon:"images",label:"Collections"},{page:"practice",icon:"chart",label:"Practice"},{page:"history",icon:"history",label:"History"}]
                delegate:NavigationButton {
                    required property var modelData
                    label:Theme.t(modelData.label);iconName:modelData.icon;current:app.page===modelData.page
                    onClicked:app.page=modelData.page
                }
            }
        }
        NavigationButton {objectName:"sidebarSavedSets";label:Theme.t("Saved sets");iconName:"folder-open";onClicked:root.savedSets()}
        Button {
            id:sessionCard;objectName:"returnSessionButton"
            visible:root.hasReference||app.sessionImageCount>0
            Layout.fillWidth:true;Layout.topMargin:16
            implicitHeight:root.compact?64:Math.max(88,sessionContents.implicitHeight+24)
            padding:root.compact?8:12;hoverEnabled:true
            enabled:root.hasReference||(!app.scanning&&app.sessionImageCount>0)
            Accessible.name:(root.hasReference?Theme.t("Return to session"):Theme.t("Start session"))+". "+Theme.t("Selected for session")+": "+app.sessionImageCount
            onClicked:{if(root.hasReference)app.page="studio";else app.startSession()}
            HoverHandler {cursorShape:Qt.PointingHandCursor}
            background:Rectangle {
                radius:12;color:Theme.panel;border.width:sessionCard.visualFocus?2:1
                border.color:sessionCard.visualFocus?Theme.accent:Theme.border
                Rectangle {
                    anchors.fill:parent;radius:parent.radius;color:Theme.text
                    opacity:sessionCard.down?.10:sessionCard.hovered?.05:0
                    Behavior on opacity {NumberAnimation {duration:app.settings.reduce_motion?0:120}}
                }
            }
            contentItem:ColumnLayout {
                id:sessionContents;spacing:root.compact?4:12
                RowLayout {
                    Layout.fillWidth:true;spacing:8
                    Item {
                        Layout.preferredWidth:20;Layout.fillWidth:root.compact;implicitHeight:20
                        ArtIcon {anchors.centerIn:parent;name:"play";width:20;height:20;tint:Theme.accentText}
                    }
                    Text {
                        objectName:"sidebarSessionTitle";visible:!root.compact;Layout.fillWidth:true
                        text:root.hasReference?Theme.t("Return to session"):Theme.t("Start session")
                        color:Theme.text;font.pixelSize:13;font.weight:Font.Medium;wrapMode:Text.WordWrap
                    }
                }
                RowLayout {
                    Layout.fillWidth:true;spacing:8
                    Text {
                        visible:!root.compact;Layout.fillWidth:true;text:Theme.t("Selected for session")
                        color:Theme.muted;font.pixelSize:11;wrapMode:Text.WordWrap
                    }
                    Text {
                        Layout.fillWidth:root.compact;horizontalAlignment:Text.AlignHCenter
                        text:app.sessionImageCount;color:Theme.muted;font.pixelSize:12;font.weight:Font.Medium
                    }
                }
            }
            ToolTip.visible:hovered||visualFocus;ToolTip.text:Accessible.name;ToolTip.delay:650
        }
        Item {Layout.fillHeight:true;Layout.minimumHeight:16}
        Rectangle {Layout.fillWidth:true;implicitHeight:1;color:Theme.border;Layout.bottomMargin:8}
        NavigationButton {objectName:"sidebarSettingsButton";label:Theme.t("Settings");iconName:"settings-2";onClicked:root.settings()}

    }
    Rectangle {anchors.right:parent.right;width:1;height:parent.height;color:Theme.border}
}
