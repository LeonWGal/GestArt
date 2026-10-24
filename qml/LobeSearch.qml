import QtQuick
import QtQuick.Controls
TextField {
    id:root
    readonly property bool rtl:i18n.rtl
    signal cleared()
    implicitHeight:36;font.family:Theme.font;font.pixelSize:13
    Accessible.name:placeholderText
    color:Theme.text;placeholderTextColor:Theme.subtle;selectByMouse:true
    leftPadding:root.rtl?32:36;rightPadding:root.rtl?36:32
    background:Rectangle {radius:10;color:Theme.raised;border.width:root.visualFocus?2:1;border.color:root.visualFocus?Theme.accent:Theme.border}
    ArtIcon {name:"search";width:16;height:16;x:root.rtl?root.width-width-12:12;anchors.verticalCenter:parent.verticalCenter;tint:Theme.muted}
    IconButton {visible:root.text!=="";x:root.rtl?2:root.width-width-2;anchors.verticalCenter:parent.verticalCenter;width:28;height:28;iconName:"x";hint:Theme.t("Clear search");onClicked:{root.clear();root.cleared();root.forceActiveFocus()}}
    Keys.onEscapePressed:{if(text!==""){clear();cleared()}}
}
