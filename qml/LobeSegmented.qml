import QtQuick
import QtQuick.Controls
Rectangle {
    id:root
    property var options:[]
    property var value:null
    property string accessibleLabel:""
    signal selected(var value)
    implicitHeight:32;radius:10;color:Theme.canvas;border.color:Theme.border
    readonly property int selectedIndex:options.findIndex(o=>o.value===value)
    readonly property real slot:(width-8)/Math.max(1,options.length)
    function choose(index){const next=Math.max(0,Math.min(options.length-1,index));selected(options[next].value);choices.itemAt(next).forceActiveFocus()}
    Rectangle {visible:root.selectedIndex>=0;x:4+(root.LayoutMirroring.enabled?root.options.length-1-root.selectedIndex:root.selectedIndex)*root.slot;y:4;width:root.slot;height:root.height-8;radius:7;color:Theme.raised;border.color:Theme.border
        Behavior on x {NumberAnimation {duration:app.settings.reduce_motion?0:140;easing.type:Easing.OutCubic}}
    }
    Row {anchors.fill:parent;anchors.margins:4
        Repeater {id:choices;model:root.options
            delegate:Button {required property var modelData;required property int index
                objectName:modelData.objectName||"";width:root.slot;height:root.height-8;padding:0;hoverEnabled:true
                Accessible.role:Accessible.RadioButton;Accessible.name:root.accessibleLabel+" · "+modelData.label;Accessible.checkable:true;Accessible.checked:root.value===modelData.value
                onClicked:root.selected(modelData.value)
                Keys.onLeftPressed:root.choose(index+(mirrored?1:-1))
                Keys.onRightPressed:root.choose(index+(mirrored?-1:1))
                Keys.onPressed:event=>{
                    if(event.key===Qt.Key_Home){root.choose(0);event.accepted=true}
                    else if(event.key===Qt.Key_End){root.choose(root.options.length-1);event.accepted=true}
                }
                background:Rectangle {radius:7;color:parent.down?Theme.hover:parent.hovered&&root.value!==modelData.value?Theme.raised:"transparent";border.width:parent.visualFocus?2:0;border.color:Theme.accent}
                contentItem:Text {text:modelData.label;color:root.value===modelData.value?Theme.text:Theme.muted;font.family:Theme.font;font.pixelSize:12;font.weight:root.value===modelData.value?Font.Medium:Font.Normal;horizontalAlignment:Text.AlignHCenter;verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight}
                ToolTip.visible:hovered||visualFocus;ToolTip.delay:800;ToolTip.text:Accessible.name
            }
        }
    }
}
