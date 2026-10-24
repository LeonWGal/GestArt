#pragma once
#include <QDate>
#include <QVariantMap>
#include <QVector>

struct PracticeDay {QDate date;int studies=0;qint64 durationMs=0;};
QVariantMap summarizePractice(const QVector<PracticeDay>& days,QDate today,QDate month,int period,
    int goalMinutes,int goalStudies,const QString& goalMode);
