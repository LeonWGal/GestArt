import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
    id:root;objectName:"libraryPage"
    signal onlineSources()
    signal downloader()
    signal savedSets()
    signal addFolder()
    signal addFiles()
    signal manageCollections()
    readonly property bool small:width<600
    readonly property bool wide:width>=900
    property bool completedOnly:false
    ColumnLayout {anchors.fill:parent;anchors.margins:root.small?16:24;spacing:12
        RowLayout {Layout.fillWidth:true;spacing:8
            Text {Layout.fillWidth:true;text:Theme.count(app.imageCount,"image","images_few","images");color:Theme.muted;font.pixelSize:13;elide:Text.ElideRight}
            IconButton {objectName:"libraryOnlineSources";width:32;height:32;iconName:"globe";hint:Theme.t("Online viewing");onClicked:root.onlineSources()}
            IconButton {objectName:"libraryDownloader";width:32;height:32;iconName:"download";hint:Theme.t("Download references");onClicked:root.downloader()}
            LobeButton {text:root.width<420?Theme.t("Add"):Theme.t("Add folder");iconName:"folder-plus";font.pixelSize:12;implicitHeight:32;onClicked:root.addFolder()}
            IconButton {id:more;width:32;height:32;iconName:"ellipsis";hint:Theme.t("Collection actions");onClicked:moreMenu.open()
                LobeMenu {id:moreMenu;y:more.height+4;x:more.width-width;width:220
                    LobeMenuItem {text:Theme.t("Online viewing");onTriggered:root.onlineSources()}
                    LobeMenuItem {text:Theme.t("Saved sets");onTriggered:root.savedSets()}
                    LobeMenuItem {text:Theme.t("Download references");onTriggered:root.downloader()}
                    LobeMenuItem {text:Theme.t("Select images");onTriggered:root.addFiles()}
                    LobeMenuItem {text:Theme.t("Manage collections");onTriggered:root.manageCollections()}
                    LobeMenuItem {visible:app.canUndoCollectionRemoval;text:Theme.t("Undo removal");onTriggered:app.undoCollectionRemoval()}
                }
            }
        }
        RowLayout {Layout.fillWidth:true;spacing:8
            LobeSearch {id:search;objectName:"librarySearch";Layout.fillWidth:true;placeholderText:Theme.t("Search references");onTextEdited:app.setSearch(text);onCleared:app.setSearch("")}
            IconButton {width:36;height:36;iconName:"circle-check";toggle:true;selected:root.completedOnly;hint:Theme.t("Completed studies");onClicked:{root.completedOnly=!root.completedOnly;app.setCompletedFilter(root.completedOnly)}}
        }
        RowLayout {visible:!root.wide;Layout.fillWidth:true;spacing:8
            LobeButton {text:Theme.t("All");implicitHeight:32;font.pixelSize:12;choice:true;selected:app.browseCollection==="";onClicked:app.browseCollection=""}
            ListView {Layout.fillWidth:true;Layout.preferredHeight:32;orientation:ListView.Horizontal;spacing:4;clip:true;model:app.collections
                delegate:LobeButton {required property var modelData;text:modelData.name;font.pixelSize:12;implicitHeight:32;choice:true;selected:app.browseCollection===modelData.id;onClicked:app.browseCollection=modelData.id}
                ScrollBar.horizontal:ScrollBar {policy:ScrollBar.AsNeeded}
            }
            IconButton {width:32;height:32;iconName:"settings-2";hint:Theme.t("Session folders");onClicked:root.manageCollections()}
        }
        RowLayout {Layout.fillWidth:true;Layout.fillHeight:true;spacing:16
            Rectangle {visible:root.wide;Layout.preferredWidth:240;Layout.fillHeight:true;radius:12;color:Theme.raised;border.color:Theme.border
                ColumnLayout {anchors.fill:parent;anchors.margins:12;spacing:12
                    LobeButton {Layout.fillWidth:true;text:Theme.t("All references");quiet:false;choice:true;selected:app.browseCollection==="";onClicked:app.browseCollection=""}
                    Text {Layout.fillWidth:true;text:Theme.t("Check folders to use in a session");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                    CollectionList {Layout.fillWidth:true;Layout.fillHeight:true;browseMode:true}
                    LobeButton {visible:app.canUndoCollectionRemoval;text:Theme.t("Undo removal");quiet:true;Layout.fillWidth:true;implicitHeight:32;onClicked:app.undoCollectionRemoval()}
                }
            }
            GridView {id:grid;objectName:"referenceGrid";Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:app.catalog
                cellWidth:width/Math.max(1,Math.floor(width/(root.small?144:190)));cellHeight:cellWidth*.8+40;cacheBuffer:240
                ScrollBar.vertical:ScrollBar {policy:ScrollBar.AsNeeded}
                delegate:Item {required property string mediaPath;required property string mediaName;required property string mediaSource;required property bool mediaCompleted
                    width:grid.cellWidth;height:grid.cellHeight
                    Rectangle {anchors.fill:parent;anchors.rightMargin:8;anchors.bottomMargin:8;radius:12;color:tile.hovered?Theme.hover:Theme.raised;border.width:activeFocus?2:1;border.color:activeFocus?Theme.accent:tile.hovered?Theme.muted:Theme.border
                        Image {id:thumbnail;anchors.fill:parent;anchors.margins:8;anchors.bottomMargin:36;source:mediaSource;asynchronous:true;cache:false;sourceSize:Qt.size(360,300);fillMode:Image.PreserveAspectFit}
                        Text {visible:thumbnail.status===Image.Error;anchors.centerIn:thumbnail;width:parent.width-24;text:Theme.t("Could not load image");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap;horizontalAlignment:Text.AlignHCenter}
                        Text {anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;anchors.margins:12;text:mediaName;color:Theme.text;font.pixelSize:12;elide:Text.ElideMiddle}
                        Rectangle {visible:mediaCompleted;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:12;width:24;height:24;radius:8;color:Theme.accent
                            ArtIcon {anchors.centerIn:parent;name:"circle-check";width:16;height:16;tint:Theme.primaryText}
                            ToolTip.visible:badgeHover.hovered;ToolTip.text:Theme.t("Study completed")
                            HoverHandler {id:badgeHover}
                        }
                        TapHandler {onTapped:app.openMedia(mediaPath)}
                        HoverHandler {id:tile;cursorShape:Qt.PointingHandCursor}
                        Accessible.role:Accessible.Button;Accessible.name:mediaName+(mediaCompleted?" · "+Theme.t("Study completed"):"");Accessible.onPressAction:app.openMedia(mediaPath)
                        activeFocusOnTab:true;Keys.onReturnPressed:app.openMedia(mediaPath);Keys.onSpacePressed:app.openMedia(mediaPath)
                    }
                }
                LobeEmpty {visible:grid.count===0;anchors.centerIn:parent;width:Math.max(80,Math.min(320,grid.width-24));showIcon:grid.height>=160;title:Theme.t(app.scanning?"Scanning":app.imageCount>0?"No matching references":"Add folders to build your collection");description:grid.height>=240?Theme.t("Try another search or reset filters."):"";actionText:grid.height>=180&&!app.scanning?Theme.t(app.imageCount>0?"Reset filters":"Add folder"):"";onActionRequested:{if(app.imageCount>0){search.clear();app.setSearch("");root.completedOnly=false;app.setCompletedFilter(false);app.browseCollection=""}else root.addFolder()}}
            }
        }
        RowLayout {Layout.fillWidth:true;spacing:8
            Text {Layout.fillWidth:true;text:Theme.t("Selected for session")+" · "+app.sessionImageCount;color:Theme.muted;font.pixelSize:12;elide:Text.ElideRight}
            LobeButton {text:Theme.t(app.active?"Return to session":"Start session");font.pixelSize:12;implicitHeight:32;primary:true;iconName:"play";enabled:app.sessionImageCount>0&&!app.scanning;onClicked:{if(app.active)app.page="studio";else app.startSession()}}
        }
    }
}
