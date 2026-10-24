import QtQuick
import QtQuick.Controls
CheckBox {
    id:control
    implicitHeight:28;padding:0;spacing:8;hoverEnabled:true
    font.family:Theme.font;font.pixelSize:12
    indicator:Rectangle {width:18;height:18;x:control.mirrored?control.width-width:0;y:(control.height-height)/2;radius:5
        color:control.checked?Theme.accent:Theme.raised;border.width:control.visualFocus?2:1;border.color:control.visualFocus||control.checked?Theme.accent:control.hovered?Theme.muted:Theme.border
        ArtIcon {anchors.centerIn:parent;visible:control.checked;name:"check";width:14;height:14;tint:Theme.primaryText}
        opacity:control.enabled?1:.45
    }
    contentItem:Text {opacity:control.enabled?1:.45;text:control.text;font:control.font;color:Theme.muted;leftPadding:control.mirrored?0:26;rightPadding:control.mirrored?26:0;verticalAlignment:Text.AlignVCenter;wrapMode:Text.WordWrap}
}
