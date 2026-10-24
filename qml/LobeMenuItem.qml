import QtQuick
import QtQuick.Controls
MenuItem {
    id:control
    implicitHeight:40
    font.family:Theme.font;font.pixelSize:13
    ToolTip.visible:hovered;ToolTip.delay:700;ToolTip.text:text
    background:Rectangle {radius:8;color:control.highlighted?Theme.raised:"transparent";border.width:control.activeFocus?1:0;border.color:Theme.accent}
    indicator:ArtIcon {x:8;y:(control.height-height)/2;width:18;height:18;visible:control.checkable&&control.checked;name:"circle-check";tint:Theme.accent}
    contentItem:Text {text:control.text;font:control.font;color:control.enabled?Theme.text:Theme.muted;leftPadding:control.checkable?32:8;rightPadding:8;verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight}
}
