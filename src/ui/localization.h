#pragma once
#include <QObject>
#include <QJsonObject>
#include <QVariantList>
class Localization : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY changed)
    Q_PROPERTY(QVariantList languages READ languages CONSTANT)
    Q_PROPERTY(bool rtl READ rtl NOTIFY changed)
public:
    explicit Localization(QObject* parent=nullptr);
    QString language() const {return language_;}
    void setLanguage(const QString& language);
    QVariantList languages() const {return languages_;}
    bool rtl() const {return language_=="ar";}
    Q_INVOKABLE QString s(const QString& text) const;
    Q_INVOKABLE QString count(int number,const QString& singular,const QString& few,const QString& plural) const;
signals:
    void changed();
private:
    QString language_="ru";
    QJsonObject modern_,modernEnglish_;
    QVariantList languages_;
};
