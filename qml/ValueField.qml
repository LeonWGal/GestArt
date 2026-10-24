import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id:root
    property string label:""
    property int value:30
    property int minimum:1
    property int maximum:900
    property int sliderMaximum:maximum
    property bool unlimited:false
    property bool allowUnlimited:true
    signal edited(int amount,bool noLimit)
    spacing:4
    RowLayout {Layout.fillWidth:true;spacing:12
        Text {text:root.label;color:Theme.text;font.pixelSize:13;Layout.fillWidth:true;wrapMode:Text.WordWrap}
        Text {visible:root.unlimited;text:"∞";color:Theme.muted;font.pixelSize:20;Layout.preferredWidth:112;horizontalAlignment:Text.AlignHCenter}
        SpinBox {
            id:spin;Accessible.name:root.label
            from: root.minimum; to: root.maximum; value: root.value; editable: true; enabled: !root.unlimited
            visible:!root.unlimited;Layout.preferredWidth:112;Layout.fillWidth:false;implicitHeight:36;font.pixelSize:14
            palette.text:Theme.text;palette.base:Theme.raised;palette.button:Theme.raised;palette.buttonText:Theme.text
            background:Rectangle {radius:8;color:Theme.raised;border.color:spin.activeFocus?Theme.accent:Theme.border;opacity:spin.enabled?1:.45}
            up.indicator:Rectangle {implicitWidth:32;implicitHeight:40;height:spin.height;x:spin.mirrored?0:spin.width-width;radius:8;color:spin.up.pressed?Theme.hover:"transparent"
                Text {anchors.centerIn:parent;text:"+";color:Theme.muted;font.pixelSize:18}
            }
            down.indicator:Rectangle {implicitWidth:32;implicitHeight:40;height:spin.height;x:spin.mirrored?spin.width-width:0;radius:8;color:spin.down.pressed?Theme.hover:"transparent"
                Text {anchors.centerIn:parent;text:"−";color:Theme.muted;font.pixelSize:18}
            }
            onValueModified: root.edited(value, root.unlimited)
        }
    }
    Slider {
            id:slider;Accessible.name:root.label;visible:root.width>=400;Layout.fillWidth:true;from:root.minimum;to:root.sliderMaximum;value:Math.min(root.value,to);enabled:!root.unlimited;stepSize:1;onMoved:root.edited(Math.round(value),root.unlimited)
            implicitHeight:40;leftPadding:8;rightPadding:8
            background:Rectangle {x:slider.leftPadding;y:(slider.height-height)/2;width:slider.availableWidth;height:4;radius:2;color:Theme.hover
                Rectangle {width:slider.position*parent.width;height:4;radius:2;x:slider.mirrored?parent.width-width:0;color:Theme.accent;opacity:slider.enabled?1:.4}
            }
            handle:Rectangle {x:slider.leftPadding+slider.visualPosition*(slider.availableWidth-width);y:(slider.height-height)/2;width:16;height:16;radius:8;color:Theme.panel;border.width:slider.activeFocus?2:1;border.color:slider.pressed||slider.activeFocus?Theme.accent:Theme.muted;opacity:slider.enabled?1:.45}
        }
    LobeCheckBox {visible:root.allowUnlimited;Layout.alignment:Qt.AlignLeading;checked:root.unlimited;text:Theme.t("Unlimited");Accessible.name:root.label+" · "+Theme.t("Unlimited");onToggled:root.edited(root.value,checked)}
}
