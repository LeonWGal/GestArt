#include <QUrlQuery>
#include <QtTest>
#include <QTemporaryDir>
#include "online_tests.h"
#include "media/online_sources.h"
#include "media/download_queue.h"
#include <QDir>
#include <QImage>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QBuffer>
#include <QTimer>
#include <cstring>
namespace {
class TestReply : public QNetworkReply {
public:
    TestReply(const QNetworkRequest& request,QByteArray bytes,int status,int delay,QObject* parent):QNetworkReply(parent),bytes_(std::move(bytes)){
        setRequest(request);setUrl(request.url());setAttribute(QNetworkRequest::HttpStatusCodeAttribute,status);open(QIODevice::ReadOnly);
        if(delay>=0)QTimer::singleShot(delay,this,[this]{complete();});
    }
    void abort()override{if(isFinished())return;setAttribute(QNetworkRequest::HttpStatusCodeAttribute,0);setError(OperationCanceledError,"cancelled");complete();}
    qint64 bytesAvailable()const override{return bytes_.size()-offset_+QNetworkReply::bytesAvailable();}
protected:
    qint64 readData(char* data,qint64 max)override{const auto count=std::min(max,qint64(bytes_.size()-offset_));if(count<=0)return -1;std::memcpy(data,bytes_.constData()+offset_,size_t(count));offset_+=count;return count;}
private:
    void complete(){if(isFinished())return;setFinished(true);emit readyRead();emit finished();}
    QByteArray bytes_;qint64 offset_=0;
};
class TestNetwork : public QNetworkAccessManager {
public:
    int active=0,peak=0,catalogRequests=0,aborted=0;
    bool stall=false,slowImages=false;
    int prokoCount=105;
    bool requireProkoCookie=false;
    QHash<QString,int> attempts;
    QByteArray image;
    TestNetwork(){QImage pixels(20,30,QImage::Format_RGB32);pixels.fill(Qt::red);QBuffer buffer(&image);buffer.open(QIODevice::WriteOnly);pixels.save(&buffer,"PNG");}
protected:
    QNetworkReply* createRequest(Operation,const QNetworkRequest& request,QIODevice*)override{
        const auto path=request.url().path();++attempts[path];
        QByteArray data;int status=200,delay=20;
        const bool isImage=path.endsWith(".png");
        if(isImage){data=image;if(path.endsWith("0.png")&&attempts[path]==1)status=503;if(slowImages&&peak>0)delay=2000;}
        else if(path.contains("getPosesForSlider")){
            QJsonArray refs;for(int i=0;i<8;++i)refs.append(QJsonObject{{"id",i},{"src",QString::number(i)+".png"},{"contributor","Test author"}});
            data=QJsonDocument(refs).toJson();
        }else if(path=="/api/timer_tools"){
            data=R"({"data":{"id":"test-tool"}})";
            if(requireProkoCookie&&cookieJar()->cookiesForUrl(request.url()).isEmpty()){data="{}";status=401;}
        }
        else if(path.endsWith("/sessions"))data="{}";
        else if(path.endsWith("/images")){
            const QUrlQuery query(request.url());const int page=query.queryItemValue("page").toInt();
            const int pageSize=std::min(40,query.queryItemValue("per_page").toInt());QJsonArray refs;
            for(int i=(page-1)*pageSize;i<std::min(page*pageSize,prokoCount);++i)refs.append(QJsonObject{{"image",QJsonObject{{"filename",QString::number(i)},{"download_url","https://static.proko.com/reference/"+QString::number(i)+".png"}}}});
            data=QJsonDocument(QJsonObject{{"data",refs}}).toJson();
        }else{
            ++catalogRequests;
            QNetworkCookie cookie("session","public-test");cookie.setSecure(true);cookie.setPath("/");
            cookieJar()->setCookiesFromUrl({cookie},QUrl("https://www.proko.com/timer"));
            data=R"(<script id="proko-web-state">{"a":{"u":"/api/reference_packs","b":{"data":[{"free":true,"id":"pack","title":"Test set","images_count":105}]}}}</script>)";
        }
        if(stall)delay=-1;
        auto* reply=new TestReply(request,data,status,delay,this);
        if(isImage){++active;peak=std::max(peak,active);connect(reply,&QNetworkReply::finished,this,[this,reply]{--active;if(reply->error()==QNetworkReply::OperationCanceledError)++aborted;});}
        if(stall)QTimer::singleShot(0,reply,[reply]{for(auto* timer:reply->findChildren<QTimer*>())timer->start(30);});
        return reply;
    }
};
}
void OnlineTests::cachedProkoCatalogKeepsTheAnonymousSession(){
    QTemporaryDir dir;TestNetwork catalogNetwork,downloadNetwork;downloadNetwork.requireProkoCookie=true;
    OnlineSources catalog(dir.path(),nullptr,&catalogNetwork);catalog.selectProvider("proko");QTRY_VERIFY(!catalog.busy());
    DownloadQueue queue(dir.path(),nullptr,&downloadNetwork);queue.setCatalogSource(&catalog);
    QVERIFY(queue.download("proko",catalog.sets(),2));QTRY_VERIFY_WITH_TIMEOUT(!queue.running(),5000);
    QCOMPARE(queue.completed(),2);QVERIFY2(queue.error().isEmpty(),qPrintable(queue.error()));QCOMPARE(downloadNetwork.catalogRequests,0);
}
void OnlineTests::entirePackUsesAllReferencesAndPersists(){
    QTemporaryDir dir;QString folder;
    {
        TestNetwork network;DownloadQueue queue(dir.path(),nullptr,&network);
        QVERIFY(queue.download("quickposes",{QVariantMap{{"id","pose_all_all"},{"name","Full body"}}},0));
        QTRY_VERIFY_WITH_TIMEOUT(!queue.running(),5000);QCOMPARE(queue.completed(),8);QCOMPARE(queue.requested(),8);
        QCOMPARE(queue.savedSets().size(),1);const auto saved=queue.savedSets().first().toMap();folder=saved.value("folder").toString();
        QVERIFY(saved.value("fullPack").toBool());QVERIFY(!saved.value("partial").toBool());QCOMPARE(saved.value("count").toInt(),8);
    }
    DownloadQueue restored(dir.path());QVERIFY(restored.jobs().isEmpty());QCOMPARE(restored.savedSets().size(),1);
    QCOMPARE(restored.savedSets().first().toMap().value("folder").toString(),folder);
    QVERIFY(restored.savedSets().first().toMap().value("fullPack").toBool());
}
void OnlineTests::partialPacksCanResumeAfterRestart(){
    QTemporaryDir dir,other;QString folder;
    {TestNetwork network;network.slowImages=true;DownloadQueue queue(dir.path(),nullptr,&network);
        QVERIFY(queue.download("quickposes",{QVariantMap{{"id","pose_all_all"},{"name","Full body"}}},0));
        QTRY_VERIFY_WITH_TIMEOUT(queue.completed()>0,5000);queue.cancel();
        QCOMPARE(queue.savedSets().size(),1);QVERIFY(queue.savedSets().first().toMap().value("partial").toBool());folder=queue.savedSets().first().toMap().value("folder").toString();
    }
    TestNetwork network;DownloadQueue restored(dir.path(),nullptr,&network);restored.setDestination(QUrl::fromLocalFile(other.path()));
    QVERIFY(restored.resumeSaved(0));QTRY_VERIFY_WITH_TIMEOUT(!restored.running(),5000);
    QVERIFY2(restored.error().isEmpty(),qPrintable(restored.error()));
    QCOMPARE(restored.completed(),8);
    const auto saved=restored.savedSets().first().toMap();QCOMPARE(saved.value("folder").toString(),folder);QCOMPARE(saved.value("count").toInt(),8);
    QVERIFY(saved.value("fullPack").toBool());QVERIFY(!saved.value("partial").toBool());QCOMPARE(restored.destination(),other.path());
}
void OnlineTests::prokoEntirePackReadsEveryPage(){
    QTemporaryDir dir;TestNetwork network;OnlineSources sources(dir.path(),nullptr,&network);
    sources.selectProvider("proko");QTRY_VERIFY(!sources.busy());sources.importImages("pack",0,true);
    QTRY_VERIFY_WITH_TIMEOUT(!sources.busy(),10000);
    QCOMPARE(sources.requested(),105);QCOMPARE(sources.completed(),105);QVERIFY2(sources.error().isEmpty(),qPrintable(sources.error()));
    QCOMPARE(network.attempts.value("/api/timer_tools/test-tool/images"),3);
}
void OnlineTests::randomSourcesRejectEntirePack(){
    QTemporaryDir dir;TestNetwork network;DownloadQueue queue(dir.path(),nullptr,&network);
    QVERIFY(!queue.download("sketchdaily",{QVariantMap{{"id","FullBodies"},{"name","Full body"}}},0));QVERIFY(!queue.error().isEmpty());QVERIFY(!queue.running());
    OnlineSources source(dir.path(),nullptr,&network);source.selectProvider("sketchdaily");source.importImages("FullBodies",0);QVERIFY(!source.busy());QVERIFY(!source.error().isEmpty());
}
void OnlineTests::savedSetsAreDiscoveredWithoutQueueHistory(){
    QTemporaryDir dir;TestNetwork network;
    {OnlineSources source(dir.path(),nullptr,&network);source.selectProvider("quickposes");source.importImages("face_all",2,true);QTRY_VERIFY(!source.busy());}
    DownloadQueue queue(dir.path());QVERIFY(queue.jobs().isEmpty());QCOMPARE(queue.savedSets().size(),1);QCOMPARE(queue.savedSets().first().toMap().value("count").toInt(),2);
    queue.refreshSavedSets();QCOMPARE(queue.savedSets().size(),1);
}
void OnlineTests::aNewDownloadDoesNotRestartOldJobs(){
    QTemporaryDir dir;TestNetwork network;DownloadQueue queue(dir.path(),nullptr,&network);
    QVERIFY(queue.add("quickposes",{QVariantMap{{"id","face_all"},{"name","Faces"}}},2,true));
    QVERIFY(queue.download("quickposes",{QVariantMap{{"id","hands_all"},{"name","Hands"}}},2));
    QTRY_VERIFY_WITH_TIMEOUT(!queue.running(),5000);QCOMPARE(queue.jobs().size(),1);QCOMPARE(queue.jobs().first().toMap().value("setId").toString(),QString("hands_all"));
    QCOMPARE(network.attempts.value("/ajax/getPosesForSlider/face_all"),0);
}
void OnlineTests::practiceStartsOnlyForItsJob(){
    QTemporaryDir dir;TestNetwork network;DownloadQueue queue(dir.path(),nullptr,&network);
    const QVariantMap first{{"id","face_all"},{"name","Faces"}},selected{{"id","pose_all_all"},{"name","Full body"}};
    QVERIFY(queue.add("quickposes",{first,selected},2,true));
    QSignalSpy ready(&queue,&DownloadQueue::practiceReady);
    QVERIFY(queue.practice("quickposes",selected,2));
    QTRY_VERIFY_WITH_TIMEOUT(!queue.running(),5000);QCOMPARE(ready.size(),1);QCOMPARE(queue.jobs().size(),2);
    QCOMPARE(ready.first().first().toString(),queue.jobs().last().toMap().value("folder").toString());
    TestNetwork slow;slow.stall=true;DownloadQueue stopped(dir.path(),nullptr,&slow);QSignalSpy cancelled(&stopped,&DownloadQueue::practiceReady);
    QVERIFY(stopped.practice("quickposes",selected,2));stopped.cancel();QTest::qWait(100);QCOMPARE(cancelled.size(),0);
}
void OnlineTests::downloadsRunInParallelAndRetry(){
    QTemporaryDir dir;TestNetwork network;OnlineSources sources(dir.path(),nullptr,&network);
    sources.selectProvider("quickposes");QSignalSpy imported(&sources,&OnlineSources::imported);sources.importImages("pose_all_all",8,true);
    QTRY_COMPARE_WITH_TIMEOUT(imported.size(),1,5000);
    QCOMPARE(sources.completed(),8);QVERIFY(sources.error().isEmpty());QCOMPARE(network.peak,4);QCOMPARE(network.active,0);
    QCOMPARE(network.attempts.value("/assets/poses/0.png"),2);
    const auto folder=imported.first().first().toString();QFile manifest(folder+"/sources.json");QVERIFY(manifest.open(QIODevice::ReadOnly));
    const auto records=QJsonDocument::fromJson(manifest.readAll()).object();QCOMPARE(records.size(),8);
    for(const auto& record:records)QCOMPARE(record.toObject().value("credit").toString(),QString("Test author"));
}
void OnlineTests::cancelAbortsAllImagesAndKeepsCompleted(){
    QTemporaryDir dir;TestNetwork network;network.slowImages=true;OnlineSources sources(dir.path(),nullptr,&network);
    sources.selectProvider("quickposes");QSignalSpy imported(&sources,&OnlineSources::imported);sources.importImages("pose_all_all",8,true);
    QTRY_VERIFY(sources.completed()>0);const auto count=sources.completed();sources.cancel();
    QVERIFY(!sources.busy());QCOMPARE(sources.completed(),count);QCOMPARE(imported.size(),1);QCOMPARE(network.active,0);QVERIFY(network.aborted>0);
    QTest::qWait(100);QCOMPARE(sources.completed(),count);
}
void OnlineTests::stalledRequestsHaveAnAbsoluteDeadline(){
    QTemporaryDir dir;TestNetwork network;network.stall=true;OnlineSources sources(dir.path(),nullptr,&network);
    sources.selectProvider("quickposes");sources.importImages("pose_all_all",2,true);
    QTRY_VERIFY_WITH_TIMEOUT(!sources.busy(),3000);QVERIFY(!sources.error().isEmpty());QCOMPARE(sources.completed(),0);
    QCOMPARE(network.attempts.value("/ajax/getPosesForSlider/pose_all_all"),2);
}
void OnlineTests::catalogIsReusedUntilRefresh(){
    QTemporaryDir dir;TestNetwork network;OnlineSources sources(dir.path(),nullptr,&network);
    sources.selectProvider("proko");QTRY_VERIFY(!sources.busy());QCOMPARE(sources.sets().size(),1);QCOMPARE(network.catalogRequests,1);
    sources.selectProvider("quickposes");sources.selectProvider("proko");QVERIFY(!sources.busy());QCOMPARE(network.catalogRequests,1);
    sources.selectProvider("proko",true);QTRY_VERIFY(!sources.busy());QCOMPARE(network.catalogRequests,2);
}
void OnlineTests::supportedSites(){
        for(const auto& id:QStringList{"quickposes","proko","poselibrary","sketchdaily"}){const auto url=OnlineSources::website(id);QVERIFY(OnlineSources::allowedUrl(url));}
        QVERIFY(OnlineSources::website("unknown").isEmpty());
}
void OnlineTests::linksStayOnTrustedHttpsHosts(){
        for(const auto& url:QStringList{"http://quickposes.com/a.jpg","file:///C:/private","https://quickposes.com.evil.example/a","https://evilquickposes.com/a","https://user:pass@proko.com/a","https://proko.com:8080/a","https://127.0.0.1/a"})QVERIFY2(!OnlineSources::allowedUrl(QUrl(url)),qPrintable(url));
        QVERIFY(OnlineSources::allowedUrl(QUrl("https://static.proko.com/image.jpg")));
        QVERIFY(OnlineSources::allowedUrl(QUrl("https://files.sketchdaily.net/references/a.jpg")));
}
void OnlineTests::quickposesRejectsInvalidAndDuplicateEntries(){
        const auto refs=OnlineSources::parseReferences("quickposes",R"([{"id":1,"src":"pose.jpg","contributor":"Photographer"},{"id":1,"src":"duplicate.jpg"},{"id":2,"src":"../private"},{"id":3,"src":"https://evil.example/a.jpg"},{"src":"missing-id.jpg"}])");
        QCOMPARE(refs.size(),1);QCOMPARE(refs.first().credit,QString("Photographer"));QCOMPARE(refs.first().url,QUrl("https://quickposes.com/assets/poses/pose.jpg"));
}
void OnlineTests::prokoUsesFullDownloadInsteadOfCover(){
        const auto refs=OnlineSources::parseReferences("proko",R"({"data":[{"id":"1","pack_title":"Free sampler","image":{"filename":"pose","download_url":"https://static.proko.com/reference/pose.jpg"}},{"image":{"filename":"bad","is_invalid":true,"download_url":"https://static.proko.com/bad.jpg"}},{"image":{"filename":"foreign","download_url":"https://evil.example/a.jpg"}}]})");
        QCOMPARE(refs.size(),1);QCOMPARE(refs.first().id,QString("pose"));QCOMPARE(refs.first().credit,QString("Free sampler"));
}
void OnlineTests::poseLibraryUsesFullImage(){
        const auto refs=OnlineSources::parseReferences("poselibrary",R"({"images":[{"poseId":"id","filePath":"gallery/pose.jpg","thumbSrc":"https://sajypjdlmwfemphcaqvi.supabase.co/thumb.jpg","src":"https://sajypjdlmwfemphcaqvi.supabase.co/full.jpg?token=public-test"}]})");
        QCOMPARE(refs.size(),1);QCOMPARE(refs.first().url.path(),QString("/full.jpg"));
}
void OnlineTests::sketchDailyKeepsCreditAndRejectsTraversal(){
        const auto refs=OnlineSources::parseReferences("sketchdaily",R"({"id":"id","file":"/images/pose.jpg","photographer":{"name":"Author"}})");
        QCOMPARE(refs.size(),1);QCOMPARE(refs.first().credit,QString("Author"));QCOMPARE(refs.first().url.host(),QString("files.sketchdaily.net"));
        QVERIFY(OnlineSources::parseReferences("sketchdaily",R"({"id":"id","file":"/../private"})").isEmpty());
}
void OnlineTests::malformedRepliesAreEmpty(){
        for(const auto& id:QStringList{"quickposes","proko","poselibrary","sketchdaily"}){QVERIFY(OnlineSources::parseReferences(id,"invalid JSON").isEmpty());QVERIFY(OnlineSources::parseReferences(id,"{}").isEmpty());}
}
void OnlineTests::invalidActionsDoNotStartRequests(){
        QTemporaryDir dir;OnlineSources sources(dir.path());QSignalSpy imported(&sources,&OnlineSources::imported);
        sources.selectProvider("unknown");QVERIFY(!sources.busy());sources.selectProvider("quickposes");QCOMPARE(sources.sets().size(),7);
        sources.importImages("not-a-set",20);QVERIFY(!sources.busy());sources.cancel();QCOMPARE(imported.size(),0);
}
QTEST_GUILESS_MAIN(OnlineTests)

void OnlineTests::batchQueueLimitsAreAtomic(){
    QTemporaryDir dir;DownloadQueue queue(dir.path());QVariantList sets;
    for(int i=0;i<6;++i)sets.append(QVariantMap{{"id",QString::number(i)},{"name","Set"}});
    QVERIFY(!queue.add("quickposes",sets,100,true));QVERIFY(queue.jobs().isEmpty());
    QVERIFY(queue.add("quickposes",sets.mid(0,5),100,true));QCOMPARE(queue.requested(),500);
    QVERIFY(!queue.add("proko",sets.mid(0,1),1,false));QCOMPARE(queue.jobs().size(),5);QCOMPARE(queue.requested(),500);
}
void OnlineTests::batchQueueDeduplicatesAndRemoves(){
    QTemporaryDir dir;DownloadQueue queue(dir.path());QVariantList sets{QVariantMap{{"id","pose_all_all"},{"name","Full body"}}};
    QVERIFY(queue.add("quickposes",sets,5,true));QVERIFY(!queue.add("quickposes",sets,5,true));QCOMPARE(queue.jobs().size(),1);
    QVERIFY(!queue.add("unknown",sets,5,false));QVERIFY(queue.jobs().first().toMap().value("separate").toBool());
    queue.remove(9);QCOMPARE(queue.jobs().size(),1);queue.remove(0);QVERIFY(queue.jobs().isEmpty());queue.clear();QCOMPARE(queue.completed(),0);
}
void OnlineTests::downloadDestinationPersists(){
    QTemporaryDir profile,target;
    {DownloadQueue queue(profile.path());queue.setDestination(QUrl::fromLocalFile(target.path()));QCOMPARE(queue.destination(),QDir::cleanPath(target.path()));queue.setDestination(QUrl("https://evil.example"));QCOMPARE(queue.destination(),QDir::cleanPath(target.path()));}
    DownloadQueue restored(profile.path());QCOMPARE(restored.destination(),QDir::cleanPath(target.path()));
}
void OnlineTests::cancellingBeforeFirstRequestDoesNotResume(){
    QTemporaryDir dir;DownloadQueue queue(dir.path());QSignalSpy imported(&queue,&DownloadQueue::imported);
    QVERIFY(queue.add("quickposes",{QVariantMap{{"id","pose_all_all"},{"name","Full body"}}},2,true));
    queue.start();QVERIFY(queue.running());queue.cancel();QCoreApplication::processEvents();
    QVERIFY(!queue.running());QCOMPARE(queue.completed(),0);QCOMPARE(imported.size(),0);QCOMPARE(queue.jobs().first().toMap().value("status").toString(),QString("Stopped"));
    queue.clear();QVERIFY(queue.jobs().isEmpty());
}

void OnlineTests::filtersUseProviderValues(){
    const QVariantMap values{{"gender","female"},{"clothing","clothed"},{"pose","stationary"},{"angle","front"}};
    QCOMPARE(OnlineSources::quickposesDescriptor("pose_all_all",values),QString("pose_female_non-nude"));
    QCOMPARE(OnlineSources::quickposesDescriptor("hands_all",values),QString("hands_female"));
    QCOMPARE(OnlineSources::quickposesDescriptor("animals",values),QString("animals"));
    QCOMPARE(OnlineSources::quickposesDescriptor("pose_all_all",{{"clothing","nude"}}),QString("pose_all_nude & semi-nude"));
    const QUrlQuery query(OnlineSources::sketchUrl(values));
    QCOMPARE(query.queryItemValue("Gender"),QString("Female"));QCOMPARE(query.queryItemValue("Clothing"),QString("true"));
    QCOMPARE(query.queryItemValue("PoseType"),QString("Stationary"));QCOMPARE(query.queryItemValue("ViewAngle"),QString("Front"));
    QVERIFY(OnlineSources::normalizedFilters("proko",values).isEmpty());
    QVERIFY(OnlineSources::normalizedFilters("sketchdaily",{{"gender","invalid"},{"pose","../bad"},{"unknown",true}}).isEmpty());
    QVERIFY(QUrlQuery(OnlineSources::sketchUrl({})).isEmpty());
}
void OnlineTests::queueKeepsDifferentFilters(){
    QTemporaryDir dir;DownloadQueue queue(dir.path());const QVariantList sets{QVariantMap{{"id","pose_all_all"},{"name","Full body"}}};
    QVERIFY(queue.add("quickposes",sets,5,true,{{"gender","male"}}));
    QVERIFY(queue.add("quickposes",sets,5,true,{{"gender","female"}}));
    QVERIFY(!queue.add("quickposes",sets,5,true,{{"gender","female"}}));
    QCOMPARE(queue.jobs().size(),2);QCOMPARE(queue.jobs().first().toMap().value("filters").toMap().value("gender").toString(),QString("male"));
}
