#include "online_sources.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QImageReader>
#include <QBuffer>
#include <QSaveFile>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QUrlQuery>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <algorithm>
#include <QSet>
#include <memory>

namespace {
constexpr qint64 MetadataLimit=8*1024*1024,ImageLimit=24*1024*1024,JobLimit=256*1024*1024;
QJsonObject object(const QByteArray& data){return QJsonDocument::fromJson(data).object();}
QString nameFor(const QString& id){return id=="quickposes"?"Quickposes":id=="proko"?"Proko":id=="poselibrary"?"Pose Library":"SketchDaily";}
QString identifier(const QJsonValue& v){return v.isString()?v.toString():v.isDouble()?QString::number(v.toInteger()):QString();}
}
OnlineSources::OnlineSources(QString directory,QObject* parent,QNetworkAccessManager* network):QObject(parent),directory_(std::move(directory)),network_(network?network:new QNetworkAccessManager(this)){
    catalogDeadline_.setSingleShot(true);catalogDeadline_.setInterval(45000);
    connect(&catalogDeadline_,&QTimer::timeout,this,[this]{fail("The source took too long to respond. Retry or open its website.");});
}
OnlineSources::~OnlineSources(){abortRequests();}
void OnlineSources::abortRequests(){
    ++generation_;catalogDeadline_.stop();
    const auto pending=replies_;replies_.clear();
    for(auto* reply:pending){reply->disconnect(this);reply->abort();reply->deleteLater();}
    activeImages_=0;sketchPending_=false;
}
void OnlineSources::copyCatalogFrom(const OnlineSources& source){
    if(source.busy_||source.sets_.isEmpty())return;
    const auto cached=source.setCache_.value(source.provider_);
    if(cached.second.isEmpty())return;
    setCache_[source.provider_]=cached;
    if(source.provider_=="poselibrary"){poseBase_=source.poseBase_;poseKey_=source.poseKey_;}
    if(source.provider_=="proko"){
        prokoVersion_=source.prokoVersion_;
        for(const auto& url:QList<QUrl>{website("proko"),QUrl("https://www.proko.com/api/")})
            network_->cookieJar()->setCookiesFromUrl(source.network_->cookieJar()->cookiesForUrl(url),url);
    }
}
void OnlineSources::cacheSets(){
    catalogDeadline_.stop();
    if(!sets_.isEmpty())setCache_[provider_]={QDateTime::currentMSecsSinceEpoch(),sets_};
}
QUrl OnlineSources::website(const QString& id){
    if(id=="quickposes")return QUrl("https://quickposes.com/en/gestures/timed");
    if(id=="proko")return QUrl("https://www.proko.com/timer");
    if(id=="poselibrary")return QUrl("https://poselibrary.com/");
    if(id=="sketchdaily")return QUrl("https://www.sketchdaily.net/en");
    return {};
}
bool OnlineSources::allowedUrl(const QUrl& url){
    if(!url.isValid()||url.scheme()!="https"||!url.userInfo().isEmpty()||(url.port()!=-1&&url.port()!=443))return false;
    const auto host=url.host().toLower();
    for(const auto& domain:QStringList{"quickposes.com","proko.com","sketchdaily.net","poselibrary.com","sajypjdlmwfemphcaqvi.supabase.co"})
        if(host==domain||host.endsWith("."+domain))return true;
    return false;
}
void OnlineSources::requestPreview(const QString& setId){
    if(busy_)return;
    QVariantMap selected;for(const auto& value:sets_)if(value.toMap().value("id")==setId)selected=value.toMap();
    if(selected.isEmpty())return;
    if(!previewWorker_){
        previewWorker_=new OnlineSources(directory_,this);
        previewWorker_->setDownloadRoot(directory_+"/cache/online-previews");
        connect(previewWorker_,&OnlineSources::changed,this,[this]{previewError_=previewWorker_->error();emit previewChanged();});
        connect(previewWorker_,&OnlineSources::imported,this,[this](const QString& folder,const QString&,int){
            for(const auto& file:QDir(folder).entryList(QDir::Files)){QImageReader reader(folder+"/"+file);if(reader.canRead()){
                previewImage_=QUrl::fromLocalFile(folder+"/"+file).toString();
                for(auto& value:sets_){auto set=value.toMap();if(set.value("id")==previewSet_){set["preview"]=previewImage_;value=set;}}
                QFile previous(directory_+"/cache/online-previews/index.json");QJsonObject index;
                if(previous.open(QIODevice::ReadOnly))index=object(previous.readAll());
                previous.close();
                index[provider_+":"+previewSet_]=folder+"/"+file;
                QSaveFile cache(directory_+"/cache/online-previews/index.json");
                if(cache.open(QIODevice::WriteOnly)){cache.write(QJsonDocument(index).toJson());cache.commit();}
                if(setCache_.contains(provider_))setCache_[provider_].second=sets_;
                emit changed();emit previewChanged();break;
            }}
        });
    }
    previewWorker_->cancel();previewSet_=setId;previewImage_=selected.value("preview").toString();previewError_.clear();
    if(previewImage_.isEmpty()){
        QFile cache(directory_+"/cache/online-previews/index.json");
        if(cache.open(QIODevice::ReadOnly)){
            const auto path=object(cache.readAll()).value(provider_+":"+setId).toString();
            if(path.startsWith(directory_+"/cache/online-previews/")&&QImageReader(path).canRead())previewImage_=QUrl::fromLocalFile(path).toString();
        }
    }
    if(!previewImage_.isEmpty()){emit previewChanged();return;}
    previewWorker_->provider_=provider_;previewWorker_->sets_={selected};previewWorker_->poseBase_=poseBase_;previewWorker_->poseKey_=poseKey_;previewWorker_->prokoVersion_=prokoVersion_;
    previewWorker_->importImages(setId,1,true);
}
void OnlineSources::openWebsite(const QString& id){const auto url=website(id);if(!url.isEmpty())QDesktopServices::openUrl(url);}
void OnlineSources::request(const QUrl& url,Callback done,const QJsonObject* body,bool image,int attempt){
    if(!allowedUrl(url)){fail("The source returned an unsupported link.");return;}
    const bool slowBackend=url.host()=="sajypjdlmwfemphcaqvi.supabase.co"&&!image;
    QNetworkRequest req(url);req.setTransferTimeout(slowBackend?25000:10000);req.setMaximumRedirectsAllowed(5);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::UserVerifiedRedirectPolicy);
    req.setRawHeader("User-Agent","Mozilla/5.0 (compatible; GestArtUltimate/1.7; desktop reference importer)");
    req.setRawHeader("Referer",website(provider_).toEncoded());
    req.setRawHeader("Accept",image?"image/*":"application/json,text/html,*/*");
    if(url.host()=="www.proko.com")req.setRawHeader("api-client-version",prokoVersion_.toUtf8());
    if(url.host()==QUrl(poseBase_).host()&&!poseKey_.isEmpty())req.setRawHeader("apikey",poseKey_.toUtf8());
    const bool hasBody=body!=nullptr;const QJsonObject payload=hasBody?*body:QJsonObject{};
    QNetworkReply* response;
    if(hasBody){req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");response=network_->post(req,(url.path().endsWith("/Next")?QJsonDocument(payload.value("_excludedIds").toArray()):QJsonDocument(payload)).toJson(QJsonDocument::Compact));}
    else response=network_->get(req);
    replies_.insert(response);const auto generation=generation_;const auto limit=image?ImageLimit:MetadataLimit;
    auto exceeded=std::make_shared<bool>(false);
    auto blockedRedirect=std::make_shared<bool>(false);
    auto* deadline=new QTimer(response);deadline->setSingleShot(true);deadline->start(slowBackend?35000:image?20000:15000);
    connect(deadline,&QTimer::timeout,response,&QNetworkReply::abort);
    connect(response,&QNetworkReply::redirected,this,[response,blockedRedirect](const QUrl& next){if(allowedUrl(next))response->redirectAllowed();else{*blockedRedirect=true;response->abort();}});
    connect(response,&QIODevice::readyRead,this,[response,limit,exceeded]{if(response->bytesAvailable()>limit){*exceeded=true;response->abort();}});
    connect(response,&QNetworkReply::finished,this,[this,response,generation,url,done=std::move(done),image,exceeded,blockedRedirect,payload,hasBody,attempt]{
        replies_.remove(response);
        const auto status=response->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto networkError=response->error();const auto data=response->isOpen()?response->readAll():QByteArray{};response->deleteLater();
        if(generation!=generation_)return;
        if(*exceeded||data.size()>(image?ImageLimit:MetadataLimit)||networkError!=QNetworkReply::NoError||status<200||status>=300){
            const bool transient=status==408||status==429||status>=500||(status==0&&networkError!=QNetworkReply::ContentAccessDenied);
            // Creating a Proko timer/session is a mutation; retry only reads and read-only RPCs.
            const bool retryable=!hasBody||url.path().contains("/functions/v1/")||url.path().endsWith("/Next");
            if(transient&&retryable&&!*exceeded&&!*blockedRedirect&&attempt<1){
                bool ok=false;int delay=response->rawHeader("Retry-After").toInt(&ok)*1000;
                delay=ok?std::clamp(delay,500,5000):750;
                QTimer::singleShot(delay,this,[this,generation,url,done,image,payload,hasBody,attempt]{if(generation==generation_)request(url,done,hasBody?&payload:nullptr,image,attempt+1);});return;
            }
            if(image){++failed_;imageDone();return;}
            fail(status==401||status==403?"This source requires access on its website.":"Could not connect. Check your connection and try again.");return;
        }
        done(data);
    });
}
void OnlineSources::fail(const QString& message){
    error_=message;abortRequests();
    if(importing_&&completed_>0){finish();return;}
    busy_=false;importing_=false;emit changed();
}
void OnlineSources::cancel(){
    abortRequests();
    if(importing_&&completed_>0){error_="Download stopped. Completed images were kept.";finish();return;}
    busy_=false;importing_=false;emit changed();
}
void OnlineSources::poseRequest(const QString& function,const QJsonObject& body,Callback done){request(QUrl(poseBase_+"/functions/v1/"+function),std::move(done),&body);}
void OnlineSources::selectProvider(const QString& id,bool refresh){
    if(busy_||website(id).isEmpty())return;
    if(previewWorker_)previewWorker_->cancel();previewImage_.clear();previewError_.clear();emit previewChanged();
    ++generation_;provider_=id;sets_.clear();error_.clear();completed_=0;requested_=0;busy_=true;emit changed();
    const auto cached=setCache_.value(id);
    if(!refresh&&!cached.second.isEmpty()&&QDateTime::currentMSecsSinceEpoch()-cached.first<600000){sets_=cached.second;busy_=false;emit changed();return;}
    catalogDeadline_.start();
    if(id=="quickposes"){
        for(const auto& pair:QList<QPair<QString,QString>>{{"pose_all_all","Full body"},{"face_all","Faces"},{"hands_all","Hands"},{"feet_all","Feet"},{"animals","Animals"},{"landscapes","Landscapes"},{"urban","Urban"}})
            sets_.append(QVariantMap{{"id",pair.first},{"name",pair.second},{"translate",true}});
        busy_=false;cacheSets();emit changed();return;
    }
    if(id=="sketchdaily"){sets_.append(QVariantMap{{"id","FullBodies"},{"name","Full body"},{"translate",true}});busy_=false;cacheSets();emit changed();return;}
    if(id=="proko"){
        request(website(id),[this](const QByteArray& data){
            const auto match=QRegularExpression("<script[^>]*id=\"proko-web-state\"[^>]*>(.*?)</script>",QRegularExpression::DotMatchesEverythingOption).match(QString::fromUtf8(data));
            const auto state=object(match.captured(1).toUtf8());
            for(const auto& value:state){const auto cached=value.toObject();if(!cached.value("u").toString().endsWith("/api/reference_packs"))continue;
                const auto version=cached.value("h").toObject().value("api-server-version").toArray();if(!version.isEmpty())prokoVersion_=version.first().toString();
                for(const auto& pack:cached.value("b").toObject().value("data").toArray()){
                    const auto p=pack.toObject();if(!p.value("free").toBool())continue;
                    const auto owner=p.value("owner").toObject();
                    const auto credit=owner.value("name").toString().isEmpty()?owner.value("username").toString():owner.value("name").toString();
                    sets_.append(QVariantMap{{"id",p.value("id").toString()},{"name",p.value("title").toString()},{"credit",credit},{"imageCount",p.value("images_count").toInt()},
                        {"preview",QString("https://img-resizer.proko.com/resize/reference_pack/cover_image/380x214x1/")+p.value("cover_image").toObject().value("signature").toString()+(p.value("has_mature_content").toBool()?"/blur/250bp":"")}});
                }
            }
            busy_=false;cacheSets();if(sets_.isEmpty())error_="No accessible images were returned. Open the source website.";emit changed();
        });return;
    }
    request(website(id),[this](const QByteArray& data){
        const auto match=QRegularExpression("<script[^>]*src=\"([^\"]*/assets/index-[^\"]+\\.js)\"").match(QString::fromUtf8(data));
        const auto script=website(provider_).resolved(QUrl(match.captured(1)));
        if(!match.hasMatch()){fail("The source changed its format. Open its website.");return;}
        request(script,[this](const QByteArray& source){
            const auto text=QString::fromUtf8(source);
            const auto base=QRegularExpression("(?:const|,)\\s*\\w+=\"(https://sajypjdlmwfemphcaqvi\\.supabase\\.co)\"").match(text);
            const auto key=QRegularExpression("\"(eyJ[A-Za-z0-9_.-]{100,})\"").match(text);
            if(!base.hasMatch()||!key.hasMatch()){fail("The source changed its format. Open its website.");return;}
            poseBase_=base.captured(1);poseKey_=key.captured(1);
            poseRequest("list-accessible-galleries",{{"signedUrlTtl",3600},{"previewOnly",false}},[this](const QByteArray& result){
                for(const auto& value:object(result).value("galleries").toArray()){
                    const auto g=value.toObject();if(!g.value("isFreeAccess").toBool())continue;
                    for(const auto& folder:QStringList{"sfw","nsfw"}){
                        if(!g.value(folder=="sfw"?"hasSfw":"hasNsfw").toBool())continue;
                        sets_.append(QVariantMap{{"id",g.value("id").toString()+"|"+folder},{"name",g.value("name").toString()+" · "+folder.toUpper()},{"imageCount",g.value(folder=="sfw"?"sfwCount":"nsfwCount").toInt()},
                            {"preview",allowedUrl(QUrl(g.value(folder=="sfw"?"coverUrlSfw":"coverUrlNsfw").toString()))?g.value(folder=="sfw"?"coverUrlSfw":"coverUrlNsfw").toString():QString{}}});
                    }
                }
                busy_=false;cacheSets();if(sets_.isEmpty())error_="No accessible images were returned. Open the source website.";emit changed();
            });
        });
    });
}
QList<OnlineSources::Reference> OnlineSources::parseReferences(const QString& provider,const QByteArray& data){
    const auto doc=QJsonDocument::fromJson(data);QJsonArray values;
    if(provider=="quickposes")values=doc.array();
    else if(provider=="proko")values=doc.object().value("data").toArray();
    else if(provider=="poselibrary")values=doc.object().value("images").toArray();
    else if(provider=="sketchdaily"&&!doc.object().isEmpty())values.append(doc.object());
    QList<Reference> result;QSet<QString> ids;
    for(const auto& value:values){const auto p=value.toObject();Reference ref;
        if(provider=="quickposes"){
            ref.id=identifier(p.value("id"));const auto path=p.value("src").toString();
            if(path.isEmpty()||path.contains("..")||path.contains(":")||path.startsWith('/'))continue;
            ref.url=QUrl("https://quickposes.com/assets/poses/"+path);ref.credit=p.value("contributor").toString();
        }else if(provider=="proko"){
            const auto image=p.value("image").toObject();if(image.value("is_invalid").toBool())continue;
            ref.id=image.value("filename").toString();ref.url=QUrl(image.value("download_url").toString());ref.credit=p.value("pack_title").toString();
        }else if(provider=="poselibrary"){
            ref.id=p.value("poseId").toString();ref.url=QUrl(p.value("src").toString());ref.credit=p.value("filePath").toString();
        }else if(provider=="sketchdaily"){
            ref.id=p.value("id").toString();const auto path=p.value("file").toString();
            if(path.contains("..")||!path.startsWith('/'))continue;
            ref.url=QUrl("https://files.sketchdaily.net/references"+path);ref.credit=p.value("photographer").toObject().value("name").toString();
        }
        if(ref.id.isEmpty()||ids.contains(ref.id)||!allowedUrl(ref.url))continue;
        ids.insert(ref.id);result.append(ref);
    }
    return result;
}
QVariantMap OnlineSources::normalizedFilters(const QString& provider,const QVariantMap& filters){
    QVariantMap result;
    if(provider!="quickposes"&&provider!="sketchdaily")return result;
    const auto accept=[&](const QString& key,const QStringList& allowed){const auto value=filters.value(key).toString();if(allowed.contains(value)&&value!="all")result[key]=value;};
    accept("gender",{"all","male","female"});accept("clothing",{"all","clothed","nude"});
    if(provider=="sketchdaily"){accept("pose",{"all","action","stationary"});accept("angle",{"all","front","side","back","above"});}
    return result;
}
QString OnlineSources::quickposesDescriptor(const QString& setId,const QVariantMap& filters){
    const auto type=setId.section('_',0,0);
    if(!QStringList{"pose","face","hands","feet"}.contains(type))return setId;
    const auto values=normalizedFilters("quickposes",filters);
    QString result=type+"_"+values.value("gender","all").toString();
    if(type=="pose"||type=="feet")result+="_"+(values.value("clothing")=="clothed"?QString("non-nude"):values.value("clothing")=="nude"?QString("nude & semi-nude"):QString("all"));
    return result;
}
QUrl OnlineSources::sketchUrl(const QVariantMap& filters){
    QUrl url("https://api-test.sketchdaily.net/ReferenceSite/FullBodies/Next");QUrlQuery query;
    const auto values=normalizedFilters("sketchdaily",filters);
    if(values.contains("gender"))query.addQueryItem("Gender",values.value("gender")=="male"?"Male":"Female");
    if(values.contains("clothing"))query.addQueryItem("Clothing",values.value("clothing")=="clothed"?"true":"false");
    if(values.contains("pose"))query.addQueryItem("PoseType",values.value("pose")=="action"?"Action":"Stationary");
    if(values.contains("angle")){const QMap<QString,QString> angles{{"front","Front"},{"side","Side"},{"back","Back"},{"above","AboveOrBelow"}};query.addQueryItem("ViewAngle",angles.value(values.value("angle").toString()));}
    url.setQuery(query);return url;
}
void OnlineSources::importImages(const QString& setId,int count,bool separateCollection,const QVariantMap& filters){
    if(busy_)return;
    QVariantMap selected;for(const auto& value:sets_)if(value.toMap().value("id").toString()==setId){selected=value.toMap();break;}
    if(selected.isEmpty())return;
    wholePack_=count==0;
    if(wholePack_&&provider_=="sketchdaily"){error_="This source offers random references, not a complete pack. Choose an image count.";emit changed();return;}
    if(wholePack_&&provider_=="proko"&&selected.value("imageCount").toInt()<=0){error_="The source did not report the pack size. Refresh the source or choose an image count.";emit changed();return;}
    separateCollection_=separateCollection;filters_=normalizedFilters(provider_,filters);
    if(provider_=="quickposes"){const auto type=setId.section('_',0,0);if(!QStringList{"pose","feet","hands","face"}.contains(type))filters_.clear();else if(type=="hands"||type=="face")filters_.remove("clothing");}
    const auto base=downloadRoot_.isEmpty()?directory_+"/online-references":downloadRoot_;
    auto label=selected.value("name").toString();label.replace(QRegularExpression("[^\\p{L}\\p{N} _.-]"),"_");label=label.left(48).trimmed();while(label.endsWith('.'))label.chop(1);
    const auto hash=QString::fromLatin1(QCryptographicHash::hash(setId.toUtf8()+(filters_.isEmpty()?QByteArray{}:QJsonDocument::fromVariant(filters_).toJson(QJsonDocument::Compact)),QCryptographicHash::Sha256).toHex().left(12));
    folder_=separateCollection?base+"/sets/"+provider_+"/"+label+"-"+hash:base+"/"+provider_;
    ++generation_;setId_=setId;setName_=selected.value("name").toString();setCredit_=selected.value("credit").toString();error_.clear();queue_.clear();manifest_=QJsonArray{};sketchIds_.clear();
    requested_=wholePack_?std::max(0,selected.value("imageCount").toInt()):std::clamp(count,1,100);completed_=index_=failed_=attempts_=0;totalBytes_=0;busy_=true;importing_=true;emit changed();
    if(provider_=="quickposes")request(QUrl("https://quickposes.com/ajax/getPosesForSlider/"+quickposesDescriptor(setId,filters_)),[this](const QByteArray& data){acceptReferences(parseReferences(provider_,data));});
    else if(provider_=="sketchdaily")fetchSketch();
    else if(provider_=="poselibrary")poseRequest("get-gallery-poses",{{"galleryId",setId.section('|',0,0)},{"contentFolder",setId.section('|',1,1)},{"signedUrlTtl",3600},{"includeFullSize",true}},[this](const QByteArray& data){acceptReferences(parseReferences(provider_,data));});
    else if(provider_=="proko"){
        const QJsonObject create{{"timer_tool",QJsonObject{{"images_count",requested_},{"seconds_per_image",30},{"reference_pack_ids",QJsonArray{setId}}}}};
        request(QUrl("https://www.proko.com/api/timer_tools"),[this](const QByteArray& data){
            const auto id=object(data).value("data").toObject().value("id").toString();
            if(!QRegularExpression("^[a-zA-Z0-9-]+$").match(id).hasMatch()){fail("The source changed its format. Open its website.");return;}
            const auto base="https://www.proko.com/api/timer_tools/"+id;
            const QJsonObject start{{"refresh",true}};
            request(QUrl(base+"/sessions"),[this,base](const QByteArray&){fetchProkoImages(base);},&start);
        },&create);
    }
}
void OnlineSources::fetchProkoImages(const QString& base,int page){
    const auto pageSize=std::min(100,requested_);
    request(QUrl(base+"/images?per_page="+QString::number(pageSize)+"&page="+QString::number(page)+"&with=image,pack_title"),[this,base,page](const QByteArray& data){
        const auto values=object(data).value("data").toArray();
        const auto refs=parseReferences("proko",data);QSet<QString> ids;
        for(const auto& ref:queue_)ids.insert(ref.id);
        const int previous=queue_.size();
        for(const auto& ref:refs)if(!ids.contains(ref.id)){ids.insert(ref.id);queue_.append(ref);}
        if(queue_.size()>=requested_||values.isEmpty()){
            auto references=queue_;
            if(wholePack_&&references.size()<requested_)error_="Some images in this pack are not accessible.";
            acceptReferences(references);return;
        }
        if(queue_.size()==previous){fail("The source repeated a page. Refresh it and try again.");return;}
        fetchProkoImages(base,page+1);
    });
}
void OnlineSources::fetchSketch(){
    if(!busy_||!importing_||sketchPending_)return;
    sketchPending_=true;++attempts_;
    // /Next accepts an exclusion array; use the shared bounded request path.
    const QJsonObject body{{"_excludedIds",QJsonArray::fromStringList(sketchIds_)}};
    request(sketchUrl(filters_),[this](const QByteArray& data){
        sketchPending_=false;
        const auto refs=parseReferences("sketchdaily",data);
        for(const auto& ref:refs)if(!sketchIds_.contains(ref.id)){sketchIds_.append(ref.id);queue_.append(ref);}
        downloadNext();
    },&body);
}
void OnlineSources::acceptReferences(QList<Reference> references){
    if(references.isEmpty()){fail("No accessible images were returned. Open the source website.");return;}
    std::shuffle(references.begin(),references.end(),*QRandomGenerator::global());
    queue_=wholePack_?references:references.mid(0,requested_);
    requested_=queue_.size();emit changed();downloadNext();
}
void OnlineSources::imageDone(){
    --activeImages_;const auto generation=generation_;emit changed();
    QTimer::singleShot(0,this,[this,generation]{if(generation==generation_)downloadNext();});
}
void OnlineSources::downloadNext(){
    if(!busy_||!importing_)return;
    if(!QDir().mkpath(folder_)){fail("Cannot save images. Check free space and folder access.");return;}
    while(busy_&&importing_&&activeImages_<4&&index_<queue_.size()){
        const auto reference=queue_[index_++];++activeImages_;
        const auto hash=QString::fromLatin1(QCryptographicHash::hash((provider_+":"+reference.id).toUtf8(),QCryptographicHash::Sha256).toHex());
        const auto file=folder_+"/"+hash;
        auto record=[this,reference](const QString& filename){
            manifest_.append(QJsonObject{{"file",filename},{"provider",provider_},{"referenceId",reference.id},{"credit",setCredit_.isEmpty()?reference.credit:setCredit_+" · "+reference.credit},{"set",setName_},{"setId",setId_},{"filters",QJsonObject::fromVariantMap(filters_)},{"website",website(provider_).toString()}});
            ++completed_;imageDone();
        };
        bool cached=false;
        for(const auto& name:QDir(folder_).entryList({hash+".*"},QDir::Files)){
            QImageReader reader(folder_+"/"+name);if(reader.canRead()){record(name);cached=true;break;}
        }
        if(cached)continue;
        request(reference.url,[this,file,record](const QByteArray& data){
            if(!wholePack_&&totalBytes_+data.size()>JobLimit){fail("The download is too large. Choose fewer images.");return;}
            QBuffer buffer;buffer.setData(data);buffer.open(QIODevice::ReadOnly);QImageReader reader(&buffer);const auto size=reader.size();
            if(!reader.canRead()||size.isEmpty()||qint64(size.width())*size.height()>64000000){++failed_;imageDone();return;}
            const auto format=reader.format();
            if(!QList<QByteArray>{"jpeg","png","webp","avif","gif","bmp"}.contains(format)||reader.read().isNull()){++failed_;imageDone();return;}
            const auto filename=file+"."+(format=="jpeg"?QString("jpg"):QString::fromLatin1(format));
            QSaveFile out(filename);if(!out.open(QIODevice::WriteOnly)||out.write(data)!=data.size()||!out.commit()){fail("Cannot save images. Check free space and folder access.");return;}
            totalBytes_+=data.size();record(QFileInfo(filename).fileName());
        },nullptr,true);
    }
    if(provider_=="sketchdaily"&&queue_.size()<requested_&&attempts_<requested_+8){fetchSketch();return;}
    if(activeImages_==0&&index_>=queue_.size()&&!sketchPending_)finish();
}
void OnlineSources::finish(){
    if(!importing_)return;
    if(completed_==0){busy_=false;importing_=false;if(error_.isEmpty())error_="No images could be downloaded. Try again or open the source website.";emit changed();return;}
    const auto folder=folder_;
    // Keep attribution for earlier imports when extending an existing collection.
    QFile previous(folder+"/sources.json");QJsonObject records;
    if(previous.open(QIODevice::ReadOnly))records=object(previous.readAll());
    previous.close();
    for(const auto& value:manifest_){const auto entry=value.toObject();records[entry.value("file").toString()]=entry;}
    const auto sourceData=QJsonDocument(records).toJson();QSaveFile provenance(folder+"/sources.json");
    if(!provenance.open(QIODevice::WriteOnly)||provenance.write(sourceData)!=sourceData.size()||!provenance.commit())
        error_="Images were saved, but the source index could not be updated. Check folder access.";
    busy_=false;importing_=false;if((failed_>0||completed_<requested_)&&error_.isEmpty())error_="Some images could not be downloaded.";
    emit changed();emit imported(folder,separateCollection_?nameFor(provider_)+" · "+setName_:nameFor(provider_),completed_);
}
