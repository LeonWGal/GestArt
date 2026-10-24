import QtQuick
import QtQuick.Controls
ComboBox {
    id:control
    implicitHeight:36;implicitWidth:160
    leftPadding:mirrored?36:12;rightPadding:mirrored?12:36
    font.family:Theme.font;font.pixelSize:13;hoverEnabled:true
    palette.text:Theme.text;palette.buttonText:Theme.text;palette.base:Theme.raised
    Accessible.name:displayText
    contentItem:Text {text:control.displayText;font:control.font;color:Theme.text;verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight}
    indicator:ArtIcon {name:"chevron-right";rotation:control.popup.visible?270:90;width:16;height:16;x:control.mirrored?12:control.width-width-12;y:(control.height-height)/2;tint:Theme.muted
        Behavior on rotation {NumberAnimation {duration:app.settings.reduce_motion?0:120;easing.type:Easing.OutCubic}}
    }
    background:Rectangle {radius:10;color:control.down?Theme.hover:Theme.raised;border.width:control.visualFocus?2:1;border.color:control.visualFocus?Theme.accent:control.hovered?Theme.muted:Theme.border}
    delegate:ItemDelegate {
        required property var modelData
        required property int index
        width:control.width-8;height:36;padding:8
        text:control.textRole&&typeof modelData==="object"?modelData[control.textRole]:String(modelData)
        highlighted:control.highlightedIndex===index
        contentItem:Text {text:parent.text;font:control.font;color:Theme.text;verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight}
        background:Rectangle {radius:8;color:parent.down?Theme.hover:parent.highlighted?Theme.accentSoft:"transparent"}
    }
    popup:Popup {
        y:control.height+4;width:control.width;padding:4;margins:8
        implicitHeight:Math.min(contentItem.implicitHeight+8,Math.max(96,(control.Window.window?control.Window.window.height:440)-80))
        contentItem:ListView {clip:true;implicitHeight:contentHeight;model:control.popup.visible?control.delegateModel:null;currentIndex:control.highlightedIndex;ScrollIndicator.vertical:ScrollIndicator {}}
        background:Rectangle {radius:12;color:Theme.panel;border.color:Theme.border}
        enter:Transition {NumberAnimation {property:"opacity";from:0;to:1;duration:app.settings.reduce_motion?0:100}}
        exit:Transition {NumberAnimation {property:"opacity";from:1;to:0;duration:app.settings.reduce_motion?0:80}}
    }
}
