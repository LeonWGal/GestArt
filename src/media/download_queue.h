#pragma once
#include "online_sources.h"
class DownloadQueue : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList savedSets READ savedSets NOTIFY savedChanged)
    Q_PROPERTY(QVariantList jobs READ jobs NOTIFY changed)
    Q_PROPERTY(bool running READ running NOTIFY changed)
    Q_PROPERTY(int completed READ completed NOTIFY changed)
    Q_PROPERTY(int requested READ requested NOTIFY changed)
    Q_PROPERTY(QString destination READ destination NOTIFY changed)
    Q_PROPERTY(QUrl destinationUrl READ destinationUrl NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
public:
    explicit DownloadQueue(QString directory,QObject* parent=nullptr,QNetworkAccessManager* network=nullptr);
    ~DownloadQueue() override;
    QVariantList savedSets()const{return savedSets_;}
    Q_INVOKABLE void refreshSavedSets();
    Q_INVOKABLE void openSavedFolder(int index);
    Q_INVOKABLE bool resumeSaved(int index);
    Q_INVOKABLE bool download(const QString& provider,const QVariantList& sets,int count,const QVariantMap& filters={},bool startAfter=false);
    Q_INVOKABLE void retry(int index);
    QVariantList jobs()const{return jobs_;}
    bool running()const{return running_;}
    int completed()const;
    int requested()const;
    QUrl destinationUrl()const{return QUrl::fromLocalFile(destination_);}
    QString destination()const{return destination_;}
    Q_INVOKABLE void setDestination(const QUrl& folder);
    Q_INVOKABLE void openFolder(int index);
    QString error()const{return error_;}
    Q_INVOKABLE bool add(const QString& provider,const QVariantList& sets,int count,bool separate,const QVariantMap& filters={});
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void start();
    Q_INVOKABLE bool practice(const QString& provider,const QVariantMap& set,int count,const QVariantMap& filters={});
    void setCatalogSource(OnlineSources* source){catalogSource_=source;}
    Q_INVOKABLE void cancel();
signals:
    void changed();
    void savedChanged();
    void practiceReady(const QString& folder);
    void imported(const QString& folder,const QString& name,int count);
private:
    enum class Phase {Idle,Loading,Importing,Cancelling};
    void next();
    void startSelected(const QList<int>& indices,int practiceJob=-1);
    void rememberSaved(const QString& folder,const QString& name,const QVariantMap& job={});
    void persistSaved();
    void workerChanged();
    void finishJob(const QString& status,const QString& error);
    OnlineSources worker_;
    QVariantList jobs_,savedSets_;
    QSet<int> selectedJobs_;
    QString error_,directory_,destination_;
    bool running_=false;
    int current_=-1,practiceJob_=-1;
    OnlineSources* catalogSource_=nullptr;
    quint64 run_=0;
    Phase phase_=Phase::Idle;
};
