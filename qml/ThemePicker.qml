import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id:root
    required property var draft
    signal edited(string key,var value)
    readonly property string base:draft.theme_base||"black"
    readonly property string group:draft.theme_accent_group||"stones"
    readonly property var tones:app.themeCatalog[base]
    readonly property var accents:app.themeCatalog[group]
    readonly property var tone:tones.find(t=>t.key===draft["theme_"+base+"_tone"])||tones[0]
    readonly property var accent:accents.find(a=>a.key===draft.theme_accent)||accents[0]
    spacing:24
    ColumnLayout {Layout.fillWidth:true;spacing:12
        Text {text:Theme.t("Base color");color:Theme.text;font.pixelSize:14;font.weight:Font.Medium}
        RowLayout {Layout.fillWidth:true;spacing:8
            Repeater {model:[{key:"white",label:"White",icon:"sun",bg:"#fafafa",fg:"#222222"},{key:"black",label:"Black",icon:"moon",bg:"#191919",fg:"#eeeeee"}]
                delegate:Button {required property var modelData;Layout.fillWidth:true;Layout.preferredWidth:1;implicitHeight:root.width<300?48:64;hoverEnabled:true
                    Accessible.name:Theme.t(modelData.label);Accessible.checkable:true;Accessible.checked:root.base===modelData.key;onClicked:{root.edited("theme_base",modelData.key);root.edited("theme_follow_system",false)}
                    background:Rectangle {
                        Rectangle {anchors.fill:parent;anchors.margins:-3;radius:15;color:"transparent";border.width:2;border.color:Theme.accent;visible:parent.parent.visualFocus}
                        radius:12;color:modelData.bg;border.width:root.base===modelData.key?2:1;border.color:root.base===modelData.key?Theme.accent:Theme.border}
                    contentItem:RowLayout {spacing:8
                        ArtIcon {visible:root.width>=300;name:modelData.icon;tint:modelData.fg;Layout.leftMargin:12}
                        Text {text:Theme.t(modelData.label);color:modelData.fg;font.pixelSize:13;Layout.fillWidth:true;horizontalAlignment:Text.AlignHCenter}
                        ArtIcon {visible:root.base===modelData.key&&root.width>=300;name:"circle-check";tint:modelData.fg;Layout.rightMargin:12}
                    }
                }
            }
        }
        LobeSwitch {Layout.fillWidth:true;text:Theme.t("Follow system appearance");checked:root.draft.theme_follow_system||false;onToggled:root.edited("theme_follow_system",checked)}
    }
    ColumnLayout {Layout.fillWidth:true;spacing:12
        Text {text:Theme.t("Undertone");color:Theme.text;font.pixelSize:14;font.weight:Font.Medium}
        GridLayout {Layout.fillWidth:true;columns:root.width<300?2:5;columnSpacing:8;rowSpacing:8
            Repeater {model:root.tones
                delegate:Button {required property var modelData;Layout.fillWidth:true;Layout.preferredWidth:1;implicitHeight:72;padding:8;hoverEnabled:true
                    Accessible.name:Theme.t(modelData.label);Accessible.checkable:true;Accessible.checked:root.tone.key===modelData.key;onClicked:root.edited("theme_"+root.base+"_tone",modelData.key)
                    background:Rectangle {
                        Rectangle {anchors.fill:parent;anchors.margins:-3;radius:15;color:"transparent";border.width:2;border.color:Theme.accent;visible:parent.parent.visualFocus}
                        radius:12;color:modelData.colors.panel;border.width:root.tone.key===modelData.key?2:1;border.color:root.tone.key===modelData.key?Theme.accent:Theme.border}
                    contentItem:ColumnLayout {spacing:4
                        Rectangle {Layout.alignment:Qt.AlignHCenter;width:20;height:20;radius:10;color:modelData.colors.raised;border.color:modelData.colors.border
                            ArtIcon {visible:root.tone.key===modelData.key;anchors.centerIn:parent;width:14;height:14;name:"circle-check";tint:modelData.colors.text}
                        }
                        Text {text:Theme.t(modelData.label);color:modelData.colors.text;font.pixelSize:11;horizontalAlignment:Text.AlignHCenter;wrapMode:Text.WordWrap;Layout.fillWidth:true;Layout.fillHeight:true}
                    }
                }
            }
        }
    }
    ColumnLayout {Layout.fillWidth:true;spacing:12
        RowLayout {Layout.fillWidth:true
            Text {text:Theme.t("Accent color");color:Theme.text;font.pixelSize:14;font.weight:Font.Medium;Layout.fillWidth:true}
            Text {text:"12";color:Theme.muted;font.pixelSize:12}
        }
        RowLayout {Layout.fillWidth:true;spacing:8
            Repeater {model:[{key:"stones",label:"Gemstones"},{key:"lobe",label:"Lobe UI"}]
                delegate:LobeButton {required property var modelData;text:Theme.t(modelData.label);Layout.fillWidth:true;implicitWidth:1;choice:true;selected:root.group===modelData.key
                    onClicked:{root.edited("theme_accent_group",modelData.key);root.edited("theme_accent",modelData.key==="stones"?"charoite":"purple")}
                }
            }
        }
        GridLayout {Layout.fillWidth:true;columns:root.width<300?2:4;columnSpacing:8;rowSpacing:8
            Repeater {model:root.accents
                delegate:Button {required property var modelData;Layout.fillWidth:true;Layout.preferredWidth:1;implicitHeight:64;padding:8;hoverEnabled:true
                    Accessible.name:Theme.t(modelData.label)+" "+modelData.color;Accessible.checkable:true;Accessible.checked:root.accent.key===modelData.key;onClicked:root.edited("theme_accent",modelData.key)
                    background:Rectangle {
                        Rectangle {anchors.fill:parent;anchors.margins:-3;radius:15;color:"transparent";border.width:2;border.color:Theme.accent;visible:parent.parent.visualFocus}
                        radius:12;color:root.accent.key===modelData.key?Theme.raised:Theme.panel;border.width:root.accent.key===modelData.key?2:1;border.color:root.accent.key===modelData.key?Theme.accent:Theme.border}
                    contentItem:ColumnLayout {spacing:4
                        Rectangle {Layout.fillWidth:true;height:24;radius:6;color:modelData.color
                            ArtIcon {visible:root.accent.key===modelData.key;anchors.centerIn:parent;name:"circle-check";width:16;height:16;tint:modelData.ink}
                        }
                        Text {text:Theme.t(modelData.label);color:Theme.text;font.pixelSize:11;Layout.fillWidth:true;elide:Text.ElideRight;horizontalAlignment:Text.AlignHCenter}
                    }
                    ToolTip.visible:hovered;ToolTip.text:Theme.t(modelData.label)+" · "+modelData.color.toUpperCase()
                }
            }
        }
    }
    Rectangle {Layout.fillWidth:true;implicitHeight:96;radius:12;color:root.tone.colors.panel;border.color:root.tone.colors.border
        ColumnLayout {anchors.fill:parent;anchors.margins:16;spacing:8
            RowLayout {Layout.fillWidth:true
                Text {text:Theme.t(root.accent.label);color:root.tone.colors.text;font.pixelSize:14;font.weight:Font.Medium;Layout.fillWidth:true;elide:Text.ElideRight}
                Text {text:root.accent.color.toUpperCase();color:root.tone.colors.muted;font.pixelSize:11}
            }
            Rectangle {Layout.fillWidth:true;height:28;radius:8;color:root.accent[root.base]
                Text {anchors.centerIn:parent;text:Theme.t("Palette preview");font.pixelSize:12;color:root.accent[root.base+"Ink"]}
            }
        }
    }
}
