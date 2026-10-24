import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Button {
    id: control
    ToolTip.visible: (hovered || visualFocus) && (text === "" ? Accessible.name !== "" : buttonLabel.truncated)
    ToolTip.text: text === "" ? Accessible.name : text
    ToolTip.delay: 500
    property bool choice:false
    property bool selected:false
    Accessible.checkable:choice||selected;Accessible.checked:selected
    property bool primary: false
    property bool quiet: false
    property bool danger: false
    property string iconName: ""
    implicitHeight: 40
    implicitWidth: text === "" ? 40 : Math.max(88, contentItem.implicitWidth + 32)
    padding: 12; leftPadding: text === "" ? 11 : 16; rightPadding: text === "" ? 11 : 16
    font.family: Theme.font; font.pixelSize: 14; font.weight: Font.Medium
    hoverEnabled: true
    HoverHandler {enabled:control.enabled;cursorShape:Qt.PointingHandCursor}
    Accessible.name: text
    background: Rectangle {
        radius: 10
        color: control.primary ? Theme.primary : control.selected ? Theme.accentSoft : control.quiet ? "transparent" : Theme.raised
        border.width: control.visualFocus ? 2 : control.quiet || control.primary ? 0 : 1
        border.color: control.visualFocus || control.selected ? Theme.accent : Theme.border
        opacity: control.enabled ? 1 : 0.45
        Rectangle {anchors.fill:parent;radius:parent.radius;color:control.primary?(Theme.primaryText.r>.5?"#000000":"#ffffff"):Theme.text;opacity:!control.enabled?0:control.down?.12:control.hovered?.06:0
            Behavior on opacity {NumberAnimation {duration:app.settings.reduce_motion?0:control.down?50:120;easing.type:Easing.OutCubic}}
        }
    }
    contentItem: Item {
        scale:control.down?.98:1
        Behavior on scale {NumberAnimation {duration:app.settings.reduce_motion?0:control.down?60:150;easing.type:Easing.OutCubic}}
        implicitWidth:labelRow.implicitWidth;implicitHeight:labelRow.implicitHeight
        RowLayout {id:labelRow;anchors.centerIn:parent;spacing:8;width:Math.min(implicitWidth,parent.width)
            ArtIcon { visible: control.iconName !== ""; name: control.iconName; Layout.preferredWidth:18; Layout.preferredHeight:18; Layout.minimumWidth:18; tint: control.primary ? Theme.primaryText : control.danger ? Theme.danger : Theme.text;opacity:control.enabled?1:0.45 }
            Text {id:buttonLabel;visible:control.text!=="";Layout.fillWidth:true;Layout.minimumWidth:0;elide:Text.ElideRight; text: control.text; font: control.font; color: control.primary ? Theme.primaryText : control.danger ? Theme.danger : Theme.text; opacity: control.enabled ? 1 : 0.45 }
        }
    }
}
