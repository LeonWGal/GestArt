import QtQuick
import QtQuick.Controls
Item {
    id:root
    implicitWidth:28;implicitHeight:28
    readonly property bool dragging:moveArea.pressed
    ArtIcon {anchors.centerIn:parent;name:"grip";width:18;height:18;tint:Theme.text}
    MouseArea {id:moveArea;anchors.fill:parent;hoverEnabled:true;cursorShape:Qt.SizeAllCursor;onPressed:app.beginWindowMove()}
    ToolTip.visible:moveArea.containsMouse;ToolTip.delay:500;ToolTip.text:Theme.t("Move window")+" · Alt"
    Accessible.role:Accessible.Button;Accessible.name:Theme.t("Move window")
}
