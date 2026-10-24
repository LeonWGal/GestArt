import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
    id:root;contentWidth:availableWidth;clip:true
    signal openSite(string provider)
    signal practice(string provider)
    signal savedSets()
    readonly property var sites:[{id:"quickposes",name:"Quickposes",description:"Timed gesture practice"},{id:"proko",name:"Proko",description:"Reference packs and timer"},{id:"poselibrary",name:"Pose Library",description:"Pose collections"},{id:"sketchdaily",name:"SketchDaily",description:"Daily reference practice"}]
    ColumnLayout {
        width:Math.max(0,Math.min(960,root.availableWidth-32));x:(root.availableWidth-width)/2;spacing:16
        Item {height:8}
        Text {Layout.fillWidth:true;text:Theme.t("Online viewing");font.pixelSize:24;font.weight:Font.DemiBold;color:Theme.text;wrapMode:Text.WordWrap}
        Text {Layout.fillWidth:true;text:Theme.t("Browse websites online or download sets for offline practice with the GestArt timer.");font.pixelSize:13;color:Theme.muted;wrapMode:Text.WordWrap}
        LobeButton {Layout.fillWidth:true;text:Theme.t("Saved sets")+" · "+downloads.savedSets.length;iconName:"folder-open";onClicked:root.savedSets()}
        GridLayout {
            Layout.fillWidth:true;columns:root.availableWidth>=640?2:1;columnSpacing:16;rowSpacing:16
            Repeater {model:root.sites
                delegate:Rectangle {
                    required property var modelData
                    Layout.fillWidth:true;Layout.preferredWidth:1;implicitHeight:card.implicitHeight+24
                    radius:14;color:Theme.raised;border.color:Theme.border
                    ColumnLayout {
                        id:card;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:12;spacing:10
                        Image {
                            Layout.fillWidth:true;Layout.preferredHeight:width/1.65;source:"qrc:/assets/site-previews/"+modelData.id+".jpg"
                            fillMode:Image.PreserveAspectCrop;asynchronous:true;clip:true
                            Accessible.name:modelData.name+" · "+Theme.t("Preview")
                            MouseArea {anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:root.openSite(modelData.id)}
                        }
                        Text {Layout.fillWidth:true;text:modelData.name;font.pixelSize:18;font.weight:Font.DemiBold;color:Theme.text}
                        Text {Layout.fillWidth:true;text:Theme.t(modelData.description);font.pixelSize:12;color:Theme.muted;wrapMode:Text.WordWrap}
                        LobeButton {Layout.fillWidth:true;text:Theme.t("Open website");iconName:"globe";onClicked:root.openSite(modelData.id)}
                        LobeButton {Layout.fillWidth:true;text:Theme.t("Download references");iconName:"play";quiet:true;onClicked:root.practice(modelData.id)}
                    }
                }
            }
        }
        Item {height:8}
    }
}
