#include "themes.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

QVariantMap themeCatalog() {
    static const QVariantMap catalog = [] {
        QFile file(":/assets/themes.json");
        if (!file.open(QIODevice::ReadOnly)) return QVariantMap{};
        return QJsonDocument::fromJson(file.readAll()).object().toVariantMap();
    }();
    return catalog;
}
static QVariantMap findOption(const QString& group, const QString& key) {
    const auto list = themeCatalog().value(group).toList();
    for (const auto& option : list) {
        const auto value = option.toMap();
        if (value.value("key").toString() == key) return value;
    }
    return list.isEmpty() ? QVariantMap{} : list.first().toMap();
}
void normalizeTheme(QVariantMap& p, bool migrateLegacy) {
    if (migrateLegacy) {
        const QString old = p.value("theme", "dark").toString();
        p["theme_base"] = old == "light" || old == "calcite" ? "white" : "black";
        p["theme_follow_system"] = old == "system";
        p["theme_white_tone"] = old == "calcite" ? "paper" : "neutral";
        p["theme_black_tone"] = "neutral";
        p["theme_accent_group"] = "lobe";
        p["theme_accent"] = "purple";
    }
    if (!QStringList{"white","black"}.contains(p.value("theme_base").toString())) p["theme_base"] = "black";
    for (const QString base : {QString("white"),QString("black")}) {
        const QString field = "theme_" + base + "_tone";
        p[field] = findOption(base,p.value(field).toString()).value("key","neutral");
    }
    p["theme_accent_group"] = "lobe";
    static const QHash<QString,QString> stoneToLobe{
        {"charoite","purple"},{"amethyst","purple"},{"emerald","green"},{"malachite","green"},
        {"sapphire","blue"},{"lapis","blue"},{"amber","gold"},{"citrine","yellow"},
        {"aquamarine","cyan"},{"tourmaline","magenta"},{"rhodonite","magenta"},{"garnet","red"},
        {"ruby","red"},{"carnelian","volcano"},{"jasper","volcano"},{"topaz","cyan"}
    };
    const QString accentKey = p.value("theme_accent").toString();
    if (stoneToLobe.contains(accentKey)) {
        p["theme_accent"] = stoneToLobe.value(accentKey);
    }
    p["theme_accent"] = findOption("lobe",p.value("theme_accent").toString()).value("key","purple");
    p["schema"] = 2;
    // Retain a compatible field for older versions and legacy export.
    p["theme"] = p.value("theme_follow_system").toBool() ? "system" : p.value("theme_base").toString() == "white" ? "light" : "dark";
}
QVariantMap themePalette(const QVariantMap& p, bool dark) {
    const QString base = dark ? "black" : "white";
    auto colors = findOption(base,p.value("theme_"+base+"_tone").toString()).value("colors").toMap();
    const auto accent = findOption("lobe",p.value("theme_accent").toString());
    colors["accent"] = accent.value(base);
    colors["accentText"] = accent.value(base+"Text",accent.value(base));
    colors["accentName"] = accent.value("label");
    colors["primary"] = accent.value(base);
    colors["primaryText"] = accent.value(base+"Ink",dark?"#141414":"#ffffff");
    return colors;
}
