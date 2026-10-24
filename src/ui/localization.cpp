#include "localization.h"
#include <QFile>
#include <QJsonDocument>
#include <QGuiApplication>
static QJsonObject readObject(const QString& path){QFile f(path);if(!f.open(QIODevice::ReadOnly))return {};return QJsonDocument::fromJson(f.readAll()).object();}
Localization::Localization(QObject* parent):QObject(parent){
    modernEnglish_=readObject(":/assets/locales-v2/en.json");
    const auto names=readObject(":/assets/languages.json");
    for(auto it=names.begin();it!=names.end();++it)
        if(QFile::exists(":/assets/locales-v2/"+it.key()+".json"))
            languages_.append(QVariantMap{{"code",it.key()},{"name",it.value().toString()}});
    setLanguage("ru");
}
void Localization::setLanguage(const QString& language){
    // Saved preferences may refer to a retired catalog. Keep the UI and selection in sync.
    const auto names=readObject(":/assets/languages.json");
    language_=names.contains(language)?language:QStringLiteral("ru");
    modern_=readObject(":/assets/locales-v2/"+language_+".json");
    QGuiApplication::setLayoutDirection(rtl()?Qt::RightToLeft:Qt::LeftToRight);emit changed();
}
QString Localization::s(const QString& text)const{
    if(modern_.contains(text))return modern_.value(text).toString(text);
    return modernEnglish_.value(text).toString(text);
}

namespace {
QString cardinalCategory(const QString& locale,int n){
    const auto language=locale.section('-',0,0);const int last=n%10,lastTwo=n%100;
    if(language=="ar"){
        if(n==0)return "zero";if(n==1)return "one";if(n==2)return "two";
        if(lastTwo>=3&&lastTwo<=10)return "few";
        if(lastTwo>=11&&lastTwo<=99)return "many";return "other";
    }
    if(language=="ru"||language=="uk"){
        if(last==1&&lastTwo!=11)return "one";
        if(last>=2&&last<=4&&(lastTwo<12||lastTwo>14))return "few";return "many";
    }
    if(language=="pl"){
        if(n==1)return "one";if(last>=2&&last<=4&&(lastTwo<12||lastTwo>14))return "few";return "many";
    }
    if(language=="cs"){if(n==1)return "one";if(n>=2&&n<=4)return "few";return "other";}
    if(language=="ro"){if(n==1)return "one";if(n==0||(lastTwo>=1&&lastTwo<=19))return "few";return "other";}
    if(language=="fr"||locale=="pt-BR")return n==0||n==1?"one":"other";
    if(QStringList{"zh","ja","ko","th","vi","id","tr"}.contains(language))return "other";
    return n==1?"one":"other";
}
}
QString Localization::count(int number,const QString& singular,const QString& few,const QString& plural)const{
    const auto category=cardinalCategory(language_,qAbs(number));
    const auto key="_plural."+singular+"."+category;
    QString word;
    if(modern_.contains(key))word=modern_.value(key).toString();
    else word=s(category=="one"?singular:category=="few"?few:plural);
    return QString::number(number)+" "+word;
}
