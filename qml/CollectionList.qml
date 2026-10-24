import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
    id:root
    property bool browseMode:false
    ListView {
        id:list;anchors.fill:parent;clip:true;spacing:8;model:app.collections
        ScrollBar.vertical:ScrollBar {}
        delegate:Rectangle {
            required property var modelData
            width:list.width;height:root.height<100?56:72;radius:12;color:modelData.selected?Theme.raised:Theme.panel
            border.color:root.browseMode&&app.browseCollection===modelData.id?Theme.accent:Theme.border
            RowLayout {anchors.fill:parent;anchors.margins:root.height<100?6:8;spacing:8
                LobeCheckBox {id:selection;checked:modelData.selected;implicitWidth:28;implicitHeight:36
                    Accessible.name:Theme.t("Use in session")+" · "+modelData.name
                    onToggled:app.selectCollection(modelData.id,checked)
                    contentItem:Item {}
                }
                Button {Layout.fillWidth:true;implicitHeight:48;padding:0;hoverEnabled:true;enabled:root.browseMode
                    Accessible.name:modelData.name;onClicked:app.browseCollection=modelData.id
                    background:Rectangle {radius:8;color:parent.hovered?Theme.hover:"transparent";border.width:parent.visualFocus?2:0;border.color:Theme.accent}
                    contentItem:ColumnLayout {spacing:4
                    Text {text:modelData.name;color:Theme.text;font.pixelSize:13;font.weight:Font.Medium;Layout.fillWidth:true;elide:Text.ElideMiddle}
                    Text {text:Theme.count(modelData.count,"image","images_few","images")+(modelData.completed>0?" · ✓ "+modelData.completed:"");color:Theme.muted;font.pixelSize:11;Layout.fillWidth:true;elide:Text.ElideRight}
                }
                }
                IconButton {id:actions;width:28;height:32;iconName:"ellipsis";hint:Theme.t("Collection actions");onClicked:menu.open()
                    LobeMenu {id:menu;y:actions.height+8;x:actions.width-width;width:Math.min(260,root.width)
                        LobeMenuItem {text:Theme.t("Rename collection");onTriggered:{renameDialog.collectionId=modelData.id;renameField.text=modelData.name;renameDialog.open()}}
                        LobeMenuItem {text:Theme.t("Only this collection");onTriggered:{const id=modelData.id;app.selectAllCollections(false);app.selectCollection(id,true)}}
                        MenuSeparator {}
                        LobeMenuItem {text:Theme.t("Remove from app");onTriggered:app.removeCollection(modelData.id)}
                    }
                }
            }
            ToolTip.visible:hover.hovered;ToolTip.delay:800;ToolTip.text:modelData.path
            HoverHandler {id:hover}
        }
        Text {visible:list.count===0;anchors.centerIn:parent;width:parent.width-24;text:Theme.t("Add folders to build your collection");color:Theme.muted;font.pixelSize:13;wrapMode:Text.WordWrap;horizontalAlignment:Text.AlignHCenter}
    }
    Dialog {id:renameDialog;parent:Overlay.overlay;anchors.centerIn:parent;modal:true;width:Math.min(360,(Overlay.overlay?Overlay.overlay.width:root.width)-16);property string collectionId:""
        title:Theme.t("Rename collection")
        contentItem:TextField {id:renameField;Accessible.name:Theme.t("Collection name");selectByMouse:true;maximumLength:100;color:Theme.text;onAccepted:{if(text.trim()!==""){app.renameCollection(renameDialog.collectionId,text);renameDialog.close()}}}
        footer:RowLayout {spacing:8
            LobeButton {text:Theme.t("Cancel");Layout.fillWidth:true;onClicked:renameDialog.close()}
            LobeButton {text:Theme.t("Save");primary:true;Layout.fillWidth:true;enabled:renameField.text.trim()!=="";onClicked:{app.renameCollection(renameDialog.collectionId,renameField.text);renameDialog.close()}}
        }
    }
}
