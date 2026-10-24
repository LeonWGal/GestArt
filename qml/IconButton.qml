import QtQuick
import QtQuick.Controls
Button {
    id:control
    property string iconName:"image"
    property string hint:""
    property bool toggle:false
    property bool selected:false
    property bool emphasized:false
    property bool danger:false
    implicitWidth:36;implicitHeight:36;padding:0
    hoverEnabled:true
    Accessible.name:hint
    Accessible.checkable:toggle||selected;Accessible.checked:selected
    ToolTip.visible:(hovered||visualFocus)&&hint!=="";ToolTip.text:hint;ToolTip.delay:650
    HoverHandler {enabled:control.enabled;cursorShape:Qt.PointingHandCursor}
    background:Rectangle {
        radius:8;color:control.selected?(control.emphasized?Theme.accent:Theme.accentSoft):"transparent"
        border.width:control.visualFocus?2:0;border.color:Theme.accent
        Rectangle {anchors.fill:parent;radius:parent.radius;color:Theme.text
            opacity:!control.enabled?0:control.down?.14:control.hovered?.08:0
            Behavior on opacity {NumberAnimation {duration:app.settings.reduce_motion?0:control.down?50:120;easing.type:Easing.OutCubic}}
        }
    }
    contentItem:Item {
        scale:control.down?.98:1
        Behavior on scale {NumberAnimation {duration:app.settings.reduce_motion?0:control.down?60:150;easing.type:Easing.OutCubic}}
        implicitWidth:18;implicitHeight:18;opacity:control.enabled?1:.35
        ArtIcon {anchors.centerIn:parent;name:control.iconName;width:18;height:18;tint:control.danger?Theme.danger:control.selected?(control.emphasized?Theme.primaryText:Theme.accentText):Theme.text}
    }
}
