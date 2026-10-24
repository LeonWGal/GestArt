import QtQuick
import QtQuick.Layouts
Rectangle {
    id:root
    property string label:""
    property string value:"0"
    property string detail:""
    property string iconName:"timer"
    property bool dense:false
    implicitHeight:root.dense?96:root.width<220?112:128;radius:16;color:Theme.raised;border.color:Theme.border
    ColumnLayout {anchors.fill:parent;anchors.margins:16;spacing:8
        RowLayout {Layout.fillWidth:true;spacing:8
            ArtIcon {name:root.iconName;tint:Theme.accentText;width:18;height:18}
            Text {text:root.label;color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap}
        }
        Text {text:root.value;color:Theme.text;font.pixelSize:root.dense?24:root.width<220?24:30;font.weight:Font.DemiBold;Layout.fillWidth:true;elide:Text.ElideRight}
        Text {visible:text!==""&&!root.dense;text:root.detail;color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;elide:Text.ElideRight}
    }
}
