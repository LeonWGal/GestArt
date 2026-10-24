import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Drawer {
    id:sheet;objectName:"onlineSourcesDrawer";edge:Qt.RightEdge;modal:true;dim:true
    property string section:"download"
    property string gender:"all"
    property string clothing:"all"
    property string pose:"all"
    property string angle:"all"
    property var selectedSets:[]
    property bool separate:true
    property bool wholePack:false
    property bool advanced:false
    property bool startAfter:false
    property bool wasRunning:false
    property string previewSet:""
    property var sessionSet:({})
    readonly property var sources:[{id:"quickposes",name:"Quickposes"},{id:"proko",name:"Proko"},{id:"poselibrary",name:"Pose Library"},{id:"sketchdaily",name:"SketchDaily"}]
    readonly property var chosen:online.sets.filter(s=>selectedSets.includes(s.id))
    readonly property int downloadCount:wholePack&&online.provider!=="sketchdaily"?0:Number(count.currentText)
    onChosenChanged:{if(chosen.length!==1)startAfter=false}
    signal startSession(string folder)
    function sourceName(id){const s=sources.find(s=>s.id===id);return s?s.name:id}
    function label(set){return set.translate?Theme.t(set.name):Theme.t(set.name||"")}
    function filters(){return {gender:gender,clothing:clothing,pose:pose,angle:angle}}
    function selection(){return chosen.map(s=>({id:s.id,name:s.name,imageCount:s.imageCount||0,preview:s.preview||""}))}
    function addSelection(){return downloads.add(online.provider,selection(),downloadCount,true,filters())}
    function beginDownload(){
        if(downloads.download(online.provider,selection(),downloadCount,filters(),startAfter&&chosen.length===1&&!app.active))selectedSets=[]
    }
    function showPreview(set){previewSet=set.id;online.requestPreview(set.id);previewPopup.open()}
    function prepareSession(set){sessionSet=set;sessionPopup.open()}
    function ensureSource(){
        if(section==="saved"){downloads.refreshSavedSets();return}
        if(online.provider==="")online.selectProvider("quickposes")
        else if(selectedSets.length===0&&!online.busy&&online.sets.length>0)selectedSets=[online.sets[0].id]
    }
    function filterEnabled(key){
        if(online.provider!=="quickposes")return true
        return chosen.some(s=>{const type=s.id.split("_")[0];return key==="gender"?["pose","feet","hands","face"].includes(type):["pose","feet"].includes(type)})
    }
    onOpened:ensureSource()
    onSectionChanged:{if(opened)ensureSource()}
    Connections {target:online;function onChanged(){
        sheet.selectedSets=sheet.selectedSets.filter(id=>online.sets.some(s=>s.id===id))
        if(!online.busy&&online.sets.length>0&&sheet.selectedSets.length===0&&!downloads.running)sheet.selectedSets=[online.sets[0].id]
        if(online.provider==="sketchdaily")sheet.wholePack=false
    }}
    Connections {target:downloads;function onChanged(){
        if(sheet.wasRunning&&!downloads.running&&downloads.savedSets.length>0)sheet.section="saved"
        sheet.wasRunning=downloads.running
    }}
    background:Rectangle {color:Theme.panel;border.color:Theme.border}
    FolderDialog {id:saveFolder;title:Theme.t("Save folder");currentFolder:downloads.destinationUrl;onAccepted:downloads.setDestination(selectedFolder)}
    Popup {
        id:previewPopup;parent:Overlay.overlay;anchors.centerIn:parent;modal:true;padding:12
        width:Math.min(700,(sheet.Window.window?sheet.Window.window.width:360)-24)
        height:Math.min(680,(sheet.Window.window?sheet.Window.window.height:440)-24)
        background:Rectangle {color:Theme.panel;radius:12;border.color:Theme.border}
        contentItem:ColumnLayout {
            RowLayout {Layout.fillWidth:true
                Text {Layout.fillWidth:true;text:Theme.t("Set preview");color:Theme.text;font.pixelSize:16}
                IconButton {iconName:"x";hint:Theme.t("Close");onClicked:previewPopup.close()}
            }
            Image {objectName:"onlinePreviewImage";Layout.fillWidth:true;Layout.fillHeight:true;source:online.previewImage;fillMode:Image.PreserveAspectFit;asynchronous:true}
            BusyIndicator {Layout.alignment:Qt.AlignHCenter;visible:online.previewBusy;running:visible}
            LobeNotice {Layout.fillWidth:true;visible:online.previewError!=="";text:Theme.t(online.previewError);actionText:Theme.t("Retry");onActionRequested:online.requestPreview(sheet.previewSet)}
        }
    }
    Popup {
        id:sessionPopup;objectName:"savedSessionSetup";parent:Overlay.overlay;anchors.centerIn:parent;modal:true;padding:16
        width:Math.min(440,(sheet.Window.window?sheet.Window.window.width:360)-24)
        height:Math.min(520,(sheet.Window.window?sheet.Window.window.height:440)-24)
        background:Rectangle {color:Theme.panel;radius:14;border.color:Theme.border}
        contentItem:ColumnLayout {spacing:12
            RowLayout {Layout.fillWidth:true
                Text {Layout.fillWidth:true;text:Theme.t("Session from saved set");color:Theme.text;font.pixelSize:18;font.weight:Font.DemiBold;wrapMode:Text.WordWrap}
                IconButton {iconName:"x";hint:Theme.t("Cancel");onClicked:sessionPopup.close()}
            }
            Text {Layout.fillWidth:true;text:sheet.label(sheet.sessionSet);color:Theme.text;font.pixelSize:14;wrapMode:Text.WordWrap}
            Text {Layout.fillWidth:true;text:Theme.t("Only this saved set will be used. No download is needed.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
            ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;clip:true;contentWidth:availableWidth
                SessionSetup {width:parent.width;namePrefix:"saved";showFolders:false;compact:true}
            }
            LobeButton {objectName:"startSavedSession";Layout.fillWidth:true;primary:true;text:Theme.t("Start session");iconName:"play";enabled:!app.active&&!app.scanning&&!!sheet.sessionSet.available
                onClicked:{const folder=sheet.sessionSet.folder;sessionPopup.close();sheet.close();sheet.startSession(folder)}
            }
        }
    }
    ColumnLayout {
        anchors.fill:parent;anchors.margins:16;spacing:12
        RowLayout {Layout.fillWidth:true
            Text {Layout.fillWidth:true;text:Theme.t("Online references");color:Theme.text;font.pixelSize:20;font.weight:Font.DemiBold;wrapMode:Text.WordWrap}
            IconButton {iconName:"x";hint:Theme.t("Close");onClicked:sheet.close()}
        }
        LobeSegmented {objectName:"downloadSections";Layout.fillWidth:true;implicitHeight:40;accessibleLabel:Theme.t("Online references");value:sheet.section
            options:[{value:"download",label:Theme.t("Download from website")},{value:"saved",label:Theme.t("Saved sets")+" · "+downloads.savedSets.length}]
            onSelected:value=>sheet.section=value
        }
        ScrollView {
            id:sourceScroll;visible:sheet.section==="download";Layout.fillWidth:true;Layout.fillHeight:true;clip:true;contentWidth:availableWidth
            ColumnLayout {width:sourceScroll.availableWidth;spacing:12
                Text {Layout.fillWidth:true;text:Theme.t("1. Choose a website");color:Theme.text;font.pixelSize:14;font.weight:Font.Medium}
                RowLayout {Layout.fillWidth:true
                    LobeComboBox {objectName:"downloadSourcePicker";Layout.fillWidth:true;model:sheet.sources.map(s=>s.name);currentIndex:Math.max(0,sheet.sources.findIndex(s=>s.id===online.provider));enabled:!online.busy&&!downloads.running;Accessible.name:Theme.t("Websites")
                        onActivated:{sheet.selectedSets=[];sheet.gender="all";sheet.clothing="all";sheet.pose="all";sheet.angle="all";online.selectProvider(sheet.sources[currentIndex].id)}
                    }
                    IconButton {iconName:online.busy?"x":"rotate-cw";hint:Theme.t(online.busy?"Cancel":"Reload");enabled:!downloads.running;onClicked:{if(online.busy)online.cancel();else online.selectProvider(online.provider,true)}}
                }
                BusyIndicator {visible:online.busy;running:visible;Layout.alignment:Qt.AlignHCenter}
                LobeNotice {visible:online.error!=="";Layout.fillWidth:true;text:Theme.t(online.error)}
                RowLayout {Layout.fillWidth:true
                    Text {Layout.fillWidth:true;text:Theme.t("2. Choose sets");color:Theme.text;font.pixelSize:14;font.weight:Font.Medium}
                    LobeButton {text:Theme.t("Select all");quiet:true;implicitHeight:28;font.pixelSize:11;enabled:!online.busy&&!downloads.running;onClicked:sheet.selectedSets=online.sets.map(s=>s.id)}
                }
                LobeSearch {id:packSearch;visible:online.sets.length>7;Layout.fillWidth:true;placeholderText:Theme.t("Search sets")}
                ListView {
                    objectName:"onlineSetPicker";Layout.fillWidth:true;Layout.preferredHeight:Math.min(280,Math.max(76,online.sets.length*82));clip:true;spacing:6
                    model:online.sets.filter(s=>sheet.label(s).toLowerCase().includes(packSearch.text.toLowerCase()))
                    ScrollBar.vertical:ScrollBar {policy:ScrollBar.AsNeeded}
                    delegate:Rectangle {
                        required property var modelData
                        width:ListView.view.width;height:76;radius:10;color:sheet.selectedSets.includes(modelData.id)?Theme.accentSoft:Theme.raised;border.color:sheet.selectedSets.includes(modelData.id)?Theme.accent:Theme.border
                        RowLayout {anchors.fill:parent;anchors.margins:8;spacing:10
                            Rectangle {Layout.preferredWidth:74;Layout.preferredHeight:58;radius:6;color:Theme.canvas;clip:true
                                Image {anchors.fill:parent;source:modelData.preview||"";asynchronous:true;fillMode:Image.PreserveAspectFit;sourceSize.width:148}
                                ArtIcon {anchors.centerIn:parent;visible:!modelData.preview;name:"images";tint:Theme.muted;width:24;height:24}
                                MouseArea {anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sheet.showPreview(modelData)}
                            }
                            LobeCheckBox {Layout.fillWidth:true;Layout.minimumWidth:0;enabled:!downloads.running;text:sheet.label(modelData)+(modelData.imageCount?" · "+modelData.imageCount:"");checked:sheet.selectedSets.includes(modelData.id)
                                onToggled:{const ids=sheet.selectedSets.filter(id=>id!==modelData.id);if(checked)ids.push(modelData.id);sheet.selectedSets=ids}
                            }
                            IconButton {iconName:"scan-eye";hint:Theme.t("Set preview");width:30;height:30;enabled:!online.busy;onClicked:sheet.showPreview(modelData)}
                        }
                    }
                }
                Text {visible:sheet.chosen.length===0&&!online.busy;Layout.fillWidth:true;text:Theme.t("Select at least one set to continue.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                Text {Layout.fillWidth:true;text:Theme.t("3. Download options");color:Theme.text;font.pixelSize:14;font.weight:Font.Medium}
                LobeCheckBox {objectName:"downloadWholePack";Layout.fillWidth:true;text:Theme.t("Download entire pack");checked:sheet.wholePack;enabled:!downloads.running&&online.provider!=="sketchdaily";onToggled:sheet.wholePack=checked}
                Text {visible:online.provider==="sketchdaily";Layout.fillWidth:true;text:Theme.t("SketchDaily supplies random references. Choose how many to save.");color:Theme.muted;font.pixelSize:11;wrapMode:Text.WordWrap}
                RowLayout {visible:!sheet.wholePack;Layout.fillWidth:true
                    Text {Layout.fillWidth:true;text:Theme.t("Images to save per set");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                    LobeComboBox {id:count;objectName:"onlineCountPicker";Layout.preferredWidth:90;model:["5","10","20","30","50","100"];currentIndex:2;enabled:!downloads.running;Accessible.name:Theme.t("Images to save per set")}
                }
                Text {visible:sheet.wholePack;Layout.fillWidth:true;text:Theme.t("Save every accessible image in each selected pack. Filters still apply.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                LobeButton {Layout.fillWidth:true;text:Theme.t("Filters and save folder");iconName:"sliders-horizontal";quiet:true;onClicked:sheet.advanced=!sheet.advanced}
                ColumnLayout {visible:sheet.advanced;Layout.fillWidth:true;spacing:10
                    Text {visible:online.provider==="proko"||online.provider==="poselibrary";Layout.fillWidth:true;text:Theme.t("This source filters by collection. Choose the appropriate sets above.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                    GridLayout {Layout.fillWidth:true;columns:2;columnSpacing:8;rowSpacing:8
                        Repeater {model:online.provider==="quickposes"?[{key:"gender",label:"Gender",values:["all","male","female"],labels:["All","Male","Female"]},{key:"clothing",label:"Clothing",values:["all","clothed","nude"],labels:["All","Clothed","Nude or partially nude"]}]:online.provider==="sketchdaily"?[{key:"gender",label:"Gender",values:["all","male","female"],labels:["All","Male","Female"]},{key:"clothing",label:"Clothing",values:["all","clothed","nude"],labels:["All","Clothed","Nude"]},{key:"pose",label:"Pose type",values:["all","action","stationary"],labels:["All","Action","Stationary"]},{key:"angle",label:"View angle",values:["all","front","side","back","above"],labels:["All","Front","Side","Back view","Above or below"]}]:[]
                            delegate:ColumnLayout {required property var modelData;Layout.fillWidth:true;Layout.preferredWidth:1;Layout.minimumWidth:0;spacing:4
                                Text {text:Theme.t(modelData.label);color:Theme.muted;font.pixelSize:11}
                                LobeComboBox {Layout.fillWidth:true;model:modelData.labels.map(v=>Theme.t(v));currentIndex:Math.max(0,modelData.values.indexOf(sheet[modelData.key]));enabled:!downloads.running&&sheet.filterEnabled(modelData.key);Accessible.name:Theme.t(modelData.label);onActivated:sheet[modelData.key]=modelData.values[currentIndex]}
                            }
                        }
                    }
                    LobeButton {Layout.fillWidth:true;text:Theme.t("Reset filters");quiet:true;enabled:!downloads.running;onClicked:{sheet.gender="all";sheet.clothing="all";sheet.pose="all";sheet.angle="all"}}
                    Text {Layout.fillWidth:true;text:Theme.t("Each set is saved in its own folder.");color:Theme.muted;font.pixelSize:11;wrapMode:Text.WordWrap}
                    Text {Layout.fillWidth:true;text:downloads.destination;color:Theme.muted;font.pixelSize:11;wrapMode:Text.WrapAnywhere}
                    LobeButton {objectName:"downloadDestination";Layout.fillWidth:true;text:Theme.t("Change save folder");iconName:"folder-open";enabled:!downloads.running;onClicked:saveFolder.open()}
                }
                LobeCheckBox {Layout.fillWidth:true;implicitHeight:Math.max(32,contentItem.implicitHeight+8);text:Theme.t("Start a session after downloading");checked:sheet.startAfter;enabled:!downloads.running&&!app.active&&sheet.chosen.length===1;onToggled:sheet.startAfter=checked}
                Text {Layout.fillWidth:true;text:Theme.t("Downloads appear in Saved sets. Start any saved set there, even after restarting GestArt.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                Repeater {model:downloads.jobs
                    delegate:Rectangle {required property var modelData;required property int index;Layout.fillWidth:true;implicitHeight:jobBody.implicitHeight+20;radius:10;color:Theme.raised;border.color:Theme.border
                        ColumnLayout {id:jobBody;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:10;spacing:6
                            Text {Layout.fillWidth:true;text:sheet.label(modelData);color:Theme.text;font.pixelSize:13;wrapMode:Text.WordWrap}
                            Text {Layout.fillWidth:true;text:Theme.t(modelData.status)+" · "+modelData.completed+(modelData.target>0?" / "+modelData.target:"");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                            ProgressBar {visible:downloads.running;Layout.fillWidth:true;from:0;to:Math.max(1,modelData.target);value:modelData.completed;indeterminate:modelData.target<=0}
                            Text {visible:!!modelData.error;Layout.fillWidth:true;text:Theme.t(modelData.error||"");color:Theme.muted;font.pixelSize:11;wrapMode:Text.WordWrap}
                            LobeButton {visible:!downloads.running&&modelData.status!=="Completed";text:Theme.t("Retry download");iconName:"rotate-cw";onClicked:downloads.retry(index)}
                        }
                    }
                }
                LobeNotice {visible:downloads.error!=="";Layout.fillWidth:true;text:Theme.t(downloads.error)}
            }
        }
        ScrollView {
            id:savedScroll;visible:sheet.section==="saved";Layout.fillWidth:true;Layout.fillHeight:true;clip:true;contentWidth:availableWidth
            ColumnLayout {width:savedScroll.availableWidth;spacing:12
                RowLayout {Layout.fillWidth:true
                    Text {Layout.fillWidth:true;text:Theme.t("Saved on this computer");color:Theme.text;font.pixelSize:15;font.weight:Font.Medium;wrapMode:Text.WordWrap}
                    IconButton {iconName:"rotate-cw";hint:Theme.t("Reload");enabled:!downloads.running;onClicked:downloads.refreshSavedSets()}
                }
                Text {Layout.fillWidth:true;text:Theme.t("Choose a set, adjust the timer and start. Other collections will not be included.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                LobeNotice {visible:app.active;Layout.fillWidth:true;text:Theme.t("Finish the current session before starting another set.")}
                LobeEmpty {visible:downloads.savedSets.length===0;Layout.fillWidth:true;title:Theme.t("No saved sets yet");description:Theme.t("Download a set from a website. It will stay here for offline practice.")}
                LobeButton {visible:downloads.savedSets.length===0;Layout.fillWidth:true;text:Theme.t("Download from website");primary:true;iconName:"download";onClicked:sheet.section="download"}
                Repeater {model:downloads.savedSets
                    delegate:Rectangle {required property var modelData;required property int index;Layout.fillWidth:true;implicitHeight:savedBody.implicitHeight+24;radius:12;color:Theme.raised;border.color:Theme.border
                        ColumnLayout {id:savedBody;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:12;spacing:10
                            RowLayout {Layout.fillWidth:true;spacing:12
                                Image {Layout.preferredWidth:80;Layout.preferredHeight:70;source:modelData.available?modelData.preview||"":"";asynchronous:true;fillMode:Image.PreserveAspectFit}
                                ColumnLayout {Layout.fillWidth:true;spacing:4
                                    Text {Layout.fillWidth:true;text:sheet.label(modelData);color:Theme.text;font.pixelSize:14;font.weight:Font.Medium;wrapMode:Text.WordWrap}
                                    Text {Layout.fillWidth:true;text:sheet.sourceName(modelData.provider)+" · "+Theme.count(modelData.count,"image","images_few","images");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                                    Text {Layout.fillWidth:true;text:Theme.t(!modelData.available?"Folder not found":modelData.partial?"Partially downloaded":modelData.fullPack?"Entire pack saved":"Saved selection");color:Theme.muted;font.pixelSize:11;wrapMode:Text.WordWrap}
                                }
                            }
                            LobeButton {visible:modelData.partial&&!!modelData.setId;Layout.fillWidth:true;text:Theme.t("Retry download");iconName:"rotate-cw";enabled:!downloads.running;onClicked:{if(downloads.resumeSaved(index))sheet.section="download"}}
                            RowLayout {Layout.fillWidth:true
                                LobeButton {objectName:"savedSetSession_"+index;Layout.fillWidth:true;primary:true;text:Theme.t("Set up session");iconName:"play";enabled:modelData.available&&!app.active&&!app.scanning;onClicked:sheet.prepareSession(modelData)}
                                IconButton {iconName:"folder-open";hint:Theme.t("Open folder");enabled:modelData.available;onClicked:downloads.openSavedFolder(index)}
                            }
                        }
                    }
                }
            }
        }
        ColumnLayout {visible:sheet.section==="download"||downloads.running;Layout.fillWidth:true;spacing:8
            Text {visible:downloads.running;Layout.fillWidth:true;text:Theme.t(downloads.jobs.some(j=>j.status==="Loading sources")?"Loading sources":"Downloading images")+" · "+downloads.completed+(downloads.requested>0?" / "+downloads.requested:"");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
            ProgressBar {visible:downloads.running;Layout.fillWidth:true;from:0;to:Math.max(1,downloads.requested);value:downloads.completed;indeterminate:downloads.requested<=0||downloads.jobs.some(j=>j.status==="Loading sources")}
            Text {visible:sheet.section==="download"&&!downloads.running;Layout.fillWidth:true;text:sheet.chosen.length+" · "+Theme.t("sets selected")+" · "+(sheet.wholePack?Theme.t("Entire pack"):count.currentText+" "+Theme.t("images per set"));color:Theme.muted;font.pixelSize:11;wrapMode:Text.WordWrap}
            LobeButton {objectName:"onlineImportButton";Layout.fillWidth:true;primary:!downloads.running
                text:downloads.running?Theme.t("Stop download"):sheet.startAfter&&sheet.chosen.length===1&&!app.active?Theme.t("Download and start session"):sheet.wholePack?Theme.t("Download entire pack"):Theme.t("Download selected sets")
                iconName:downloads.running?"x":"download";enabled:downloads.running||(!online.busy&&sheet.chosen.length>0)
                onClicked:{if(downloads.running)downloads.cancel();else sheet.beginDownload()}
            }
        }
    }
}
