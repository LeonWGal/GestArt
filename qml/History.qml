import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
    id:root;objectName:"historyPage"
    signal dataSettings()
    property string kind:"all"
    property string query:""
    readonly property var entries:app.history.filter(event=>{
        const matching=root.kind==="all"||event.kind===root.kind||(root.kind==="viewed"&&["shown","browsed","legacy"].includes(event.kind));
        return matching&&event.path.toLocaleLowerCase().includes(root.query.toLocaleLowerCase())
    })
    function kindLabel(event){return Theme.t(event.kind==="completed"?"Completed study":event.kind==="skipped"?"Skipped":event.kind==="legacy"?"Imported":"Viewed")}
    function eventDate(event){if(!event.at)return Theme.t("Undated");const date=new Date(event.at);return isNaN(date.getTime())?Theme.t("Undated"):date.toLocaleDateString(Qt.locale(i18n.language),"d MMM")+" · "+Qt.formatTime(date,"hh:mm")}
    ColumnLayout {anchors.fill:parent;anchors.margins:root.width<600?16:24;spacing:12
        RowLayout {Layout.fillWidth:true
            Text {Layout.fillWidth:true;text:root.entries.length+" / "+app.history.length+" · "+Theme.t("History");color:Theme.muted;font.pixelSize:13;elide:Text.ElideRight}
            IconButton {iconName:"download";hint:Theme.t("Import previous data");onClicked:app.importLegacy()}
            IconButton {iconName:"trash-2";danger:true;hint:Theme.t("Clear history");onClicked:clearDialog.open()}
        }
        LobeSearch {id:search;objectName:"historySearch";Layout.fillWidth:true;placeholderText:Theme.t("Search history");onTextChanged:root.query=text}
        LobeSegmented {objectName:"historyKind";Layout.fillWidth:true;accessibleLabel:Theme.t("History filter");value:root.kind
            options:[{value:"all",label:Theme.t("All")},{value:"completed",label:Theme.t("Studies")},{value:"viewed",label:Theme.t("Views")},{value:"skipped",label:Theme.t("Skips")}];onSelected:value=>root.kind=value
        }
        LobeNotice {visible:app.historyLoading||!app.historyHealthy;Layout.fillWidth:true;warning:!app.historyLoading;text:Theme.t(app.historyLoading?"Loading history":"Practice is available. History cannot be saved right now.");actionText:app.historyLoading?"":Theme.t("Data");onActionRequested:root.dataSettings()}
        ListView {id:list;objectName:"historyList";Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:root.entries;spacing:4
            ScrollBar.vertical:ScrollBar {policy:ScrollBar.AsNeeded}
            delegate:Button {required property var modelData
                width:list.width;height:64;padding:8;hoverEnabled:true
                Accessible.name:modelData.path.split(/[\\/]/).pop()+" · "+root.kindLabel(modelData)+" · "+root.eventDate(modelData)
                onClicked:app.openMedia(modelData.path)
                background:Rectangle {radius:10;color:parent.down?Theme.hover:parent.hovered?Theme.raised:"transparent";border.width:parent.visualFocus?2:0;border.color:Theme.accent}
                contentItem:RowLayout {spacing:12
                    Rectangle {width:32;height:32;radius:8;color:Theme.raised
                        ArtIcon {anchors.centerIn:parent;name:modelData.kind==="completed"?"circle-check":modelData.kind==="skipped"?"skip-forward":"image";tint:modelData.kind==="completed"?Theme.accentText:Theme.muted;width:18;height:18}
                    }
                    ColumnLayout {Layout.fillWidth:true;spacing:4
                        Text {Layout.fillWidth:true;text:modelData.path.split(/[\\/]/).pop();color:Theme.text;font.pixelSize:13;elide:Text.ElideMiddle}
                        Text {Layout.fillWidth:true;text:root.kindLabel(modelData)+(modelData.kind==="completed"?" · "+Theme.duration(modelData.duration):"")+" · "+root.eventDate(modelData);color:Theme.muted;font.pixelSize:11;elide:Text.ElideRight}
                    }
                    ArtIcon {name:"arrow-up-right";width:16;height:16;tint:Theme.muted}
                }
                ToolTip.visible:hovered;ToolTip.delay:900;ToolTip.text:modelData.path
            }
            LobeEmpty {visible:list.count===0;anchors.centerIn:parent;width:Math.min(320,parent.width-16);showIcon:list.height>=180;title:Theme.t(app.history.length?"No matching references":"Your history will appear here");description:Theme.t(app.history.length?"Try another search or reset filters.":"Viewed references and completed studies appear here.");actionText:list.height>=180?Theme.t(app.history.length?"Reset filters":"Home"):"";onActionRequested:{if(app.history.length){search.clear();root.kind="all"}else app.page="home"}}
        }
        Text {visible:app.history.length>=2000;Layout.fillWidth:true;text:Theme.t("Showing the latest 2000 events.");color:Theme.muted;font.pixelSize:11;wrapMode:Text.WordWrap}
    }
    Dialog {id:clearDialog;parent:Overlay.overlay;anchors.centerIn:parent;width:Math.min(400,Overlay.overlay.width-16);modal:true;title:Theme.t("Clear history");implicitHeight:200;height:Math.min(implicitHeight,Overlay.overlay.height-16)
        contentItem:ScrollView {clip:true;contentWidth:availableWidth;Text {width:clearDialog.availableWidth;text:Theme.t("Clear history for selected folders?");color:Theme.text;wrapMode:Text.WordWrap;font.pixelSize:13}}
        footer:RowLayout {spacing:8
            LobeButton {text:Theme.t("Cancel");onClicked:clearDialog.reject()}
            LobeButton {text:Theme.t("Clear history");iconName:"trash-2";danger:true;Layout.fillWidth:true;font.pixelSize:12;leftPadding:8;rightPadding:8;enabled:app.collections.some(c=>c.selected);onClicked:clearDialog.accept()}
        }
        onAccepted:app.clearHistory(false)
    }
}
