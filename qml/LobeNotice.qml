import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
    id:root
    property string text:""
    property bool warning:false
    property string actionText:""
    signal actionRequested()
    implicitHeight:body.implicitHeight+24;radius:10;color:Theme.raised;border.color:Theme.border
    Accessible.role:Accessible.AlertMessage;Accessible.name:text
    RowLayout {id:body;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:12;spacing:8
        ArtIcon {name:root.warning?"alert-triangle":"info";width:18;height:18;tint:root.warning?Theme.danger:Theme.muted;Layout.alignment:Qt.AlignTop}
        Text {Layout.fillWidth:true;text:root.text;color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
        IconButton {objectName:"noticeAction";visible:root.actionText!=="";width:28;height:28;iconName:"settings-2";hint:root.actionText;onClicked:root.actionRequested()}
    }
}
