import QtQuick
Item {
    id:root;objectName:"imageGrid";clip:true
    property int horizontalLines:2
    property int verticalLines:2
    property real imageScale:1
    property real pixelRatio:1
    readonly property real stroke:1/(Math.max(.1,imageScale)*Math.max(1,pixelRatio))
    Repeater {model:root.verticalLines
        delegate:Item {required property int index;objectName:"gridVertical-"+index
            x:(index+1)*root.width/(root.verticalLines+1)-root.stroke;width:2*root.stroke;height:root.height
            Rectangle {width:root.stroke;height:parent.height;color:"#000000"}
            Rectangle {x:root.stroke;width:root.stroke;height:parent.height;color:"#ffffff"}
        }
    }
    Repeater {model:root.horizontalLines
        delegate:Item {required property int index;objectName:"gridHorizontal-"+index
            y:(index+1)*root.height/(root.horizontalLines+1)-root.stroke;height:2*root.stroke;width:root.width
            Rectangle {width:parent.width;height:root.stroke;color:"#000000"}
            Rectangle {y:root.stroke;width:parent.width;height:root.stroke;color:"#ffffff"}
        }
    }
}
