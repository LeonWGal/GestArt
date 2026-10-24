#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>
#include <QUrl>
#include <functional>
#include <QSet>
#include <QTimer>
#include <QDateTime>

class OnlineSources : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString provider READ provider NOTIFY changed)
    Q_PROPERTY(QVariantList sets READ sets NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool importing READ importing NOTIFY changed)
    Q_PROPERTY(int completed READ completed NOTIFY changed)
    Q_PROPERTY(int requested READ requested NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString previewImage READ previewImage NOTIFY previewChanged)
    Q_PROPERTY(QString previewError READ previewError NOTIFY previewChanged)
    Q_PROPERTY(bool previewBusy READ previewBusy NOTIFY previewChanged)
public:
    struct Reference {QString id;QUrl url;QString credit;};
    explicit OnlineSources(QString directory,QObject* parent=nullptr,QNetworkAccessManager* network=nullptr);
    ~OnlineSources() override;
    QString provider()const{return provider_;}
    QVariantList sets()const{return sets_;}
    bool busy()const{return busy_;}
    bool importing()const{return importing_;}
    int completed()const{return completed_;}
    int requested()const{return requested_;}
    QString error()const{return error_;}
    Q_INVOKABLE void selectProvider(const QString& id,bool refresh=false);
    Q_INVOKABLE void importImages(const QString& setId,int count,bool separateCollection=false,const QVariantMap& filters={});
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void openWebsite(const QString& id);
    Q_INVOKABLE QUrl siteUrl(const QString& id)const{return website(id);}
    static QUrl website(const QString& id);
    Q_INVOKABLE void requestPreview(const QString& setId);
    QString previewImage()const{return previewImage_;}
    QString previewError()const{return previewError_;}
    bool previewBusy()const{return previewWorker_&&previewWorker_->busy();}
    void copyCatalogFrom(const OnlineSources& source);
    void setDownloadRoot(const QString& root){downloadRoot_=root;}
    static QVariantMap normalizedFilters(const QString& provider,const QVariantMap& filters);
    static QString quickposesDescriptor(const QString& setId,const QVariantMap& filters);
    static QUrl sketchUrl(const QVariantMap& filters);
    static bool allowedUrl(const QUrl& url);
    static QList<Reference> parseReferences(const QString& provider,const QByteArray& data);
signals:
    void changed();
    void previewChanged();
    void imported(const QString& folder,const QString& name,int count);
private:
    using Callback=std::function<void(QByteArray)>;
    void request(const QUrl& url,Callback done,const QJsonObject* body=nullptr,bool image=false,int attempt=0);
    void abortRequests();
    void imageDone();
    void cacheSets();
    void fail(const QString& message);
    void finish();
    void downloadNext();
    void fetchSketch();
    void fetchProkoImages(const QString& base,int page=1);
    void acceptReferences(QList<Reference> references);
    void poseRequest(const QString& function,const QJsonObject& body,Callback done);
    bool separateCollection_=false,wholePack_=false;
    QString folder_,downloadRoot_;
    QString directory_,provider_,setId_,setName_,setCredit_,error_,poseBase_,poseKey_,prokoVersion_="41.0.0";
    QVariantList sets_;
    QVariantMap filters_;
    QList<Reference> queue_;
    QJsonArray manifest_;
    QStringList sketchIds_;
    QNetworkAccessManager* network_;
    QSet<QNetworkReply*> replies_;
    QHash<QString,QPair<qint64,QVariantList>> setCache_;
    OnlineSources* previewWorker_=nullptr;
    QString previewImage_,previewError_,previewSet_;
    bool sketchPending_=false;
    int activeImages_=0;
    QTimer catalogDeadline_;
    quint64 generation_=0;
    bool busy_=false,importing_=false;
    int requested_=0,completed_=0,index_=0,failed_=0,attempts_=0;
    qint64 totalBytes_=0;
};
