import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Drawer {
    id:sheet;objectName:"settingsDrawer";edge:Qt.RightEdge;modal:true;dim:true
    property var draft:({})
    property int section:0
    readonly property bool compact:width<440||height<360
    readonly property int inset:compact?12:24
    readonly property var sections:["Session","Appearance","Window","Goal","Data"]
    function edit(key,value) {let next=Object.assign({},draft);next[key]=value;draft=next}
    onAboutToShow:draft=Object.assign({},app.settings)
    onSectionChanged:{if(scroll.contentItem)scroll.contentItem.contentY=0}
    enter:Transition {NumberAnimation {property:"position";from:0;to:1;duration:app.settings.reduce_motion?0:140;easing.type:Easing.OutCubic}}
    exit:Transition {NumberAnimation {property:"position";from:1;to:0;duration:app.settings.reduce_motion?0:100;easing.type:Easing.OutCubic}}
    background:Rectangle {color:Theme.panel;border.color:Theme.border}
    ColumnLayout {anchors.fill:parent;spacing:0
        RowLayout {Layout.fillWidth:true;Layout.leftMargin:sheet.inset;Layout.rightMargin:sheet.inset;Layout.topMargin:sheet.compact?4:16;Layout.bottomMargin:sheet.compact?4:12
            Text {text:Theme.t("Settings");color:Theme.text;font.pixelSize:sheet.compact?16:24;font.weight:Font.DemiBold;Layout.fillWidth:true}
            IconButton {iconName:"x";width:sheet.compact?28:36;height:width;hint:Theme.t("Close");onClicked:sheet.close()}
        }
        LobeComboBox {objectName:"settingsCategory";visible:sheet.width<420||sheet.height<400;Layout.fillWidth:true;Layout.leftMargin:sheet.inset;Layout.rightMargin:sheet.inset;Layout.bottomMargin:8;implicitHeight:32
            Accessible.name:Theme.t("Settings category");model:sheet.sections.map(s=>Theme.t(s));currentIndex:sheet.section;onActivated:sheet.section=currentIndex
        }
        Flow {visible:sheet.width>=420&&sheet.height>=400;Layout.fillWidth:true;Layout.leftMargin:sheet.inset;Layout.rightMargin:sheet.inset;Layout.bottomMargin:16;spacing:8
            Repeater {model:sheet.sections
                delegate:LobeButton {required property string modelData;required property int index;text:Theme.t(modelData);font.pixelSize:12;leftPadding:8;rightPadding:8;width:(parent.width-16)/3;implicitHeight:32;choice:true;selected:sheet.section===index;quiet:sheet.section!==index;onClicked:sheet.section=index}
            }
        }
        ScrollView {id:scroll;objectName:"settingsScroll";Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
            ColumnLayout {width:sheet.width-2*sheet.inset;x:sheet.inset;spacing:24
                ColumnLayout {visible:sheet.section===0;Layout.fillWidth:true;spacing:sheet.compact?12:24
                    Text {visible:!sheet.compact;text:Theme.t("Session rhythm");color:Theme.text;font.pixelSize:18;font.weight:Font.Medium}
                    Text {visible:!sheet.compact;Layout.fillWidth:true;text:Theme.t("Next session")+" · "+Theme.sessionSummary(sheet.draft);color:Theme.accentText;font.pixelSize:13;wrapMode:Text.WordWrap}
                    LobeSegmented {Layout.fillWidth:true;accessibleLabel:Theme.t("Time per image");value:sheet.draft.unlimited_time?null:sheet.draft.display_time
                        options:[30,60,120,300].map(n=>({value:n,label:n<60?n+" "+Theme.t("sec"):n/60+" "+Theme.t("min")}));onSelected:n=>{sheet.edit("display_time",n);sheet.edit("unlimited_time",false)}
                    }
                    ValueField {Layout.fillWidth:true;label:Theme.t("Time per image");value:sheet.draft.display_time||30;maximum:86400;sliderMaximum:600;unlimited:sheet.draft.unlimited_time||false;onEdited:(n,u)=>{sheet.edit("display_time",n);sheet.edit("unlimited_time",u)}}
                    ValueField {Layout.fillWidth:true;label:Theme.t("Studies per session");value:sheet.draft.num_images||10;maximum:100000;sliderMaximum:100;unlimited:sheet.draft.unlimited_images||false;onEdited:(n,u)=>{sheet.edit("num_images",n);sheet.edit("unlimited_images",u)}}
                    Rectangle {Layout.fillWidth:true;height:1;color:Theme.border}
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Enable breaks");checked:sheet.draft.use_break||false;onToggled:sheet.edit("use_break",checked)}
                    ValueField {visible:sheet.draft.use_break||false;Layout.fillWidth:true;label:Theme.t("Duration (min):");value:Math.max(1,Math.floor((sheet.draft.break_duration||300)/60));maximum:1440;sliderMaximum:60;unlimited:sheet.draft.unlimited_break||false;onEdited:(n,u)=>{sheet.edit("break_duration",n*60);sheet.edit("unlimited_break",u)}}
                    Rectangle {Layout.fillWidth:true;height:1;color:Theme.border}
                    ValueField {Layout.fillWidth:true;label:Theme.t("Timer sound volume");minimum:0;maximum:100;sliderMaximum:100;allowUnlimited:false;value:sheet.draft.timer_volume||0;onEdited:(n,u)=>sheet.edit("timer_volume",n)}
                    Text {Layout.fillWidth:true;text:Theme.t("A beat at 3, 2 and 1 seconds, then a final ding at the image change.");color:Theme.muted;wrapMode:Text.WordWrap;font.pixelSize:12}
                    Text {Layout.fillWidth:true;text:Theme.t("Timing changes apply to the next session.");color:Theme.muted;wrapMode:Text.WordWrap;font.pixelSize:12}
                }
                ColumnLayout {visible:sheet.section===1;Layout.fillWidth:true;spacing:16
                    ThemePicker {Layout.fillWidth:true;draft:sheet.draft;onEdited:(key,value)=>sheet.edit(key,value)}
                    Rectangle {Layout.fillWidth:true;height:1;color:Theme.border}
                    Text {text:Theme.t("Session panels");color:Theme.text;font.pixelSize:18;font.weight:Font.Medium;Layout.fillWidth:true;wrapMode:Text.WordWrap}
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Translucent session panels");checked:sheet.draft.session_glass||false;onToggled:sheet.edit("session_glass",checked)}
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Blur the background behind panels");checked:sheet.draft.session_blur||false;enabled:sheet.draft.session_glass||false;onToggled:sheet.edit("session_blur",checked)}
                    ValueField {visible:sheet.draft.session_glass||false;Layout.fillWidth:true;label:Theme.t("Panel opacity (%)");minimum:70;maximum:95;sliderMaximum:95;allowUnlimited:false;value:sheet.draft.session_panel_opacity||78;onEdited:(n,u)=>sheet.edit("session_panel_opacity",n)}
                    Text {Layout.fillWidth:true;text:Theme.t("Transparency affects the panel background. Text stays sharp.");color:Theme.muted;font.pixelSize:12;wrapMode:Text.WordWrap}
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Reduce motion");checked:sheet.draft.reduce_motion||false;onToggled:sheet.edit("reduce_motion",checked)}
                    ColumnLayout {Layout.fillWidth:true;spacing:8
                        Text {text:Theme.t("Interface language");color:Theme.muted;font.pixelSize:13}
                        LobeComboBox {Layout.fillWidth:true;Accessible.name:Theme.t("Interface language");model:i18n.languages;textRole:"name";valueRole:"code";currentIndex:i18n.languages.findIndex(item=>item.code===sheet.draft.language);onActivated:sheet.edit("language",currentValue)}
                    }
                }
                ColumnLayout {visible:sheet.section===2;Layout.fillWidth:true;spacing:16
                    Text {text:Theme.t("Window and focus");color:Theme.text;font.pixelSize:18;font.weight:Font.Medium;Layout.fillWidth:true;wrapMode:Text.WordWrap}
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Always on top");checked:sheet.draft.always_on_top||false;onToggled:sheet.edit("always_on_top",checked)}
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Drag image to move pinned window");checked:sheet.draft.drag_when_pinned||false;onToggled:sheet.edit("drag_when_pinned",checked)}
                    Text {text:Theme.t("Zoomed images can still be panned. Alt-drag moves the window.");color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap}
                    Rectangle {Layout.fillWidth:true;height:1;color:Theme.border}
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Show timer");checked:sheet.draft.show_timer||false;onToggled:sheet.edit("show_timer",checked)}
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Hide timer when pointer leaves");checked:sheet.draft.hide_timer_on_leave||false;onToggled:sheet.edit("hide_timer_on_leave",checked)}
                    ColumnLayout {Layout.fillWidth:true;spacing:8
                        Text {text:Theme.t("Timer position");color:Theme.muted;font.pixelSize:13}
                        LobeComboBox {Layout.fillWidth:true;textRole:"label";valueRole:"key";Accessible.name:Theme.t("Timer position");model:[{key:"Left",label:Theme.t("Left")},{key:"Center",label:Theme.t("Center")},{key:"Right",label:Theme.t("Right")}];currentIndex:model.findIndex(v=>v.key===sheet.draft.timer_position);onActivated:sheet.edit("timer_position",currentValue)}
                    }
                }
                ColumnLayout {visible:sheet.section===3;Layout.fillWidth:true;spacing:16
                    Text {text:Theme.t("A goal for your daily practice");color:Theme.text;font.pixelSize:18;font.weight:Font.Medium;Layout.fillWidth:true;wrapMode:Text.WordWrap}
                    Text {text:Theme.t("Choose time, studies or both. You can change your goal whenever you need.");color:Theme.muted;font.pixelSize:13;Layout.fillWidth:true;wrapMode:Text.WordWrap}
                    LobeComboBox {Layout.fillWidth:true;textRole:"label";valueRole:"key";Accessible.name:Theme.t("Today's goal");model:[{key:"time",label:Theme.t("Practice time")},{key:"studies",label:Theme.t("Completed studies")},{key:"both",label:Theme.t("Time and studies")}];currentIndex:model.findIndex(v=>v.key===sheet.draft.daily_goal_mode);onActivated:sheet.edit("daily_goal_mode",currentValue)}
                    ValueField {visible:sheet.draft.daily_goal_mode!=="studies";Layout.fillWidth:true;label:Theme.t("Minutes per day");minimum:1;maximum:240;sliderMaximum:60;allowUnlimited:false;value:sheet.draft.daily_goal_minutes||15;onEdited:(n,u)=>sheet.edit("daily_goal_minutes",n)}
                    ValueField {visible:sheet.draft.daily_goal_mode!=="time";Layout.fillWidth:true;label:Theme.t("Studies per day");minimum:1;maximum:1000;sliderMaximum:50;allowUnlimited:false;value:sheet.draft.daily_goal_studies||10;onEdited:(n,u)=>sheet.edit("daily_goal_studies",n)}
                    Text {text:Theme.t("Your calendar updates automatically from completed studies. The streak counts days with practice.");color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap}
                }
                ColumnLayout {visible:sheet.section===4;Layout.fillWidth:true;spacing:16
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Save viewed images");checked:sheet.draft.save_history||false;onToggled:sheet.edit("save_history",checked)}
                    LobeNotice {visible:!!sheet.draft.save_history&&!app.historyLoading&&!app.historyHealthy;Layout.fillWidth:true;warning:true;text:Theme.t("Practice is available. History cannot be saved right now.")}
                    LobeButton {text:Theme.t("Open data folder");iconName:"folder-open";font.pixelSize:12;onClicked:app.openDataFolder()}
                    Text {visible:!app.historyLoading&&!app.historyHealthy;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:Theme.muted;font.pixelSize:12;text:Theme.t("Check write access to this folder, free disk space if needed, then restart the app.")}
                    Text {text:Theme.t("History supplies your statistics and practice calendar.");color:Theme.muted;font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap}
                    LobeSwitch {Layout.fillWidth:true;text:Theme.t("Confirm file deletion");checked:sheet.draft.confirm_delete||false;onToggled:sheet.edit("confirm_delete",checked)}
                    Text {text:Theme.t("Grid Settings");color:Theme.text;font.pixelSize:16;font.weight:Font.Medium}
                    ValueField {Layout.fillWidth:true;label:Theme.t("Horizontal lines");minimum:0;maximum:10;sliderMaximum:10;allowUnlimited:false;value:sheet.draft.grid_h_lines===undefined?2:sheet.draft.grid_h_lines;onEdited:(n,u)=>sheet.edit("grid_h_lines",n)}
                    ValueField {Layout.fillWidth:true;label:Theme.t("Vertical lines");minimum:0;maximum:10;sliderMaximum:10;allowUnlimited:false;value:sheet.draft.grid_v_lines===undefined?2:sheet.draft.grid_v_lines;onEdited:(n,u)=>sheet.edit("grid_v_lines",n)}
                    Text {Layout.fillWidth:true;text:Theme.t("Space")+" · "+Theme.t("Pause")+" / "+Theme.t("Resume")+"\n← → · "+Theme.t("Previous image")+" / "+Theme.t("Next image")+"\nF11 · "+Theme.t("Fullscreen")+"\nAlt · "+Theme.t("Move window");color:Theme.muted;font.pixelSize:12;lineHeight:1.5;wrapMode:Text.WordWrap}
                    Text {text:"GestArt Ultimate · "+app.appVersion;color:Theme.subtle;font.pixelSize:12}
                }
                Item {height:8}
            }
        }
        Rectangle {Layout.fillWidth:true;height:1;color:Theme.border}
        RowLayout {Layout.fillWidth:true;Layout.margins:sheet.compact?8:16;spacing:8
            LobeButton {objectName:"settingsCancelButton";text:sheet.width<300?"":Theme.t("Cancel");Accessible.name:Theme.t("Cancel");iconName:sheet.width<300?"x":"";implicitHeight:sheet.compact?32:40;onClicked:sheet.close()}
            LobeButton {objectName:"settingsSaveButton";text:Theme.t("Save");iconName:"circle-check";primary:true;Layout.fillWidth:true;implicitHeight:sheet.compact?32:40;onClicked:{app.applySettings(sheet.draft);sheet.close()}}
        }
    }
}
