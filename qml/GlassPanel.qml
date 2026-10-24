import QtQuick
Item {
    id:root
    property real radius:12
    property Item scene:null
    property real blurRadius:12
    property real surfaceOpacity:app.settings.session_glass?(app.settings.session_panel_opacity||78)/100:.96
    property bool captureEnabled:true
    readonly property bool frosted:visible&&captureEnabled&&app.settings.session_glass&&app.settings.session_blur&&!app.softwareRenderer&&scene!==null&&width>0&&height>0
    readonly property color surfaceColor:Qt.rgba(Theme.panel.r,Theme.panel.g,Theme.panel.b,surfaceOpacity)
    readonly property real pixelRatio:Screen.devicePixelRatio
    readonly property real kernelRadius:Math.min(64,Math.ceil(blurRadius*Math.max(1,pixelRatio)))
    readonly property point origin:{
        // Chrome and scene share untransformed ancestors. Use their positions in
        // the result so anchors and window resizing invalidate this binding.
        let x=0,y=0,node=root;
        while(node){x+=node.x;y+=node.y;node=node.parent;}
        node=scene;
        while(node){x-=node.x;y-=node.y;node=node.parent;}
        return Qt.point(x,y)
    }
    readonly property rect sampleRect:{
        const padding=Math.max(16,root.blurRadius+2),dpr=Math.max(1,pixelRatio);
        const left=Math.max(0,Math.floor((origin.x-padding)*dpr)/dpr);
        const top=Math.max(0,Math.floor((origin.y-padding)*dpr)/dpr);
        const right=Math.min(scene?scene.width:1,Math.ceil((origin.x+width+padding)*dpr)/dpr);
        const bottom=Math.min(scene?scene.height:1,Math.ceil((origin.y+height+padding)*dpr)/dpr);
        return Qt.rect(left,top,Math.max(1,right-left),Math.max(1,bottom-top))
    }
    ShaderEffectSource {
        id:crop;objectName:"glassCrop";visible:false;smooth:true;live:root.frosted;hideSource:false;recursive:false
        sourceItem:root.frosted?root.scene:null
        sourceRect:root.sampleRect
        textureSize:Qt.size(Math.ceil(root.sampleRect.width*root.pixelRatio),Math.ceil(root.sampleRect.height*root.pixelRatio))
        width:root.sampleRect.width;height:root.sampleRect.height
    }
    ShaderEffect {
        id:horizontal;visible:false;width:root.sampleRect.width;height:root.sampleRect.height
        property var source:crop
        property vector2d sampleStep:Qt.vector2d(1/Math.max(1,crop.textureSize.width),0)
        property real kernelRadius:root.kernelRadius
        fragmentShader:"qrc:/shaders/blur.frag.qsb"
    }
    ShaderEffectSource {
        id:blurred;visible:false;smooth:true;live:root.frosted;hideSource:false;recursive:false
        sourceItem:root.frosted?horizontal:null
        textureSize:crop.textureSize;width:horizontal.width;height:horizontal.height
    }
    Rectangle {anchors.fill:parent;radius:root.radius;color:root.surfaceColor;visible:!root.frosted}
    ShaderEffect {
        objectName:"glassSurface";anchors.fill:parent;visible:root.frosted
        property var source:blurred
        property vector2d cropOffset:Qt.vector2d(root.origin.x-root.sampleRect.x,root.origin.y-root.sampleRect.y)
        property vector2d panelSize:Qt.vector2d(root.width,root.height)
        property vector2d cropSize:Qt.vector2d(root.sampleRect.width,root.sampleRect.height)
        property vector2d sampleStep:Qt.vector2d(0,1/Math.max(1,crop.textureSize.height))
        property real kernelRadius:root.kernelRadius
        property real cornerRadius:root.radius
        property color tint:root.surfaceColor
        fragmentShader:"qrc:/shaders/glass.frag.qsb"
    }
    Rectangle {anchors.fill:parent;radius:root.radius;color:"transparent";border.color:Qt.rgba(Theme.text.r,Theme.text.g,Theme.text.b,Theme.dark?.16:.12)}
}
