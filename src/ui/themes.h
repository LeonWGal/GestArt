#pragma once
#include <QVariantMap>

QVariantMap themeCatalog();
void normalizeTheme(QVariantMap& preferences, bool migrateLegacy = false);
QVariantMap themePalette(const QVariantMap& preferences, bool dark);
