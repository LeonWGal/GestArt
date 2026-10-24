pragma Singleton
import QtQuick
QtObject {
    readonly property bool dark: app.dark
    readonly property var colors: app.themePalette
    readonly property color canvas: colors.canvas
    readonly property color panel: colors.panel
    readonly property color raised: colors.raised
    readonly property color hover: colors.hover
    readonly property color border: colors.border
    readonly property color text: colors.text
    readonly property color muted: colors.muted
    readonly property color subtle: colors.subtle
    readonly property color primary: colors.primary
    readonly property color primaryText: colors.primaryText
    readonly property color accent: colors.accent
    readonly property color accentText: colors.accentText
    readonly property color accentSoft: Qt.rgba(accent.r,accent.g,accent.b,dark?.14:.10)
    readonly property color overlay: Qt.rgba(panel.r,panel.g,panel.b,0.96)
    readonly property color danger: dark ? "#ee9e9e" : "#a02d2d"
    readonly property int radius: 12
    readonly property string font: "Inter"
    function t(text) { const language = i18n.language; return i18n.s(text); }
    function duration(ms) {const seconds=Math.max(0,Math.round((ms||0)/1000));if(seconds<60)return seconds+" "+t("sec");const minutes=Math.floor(seconds/60);if(minutes<60)return minutes+" "+t("min");return Math.floor(minutes/60)+" "+t("h")+(minutes%60?" "+minutes%60+" "+t("min"):"")}
    function dateLabel(iso,format) {if(!iso)return "";const parts=iso.split("-");return new Date(Number(parts[0]),Number(parts[1])-1,Number(parts[2]),12).toLocaleDateString(Qt.locale(i18n.language),format||"d MMM")}
    function sessionSummary(s) {return (s.unlimited_time?"∞":(s.display_time||30)+" "+t("sec"))+" · "+(s.unlimited_images?"∞ "+t("studies"):count(s.num_images||10,"study","studies_few","studies"))}
    function monthLabel(iso) {if(!iso)return "";const parts=iso.split("-");const name=Qt.locale(i18n.language).standaloneMonthName(Number(parts[1])-1,Locale.LongFormat);return name.charAt(0).toUpperCase()+name.slice(1)+" "+parts[0]}
    function timeCode(ms) {const secs=Math.max(0,Math.round((ms||0)/1000));return Math.floor(secs/60)+":"+String(secs%60).padStart(2,"0")}
    function goalSummary(p) {const time=duration(p.todayMs||0)+" / "+(p.goalMinutes||15)+" "+t("min");const studies=(p.todayStudies||0)+" / "+(p.goalStudies||10)+" "+t("studies");return p.goalMode==="studies"?studies:p.goalMode==="both"?time+"\n"+studies:time}
    function count(n,singular,few,plural) {
        const language=i18n.language;
        return i18n.count(n,singular,few,plural);
    }
}
