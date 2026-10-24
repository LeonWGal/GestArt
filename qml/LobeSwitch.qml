import QtQuick
import QtQuick.Controls
Switch {
    id: control
    padding:0;spacing:12;implicitHeight:Math.max(32,contentItem.implicitHeight+8)
    font.family:Theme.font;font.pixelSize:13;hoverEnabled:true
    indicator:Rectangle {
        x:control.mirrored?control.width-width:0;y:(control.height-height)/2;width:40;height:24;radius:12
        color:control.checked?Theme.accent:control.down?Theme.muted:Theme.hover
        border.color:control.activeFocus||control.checked?Theme.accent:Theme.border;border.width:control.activeFocus||control.hovered?2:1
        Rectangle {width:18;height:18;radius:9;y:3;x:control.checked!==control.mirrored?19:3
            color:control.checked?Theme.primaryText:Theme.muted
            Behavior on x {NumberAnimation {duration:app.settings.reduce_motion?0:120}}
        }
        opacity:control.enabled?1:.45
    }
    contentItem:Text {opacity:control.enabled?1:.45;text:control.text;font:control.font;color:Theme.text;leftPadding:control.mirrored?0:52;rightPadding:control.mirrored?52:0;verticalAlignment:Text.AlignVCenter;wrapMode:Text.WordWrap}
}
