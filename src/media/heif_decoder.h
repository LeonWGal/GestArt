#pragma once
#include <QImage>
QImage decodeHeif(const QString& path,QSize requested,QString* error);
QSize heifNaturalSize(const QString& path);
