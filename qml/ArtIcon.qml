import QtQuick
Image {
    property string name: "image"
    property color tint: Theme.muted
    source: name === "" ? "" : "image://icons/" + name + "/" + tint.toString().replace("#", "")
    sourceSize: Qt.size(Math.max(1,Math.ceil(width * Math.max(1,Screen.devicePixelRatio))), Math.max(1,Math.ceil(height * Math.max(1,Screen.devicePixelRatio))))
    fillMode: Image.PreserveAspectFit
    width: 20; height: 20
    cache: true
}
