#include "practice_stats.h"
#include <QMap>
#include <algorithm>

QVariantMap summarizePractice(const QVector<PracticeDay>& rows,QDate today,QDate month,int period,int goalMinutes,int goalStudies,const QString& goalMode){
    if(!today.isValid())today=QDate::currentDate();
    if(!month.isValid())month=today;month=QDate(month.year(),month.month(),1);
    goalMinutes=std::clamp(goalMinutes,1,240);goalStudies=std::clamp(goalStudies,1,1000);
    if(period!=7&&period!=30&&period!=90)period=0;
    QMap<QDate,PracticeDay> days;
    for(const auto& row:rows)if(row.date.isValid()&&row.date<=today&&row.studies>0){auto& value=days[row.date];value.date=row.date;value.studies+=row.studies;value.durationMs+=std::max<qint64>(0,row.durationMs);}
    auto progress=[&](const PracticeDay& day){const double time=double(day.durationMs)/(goalMinutes*60000.0),studies=double(day.studies)/goalStudies;return std::clamp(goalMode=="studies"?studies:goalMode=="both"?std::min(time,studies):time,0.0,1.0);};
    auto dayMap=[&](QDate date){const auto day=days.value(date);return QVariantMap{{"date",date.toString(Qt::ISODate)},{"studies",day.studies},{"durationMs",day.durationMs},{"progress",progress(day)},{"goalReached",progress(day)>=1},{"today",date==today},{"future",date>today},{"inMonth",date.month()==month.month()&&date.year()==month.year()}};};
    const QDate start=period>0?today.addDays(1-period):QDate{};
    qint64 totalMs=0,allMs=0;int studies=0,allStudies=0,active=0,best=0,run=0;QDate previous;
    for(const auto& day:days){allMs+=day.durationMs;allStudies+=day.studies;
        if(!start.isValid()||day.date>=start){totalMs+=day.durationMs;studies+=day.studies;++active;}
        run=previous.isValid()&&previous.daysTo(day.date)==1?run+1:1;best=std::max(best,run);previous=day.date;
    }
    int streak=0;QDate cursor=days.contains(today)?today:today.addDays(-1);
    while(days.contains(cursor)){++streak;cursor=cursor.addDays(-1);}
    QVariantList week,calendar,chart;const QDate monday=today.addDays(1-today.dayOfWeek());
    for(int i=0;i<7;++i)week.append(dayMap(monday.addDays(i)));
    const QDate gridStart=month.addDays(1-month.dayOfWeek());
    for(int i=0;i<42;++i)calendar.append(dayMap(gridStart.addDays(i)));
    const int chartDays=period==0?90:period;qint64 chartMax=0;
    for(int i=1-chartDays;i<=0;++i){auto value=dayMap(today.addDays(i));chartMax=std::max(chartMax,value.value("durationMs").toLongLong());chart.append(value);}
    const auto current=days.value(today);
    return {{"today",today.toString(Qt::ISODate)},{"month",month.toString(Qt::ISODate)},{"period",period},
        {"totalMs",totalMs},{"totalStudies",studies},{"activeDays",active},{"averageMs",studies>0?totalMs/studies:0},
        {"allTimeMs",allMs},{"allTimeStudies",allStudies},{"currentStreak",streak},{"bestStreak",best},
        {"todayMs",current.durationMs},{"todayStudies",current.studies},{"goalMinutes",goalMinutes},{"goalStudies",goalStudies},
        {"goalMode",goalMode},{"goalProgress",progress(current)},{"goalReached",progress(current)>=1},
        {"week",week},{"calendar",calendar},{"chart",chart},{"chartMaxMs",chartMax},{"chartDays",chartDays}};
}
