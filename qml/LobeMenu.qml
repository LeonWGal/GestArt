import QtQuick
import QtQuick.Controls
Menu {
    id:control
    popupType:Popup.Item
    padding:8;margins:8
    height:Math.min(implicitHeight,Overlay.overlay?Overlay.overlay.height-16:480)
    contentItem:ListView {
        implicitHeight:contentHeight;model:control.contentModel;currentIndex:control.currentIndex
        clip:true;interactive:contentHeight>height;highlightMoveDuration:0
        ScrollBar.vertical:ScrollBar {policy:control.height<control.implicitHeight?ScrollBar.AlwaysOn:ScrollBar.AsNeeded}
    }
    background:Rectangle {radius:12;color:Theme.panel;border.color:Theme.border}
}
