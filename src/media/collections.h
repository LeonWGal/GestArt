#pragma once
#include <QVariantList>
#include <QStringList>

QVariantMap makeCollection(QStringList paths, const QString& kind, const QString& name);
QVariantList normalizeCollections(const QVariantList& values);
QStringList collectionPaths(const QVariantList& collections, bool selectedOnly = false);
bool pathInSources(const QString& path, const QStringList& sources);
