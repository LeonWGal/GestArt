import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id:root
    property string title:""
    property string description:""
    property string iconName:"images"
    property bool showIcon:true
    property string actionText:""
    signal actionRequested()
    spacing:12
    ArtIcon {visible:root.showIcon;Layout.alignment:Qt.AlignHCenter;name:root.iconName;width:32;height:32;tint:Theme.muted}
    Text {Layout.fillWidth:true;text:root.title;color:Theme.text;font.pixelSize:16;font.weight:Font.Medium;wrapMode:Text.WordWrap;horizontalAlignment:Text.AlignHCenter}
    Text {visible:text!=="";Layout.fillWidth:true;text:root.description;color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap;horizontalAlignment:Text.AlignHCenter}
    LobeButton {visible:root.actionText!=="";Layout.alignment:Qt.AlignHCenter;text:root.actionText;onClicked:root.actionRequested()}
}
