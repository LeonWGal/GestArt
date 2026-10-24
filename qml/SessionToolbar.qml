import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlassPanel {
    id:root;objectName:"sessionToolbar"
    property real availableWidth:600
    property bool actualSize:false
    property int viewPercent:100
    signal fitRequested()
    signal resetRequested()
    signal deleteRequest()
    readonly property bool micro:availableWidth<324
    readonly property bool compact:availableWidth<604
    readonly property int buttonSize:micro?24:availableWidth<380?28:36
    readonly property bool singleRow:false
    readonly property bool doubleRow:true
    width:Math.min(availableWidth,420)
    height:groups.implicitHeight+16;radius:16
    ColumnLayout {
        id:groups;objectName:"sessionToolbarContent";anchors.fill:parent;anchors.margins:8;spacing:6
        RowLayout {id:playback;objectName:"sessionNavigationRow";Layout.fillWidth:true;spacing:4
            LobeButton {objectName:"sessionPreviousButton";Layout.fillWidth:true;Layout.preferredWidth:1;implicitHeight:36;quiet:true;iconName:"chevron-left";Accessible.name:Theme.t("Previous image")+" · ←";enabled:app.canPrevious;onClicked:app.previous()}
            LobeButton {objectName:"sessionPlayButton";Layout.fillWidth:true;Layout.preferredWidth:1;implicitHeight:36;quiet:true;iconName:app.paused||app.state==="review"||!app.active?"play":"pause";Accessible.name:Theme.t(app.paused?"Resume":app.state==="review"?"Return to session":app.active?"Pause":"Start session")+" · Space";enabled:(app.active||app.sessionImageCount>0)&&!app.scanning;onClicked:app.togglePause()}
            LobeButton {objectName:"sessionSkipButton";Layout.fillWidth:true;Layout.preferredWidth:1;implicitHeight:36;quiet:true;iconName:"skip-forward";Accessible.name:Theme.t("Skip")+" · S";enabled:app.active;onClicked:app.skip()}
            LobeButton {objectName:"sessionNextButton";Layout.fillWidth:true;Layout.preferredWidth:1;implicitHeight:36;quiet:true;iconName:"chevron-right";Accessible.name:Theme.t("Next image")+" · →";enabled:app.canNext;onClicked:app.next()}
        }
        RowLayout {id:tools;objectName:"sessionToolsRow";Layout.fillWidth:true;spacing:4
            IconButton {objectName:"sessionBwButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:"contrast";hint:Theme.t("B/W filter")+" · B";toggle:true;selected:app.bw;onClicked:app.setEffect("bw")}
            IconButton {objectName:"sessionFlipHButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:"flip-horizontal-2";hint:Theme.t("Flip horizontally")+" · H";toggle:true;selected:app.flipH;onClicked:app.setEffect("horizontal")}
            IconButton {objectName:"sessionFlipVButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:"flip-vertical-2";hint:Theme.t("Flip vertically")+" · V";toggle:true;selected:app.flipV;onClicked:app.setEffect("vertical")}
            IconButton {objectName:"sessionRotateButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:"rotate-90";hint:Theme.t("Rotate 90°")+" · R";onClicked:app.setEffect("rotate")}
            IconButton {objectName:"sessionGridButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:"grid-2x2";hint:Theme.t("Toggle grid")+" · G";toggle:true;selected:app.grid;onClicked:app.setEffect("grid")}
            IconButton {objectName:"sessionResetButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:"image-reset";hint:Theme.t("Reset image")+" · Backspace";onClicked:{app.resetEffects();root.resetRequested()}}
            IconButton {objectName:"sessionFitButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:root.actualSize?"fit-image":"scan";hint:root.actualSize?Theme.t("Fit")+" · Ctrl+0":"100% · Ctrl+1";emphasized:true;toggle:true;selected:root.actualSize;onClicked:root.fitRequested()}
            IconButton {objectName:"sessionCopyButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:"copy";hint:Theme.t("Copy image to clipboard")+" · Ctrl+C / C";onClicked:app.copyImage()}
            IconButton {objectName:"sessionOpenButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:"external-link";hint:Theme.t("Open file")+" · O";onClicked:app.openCurrentFile()}
            IconButton {objectName:"sessionDeleteButton";Layout.fillWidth:true;Layout.minimumWidth:24;Layout.preferredHeight:root.buttonSize;iconName:"trash-2";hint:Theme.t("Move to trash")+" · Delete";danger:true;onClicked:root.deleteRequest()}
        }
    }
}
