#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QImage>
#include <QSqlQuery>
#include "media/catalog.h"
#include "media/collections.h"
#include "media/image_provider.h"
#include "storage/store.h"
#include "storage/practice_stats.h"
#include "ui/themes.h"
#include "ui/localization.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QColor>
#include <QBuffer>
#include <cmath>

class IntegrationTests : public QObject {
    Q_OBJECT
private slots:
    void supportedCatalogsCompleteAndRetiredResourcesAbsent(){
        const auto read=[](const QString& path){QFile f(path);if(!f.open(QIODevice::ReadOnly))return QJsonObject{};return QJsonDocument::fromJson(f.readAll()).object();};
        const auto source=read(":/assets/locales-v2/en.json");QVERIFY(source.size()>=300);
        QFile list(":/tools/localization/targets.json");QVERIFY(list.open(QIODevice::ReadOnly));const auto targets=QJsonDocument::fromJson(list.readAll()).array();QCOMPARE(targets.size(),20);
        Localization localization;
        for(const auto& entry:targets){
            const auto lang=entry.toString();const auto catalog=read(":/assets/locales-v2/"+lang+".json");
            QCOMPARE(catalog.size(),source.size());localization.setLanguage(lang);QCOMPARE(localization.language(),lang);
            for(auto it=source.begin();it!=source.end();++it){QVERIFY2(catalog.contains(it.key()),qPrintable(lang+": "+it.key()));QVERIFY(catalog.value(it.key()).isString());QVERIFY(!catalog.value(it.key()).toString().trimmed().isEmpty());QCOMPARE(localization.s(it.key()),catalog.value(it.key()).toString());}
        }
        const auto names=read(":/assets/languages.json");QCOMPARE(names.size(),22);QCOMPARE(localization.languages().size(),22);
        for(auto it=names.begin();it!=names.end();++it){
            const auto catalog=read(":/assets/locales-v2/"+it.key()+".json");QCOMPARE(catalog.size(),source.size());
            localization.setLanguage(it.key());QCOMPARE(localization.language(),it.key());
            for(auto key=source.begin();key!=source.end();++key){QVERIFY(catalog.value(key.key()).isString());QVERIFY(!catalog.value(key.key()).toString().trimmed().isEmpty());}
        }
        QVERIFY(!QFile::exists(":/assets/locales/en.json"));QVERIFY(!QFile::exists(":/assets/locales/af.json"));
        QVERIFY(!QFile::exists(":/assets/locales-extra/ru.json"));
        localization.setLanguage("ar");QVERIFY(localization.rtl());
        localization.setLanguage("af");QCOMPARE(localization.language(),QString("ru"));QVERIFY(!localization.rtl());
        QCOMPARE(localization.s("nonexistent-key"),QString("nonexistent-key"));
        localization.setLanguage("ru");QCOMPARE(localization.s("Manage collections"),QString::fromUtf8("Управление коллекциями"));
        localization.setLanguage("invalid-locale");QCOMPARE(localization.language(),QString("ru"));
    }
    void localizedIntegerPluralCategories(){
        const auto read=[](const QString& path){QFile f(path);f.open(QIODevice::ReadOnly);return QJsonDocument::fromJson(f.readAll()).object();};
        struct Example {const char* locale;int number;const char* category;};
        const Example examples[]={{"ru",1,"one"},{"ru",2,"few"},{"ru",11,"many"},{"ru",21,"one"},{"uk",22,"few"},{"pl",21,"many"},{"pl",22,"few"},{"cs",4,"few"},{"cs",5,"other"},{"ro",0,"few"},{"ro",20,"other"},{"fr",0,"one"},{"pt-BR",0,"one"},{"ar",0,"zero"},{"ar",1,"one"},{"ar",2,"two"},{"ar",7,"few"},{"ar",25,"many"},{"ar",100,"other"},{"ja",1,"other"},{"tr",2,"other"},{"de",1,"one"},{"de",2,"other"}};
        Localization localization;
        for(const auto& example:examples){localization.setLanguage(example.locale);const auto catalog=read(":/assets/locales-v2/"+QString(example.locale)+".json");const auto word=catalog.value("_plural.image."+QString(example.category)).toString();QVERIFY(!word.isEmpty());QCOMPARE(localization.count(example.number,"image","images_few","images"),QString::number(example.number)+" "+word);}
    }
    void damagedMiddlePageRecoversReadableTail(){
        QTemporaryDir dir;QVERIFY(QFile::copy(QStringLiteral(GESTART_FIXTURE_DIR "/corrupt-middle.sqlite"),dir.filePath("gestart.sqlite")));
        Store store(dir.path());QSignalSpy recovery(&store,&Store::recovered);QSignalSpy snapshots(&store,&Store::snapshot);
        QTRY_VERIFY_WITH_TIMEOUT(!recovery.isEmpty(),10000);QCOMPARE(recovery.first()[0].toInt(),583);
        QTRY_VERIFY_WITH_TIMEOUT(!snapshots.isEmpty(),5000);QCOMPARE(snapshots.last()[0].toList().size(),583);
        QVERIFY(QFile::exists(recovery.first()[1].toString()+"/gestart.sqlite"));
        QVERIFY(QFile::exists(recovery.first()[1].toString()+"/recovery-report.json"));
        store.record("after-repair.png",{},"completed",1000);store.refresh();
        QTRY_COMPARE_WITH_TIMEOUT(snapshots.size(),2,5000);QCOMPARE(snapshots.last()[0].toList().size(),584);
    }
    void browsingFiltersDoNotChangeSessionSources(){
        CatalogModel model;CatalogFilter filter;filter.setSourceModel(&model);filter.setFilterRole(CatalogModel::NameRole);
        model.append({{"/a/pose.jpg","pose.jpg","",1},{"/b/pose.jpg","pose.jpg","",1},{"/b/hand.jpg","hand.jpg","",1}});
        model.markCompleted("/b/pose.jpg");filter.setAllowedPaths({"/b/pose.jpg","/b/hand.jpg"},true);QCOMPARE(filter.rowCount(),2);
        filter.setCompletedOnly(true);QCOMPARE(filter.rowCount(),1);filter.setFilterFixedString("hand");QCOMPARE(filter.rowCount(),0);
        filter.setCompletedOnly(false);QCOMPARE(filter.rowCount(),1);QCOMPARE(model.rowCount(),3);
        filter.setAllowedPaths({},false);filter.setFilterFixedString("");QCOMPARE(filter.rowCount(),3);
    }
    void damagedIndexRecoveredWithOriginalBackup(){
        QTemporaryDir dir;const auto file=dir.filePath("gestart.sqlite");int page=0;
        {Store store(dir.path());QSignalSpy snapshot(&store,&Store::snapshot);QTRY_VERIFY_WITH_TIMEOUT(!snapshot.isEmpty(),5000);
            store.record("reference.png",QString{},"shown");store.record("reference.png",QString{},"completed",900);store.refresh();QTRY_VERIFY_WITH_TIMEOUT(snapshot.size()>=2,5000);}
        const QString connection="corruption-test";
        {auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(file);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("SELECT rootpage FROM sqlite_master WHERE name='events_root'"));QVERIFY(q.next());page=q.value(0).toInt();db.close();}
        QSqlDatabase::removeDatabase(connection);
        QFile damaged(file);QVERIFY(damaged.open(QIODevice::ReadWrite));QVERIFY(damaged.seek((page-1)*4096));QCOMPARE(damaged.write(QByteArray(16,'\0')),qint64(16));damaged.close();
        Store repaired(dir.path());QSignalSpy recovered(&repaired,&Store::recovered);QSignalSpy snapshots(&repaired,&Store::snapshot);QSignalSpy errors(&repaired,&Store::error);
        QTRY_VERIFY_WITH_TIMEOUT(!recovered.isEmpty()||!errors.isEmpty(),5000);
        QVERIFY2(!recovered.isEmpty(),errors.isEmpty()?"No recovery notification":qPrintable(errors.last()[0].toString()));QCOMPARE(recovered.first()[0].toInt(),2);QVERIFY(QFile::exists(recovered.first()[1].toString()+"/gestart.sqlite"));
        QTRY_VERIFY_WITH_TIMEOUT(!snapshots.isEmpty(),5000);QCOMPARE(snapshots.last()[2].toMap().value("completed").toInt(),1);QCOMPARE(errors.size(),0);
        repaired.record("after.png",QString{},"shown");repaired.refresh();QTRY_VERIFY_WITH_TIMEOUT(snapshots.size()>=2,5000);QCOMPARE(snapshots.last()[0].toList().size(),3);QCOMPARE(errors.size(),0);
    }
    void iconsHandleEmptyRequestedSize(){
        IconProvider provider;QSize size;
        QVERIFY(!provider.requestImage("home/ff00ff",&size,{0,0}).isNull());QCOMPARE(size,QSize(24,24));
        QVERIFY(!provider.requestImage("home/ff00ff",&size,{40,40}).isNull());QCOMPARE(size,QSize(40,40));
        QVERIFY(!provider.requestImage("home/ff00ff",&size,{2048,2048}).isNull());QCOMPARE(size,QSize(512,512));
    }
    void exifRotationFitsRequestedBounds(){
        QTemporaryDir dir;QImage raw(60,100,QImage::Format_RGB32);raw.fill(Qt::green);
        QByteArray jpeg;QBuffer buffer(&jpeg);QVERIFY(buffer.open(QIODevice::WriteOnly));QVERIFY(raw.save(&buffer,"JPEG"));
        const QByteArray orientation=QByteArray::fromHex("ffe1002245786966000049492a0008000000010012010300010000000600000000000000");
        jpeg.insert(2,orientation);QFile file(dir.filePath("rotated.jpg"));QVERIFY(file.open(QIODevice::WriteOnly));file.write(jpeg);file.close();
        QCOMPARE(imageNaturalSize(file.fileName()),QSize(100,60));
        QString error;const auto image=decodeImage(file.fileName(),{50,30},&error);QVERIFY2(!image.isNull(),qPrintable(error));QCOMPARE(image.size(),QSize(50,30));
    }
    void practicePeriodsStreaksAndGoals(){
        const QDate today(2026,10,12);const QVector<PracticeDay> rows{{today,3,900000},{today.addDays(-1),2,600000},{today.addDays(-2),1,300000},{today.addDays(-40),4,1200000},{today.addDays(1),99,9999999}};
        auto p=summarizePractice(rows,today,today,7,15,3,"both");
        QCOMPARE(p.value("totalStudies").toInt(),6);QCOMPARE(p.value("totalMs").toLongLong(),1800000);QCOMPARE(p.value("activeDays").toInt(),3);
        QCOMPARE(p.value("allTimeStudies").toInt(),10);QCOMPARE(p.value("currentStreak").toInt(),3);QCOMPARE(p.value("bestStreak").toInt(),3);
        QVERIFY(p.value("goalReached").toBool());QCOMPARE(p.value("calendar").toList().size(),42);QCOMPARE(p.value("week").toList().size(),7);
        p=summarizePractice(rows,today,today,30,30,2,"both");QCOMPARE(p.value("goalProgress").toDouble(),.5);
        p=summarizePractice(rows,today,today,0,15,5,"studies");QCOMPARE(p.value("totalStudies").toInt(),10);QCOMPARE(p.value("goalProgress").toDouble(),.6);
        p=summarizePractice(rows,today,today,90,15,5,"time");QCOMPARE(p.value("totalStudies").toInt(),10);QCOMPARE(p.value("chart").toList().size(),90);
        p=summarizePractice(rows,today.addDays(2),today,7,15,3,"time");QCOMPARE(p.value("currentStreak").toInt(),4);
        p=summarizePractice(rows,today.addDays(1),QDate(2026,9,1),7,15,3,"time");QCOMPARE(p.value("currentStreak").toInt(),4);
        p=summarizePractice({},today,today,7,15,10,"time");QCOMPARE(p.value("goalProgress").toDouble(),0.0);QCOMPARE(p.value("chart").toList().size(),7);QCOMPARE(p.value("currentStreak").toInt(),0);
    }
    void practiceStoreIgnoresShownSkippedAndImported(){
        QTemporaryDir dir;Store store(dir.path());QSignalSpy snapshots(&store,&Store::snapshot);QSignalSpy stats(&store,&Store::analyticsReady);
        QTRY_VERIFY_WITH_TIMEOUT(!snapshots.isEmpty(),5000);
        store.record(dir.filePath("refs/completed.png"),"refs","completed",900000);store.record(dir.filePath("refs/shown.png"),"refs","shown");store.record(dir.filePath("refs/skip.png"),"refs","skipped");
        store.analytics(42,7,QDate::currentDate(),15,1,"time");QTRY_COMPARE_WITH_TIMEOUT(stats.size(),1,5000);
        QCOMPARE(stats.last()[0].toULongLong(),quint64(42));const auto p=stats.last()[1].toMap();QCOMPARE(p.value("todayStudies").toInt(),1);QCOMPARE(p.value("todayMs").toLongLong(),900000);QVERIFY(p.value("goalReached").toBool());
        store.clearSources({dir.filePath("refs")});store.analytics(43,7,QDate::currentDate(),15,1,"time");QTRY_COMPARE_WITH_TIMEOUT(stats.size(),2,5000);QCOMPARE(stats.last()[1].toMap().value("totalStudies").toInt(),0);
    }
    void collectionValidationAndSelection(){
        QTemporaryDir dir;const auto first=makeCollection({dir.filePath("refs")},"folder","  Practice  ");
        auto second=makeCollection({dir.filePath("other")},"folder","Other");second["selected"]=false;
        auto files=makeCollection({dir.filePath("b.png"),dir.filePath("a.png")},"files","Images");
        QCOMPARE(files.value("id"),makeCollection({dir.filePath("a.png"),dir.filePath("b.png")},"files","Renamed").value("id"));
        const auto normalized=normalizeCollections({first,first,second,files,QVariantMap{{"paths",QStringList{"relative"}}}});
        QCOMPARE(normalized.size(),3);QCOMPARE(normalized.first().toMap().value("name").toString(),QString("Practice"));
        QCOMPARE(collectionPaths(normalized).size(),4);QCOMPARE(collectionPaths(normalized,true).size(),3);
        QVERIFY(pathInSources(dir.filePath("refs/nested/a.jpg"),{dir.filePath("refs")}));
        QVERIFY(!pathInSources(dir.filePath("refs-extra/a.jpg"),{dir.filePath("refs")}));
        QVERIFY(!pathInSources(dir.filePath("a.png.extra"),{dir.filePath("a.png")}));
        QVERIFY(pathInSources(dir.filePath("a.png"),{dir.filePath("a.png")}));
    }
    void completionBadgesFollowPaths(){
        CatalogModel model;model.setCompleted({"/refs/a.png"});
        model.append({{"/refs/a.png","a.png",{},1},{"/refs/b.png","b.png",{},1},{"/refs-other/c.png","c.png",{},1}});
        QVERIFY(model.data(model.index(0),CatalogModel::CompletedRole).toBool());
        QVERIFY(!model.data(model.index(1),CatalogModel::CompletedRole).toBool());
        QSignalSpy changed(&model,&CatalogModel::dataChanged);model.markCompleted("/refs/b.png");QCOMPARE(changed.size(),1);
        QCOMPARE(model.pathsForSources({"/refs"}).size(),2);
        model.removePath("/refs/a.png");QVERIFY(model.data(model.index(0),CatalogModel::CompletedRole).toBool());
        model.clear();model.append({{"/refs/b.png","b.png",{},1}});QVERIFY(model.data(model.index(0),CatalogModel::CompletedRole).toBool());
        model.setCompleted({});QVERIFY(!model.data(model.index(0),CatalogModel::CompletedRole).toBool());
    }
    void completedBadgesPersistAndClearSelectedSources(){
        QTemporaryDir dir;const QString a=dir.filePath("refs/a.png"),b=dir.filePath("refs-other/b.png"),c=dir.filePath("refs/c.png");
        {
            Store store(dir.path());QSignalSpy badges(&store,&Store::completedPaths);
            QTRY_VERIFY_WITH_TIMEOUT(!badges.isEmpty(),5000);
            store.record(a,"selection:one","completed",100);store.record(b,"selection:two","completed",100);
            store.record(c,"selection:one","shown");store.record(c,"selection:one","skipped");store.refresh();
            QTRY_VERIFY_WITH_TIMEOUT(badges.last()[0].toStringList().size()==2,5000);
            QVERIFY(!badges.last()[0].toStringList().contains(c));
            store.savePreferences({{"collections",QVariantList{makeCollection({dir.filePath("refs")},"folder","My references")}}});
        }
        QCOMPARE(loadPreferences(dir.path()).value("collections").toList().size(),1);
        Store reopened(dir.path());QSignalSpy badges(&reopened,&Store::completedPaths);
        QTRY_VERIFY_WITH_TIMEOUT(!badges.isEmpty(),5000);QCOMPARE(badges.last()[0].toStringList().size(),2);
        reopened.clearSources({dir.filePath("refs")});
        QTRY_VERIFY_WITH_TIMEOUT(badges.last()[0].toStringList().size()==1,5000);QCOMPARE(badges.last()[0].toStringList().first(),b);
    }
    void themeMigrationAndValidation(){
        QTemporaryDir dir;QFile file(dir.filePath("settings.json"));QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{\"theme\":\"charoite\"}");file.close();
        auto p=loadPreferences(dir.path());QCOMPARE(p.value("theme_base").toString(),QString("black"));
        QCOMPARE(p.value("theme_accent").toString(),QString("charoite"));QCOMPARE(p.value("schema").toInt(),2);
        QVERIFY(file.open(QIODevice::WriteOnly));file.write("{\"theme\":\"calcite\"}");file.close();
        p=loadPreferences(dir.path());QCOMPARE(p.value("theme_base").toString(),QString("white"));QCOMPARE(p.value("theme_white_tone").toString(),QString("paper"));
        p["theme_accent_group"]="lobe";p["theme_accent"]="blue";p["theme_black_tone"]="ink";normalizeTheme(p);
        QCOMPARE(p.value("theme_base").toString(),QString("white"));QCOMPARE(p.value("theme_white_tone").toString(),QString("paper"));
        QVERIFY(QColor(themePalette(p,false).value("accent").toString()).isValid());
        p["theme_base"]="bad";p["theme_black_tone"]="bad";p["theme_accent_group"]="bad";p["theme_accent"]="bad";normalizeTheme(p);
        QCOMPARE(p.value("theme_base").toString(),QString("black"));QCOMPARE(p.value("theme_black_tone").toString(),QString("neutral"));
        QVERIFY(QColor(themePalette(p,true).value("accent").toString()).isValid());
    }
    void paletteContrast(){
        auto luminance=[](const QColor& c){auto linear=[](double v){return v<=0.04045?v/12.92:std::pow((v+0.055)/1.055,2.4);};return .2126*linear(c.redF())+.7152*linear(c.greenF())+.0722*linear(c.blueF());};
        auto ratio=[&](const QString& a,const QString& b){const double x=luminance(QColor(a)),y=luminance(QColor(b));return (std::max(x,y)+.05)/(std::min(x,y)+.05);};
        const auto catalog=themeCatalog();QCOMPARE(catalog.value("stones").toList().size(),12);QCOMPARE(catalog.value("lobe").toList().size(),12);
        for(const QString base:{QString("white"),QString("black")}){
            const auto tones=catalog.value(base).toList();QCOMPARE(tones.size(),5);
            for(const auto& value:tones){const auto colors=value.toMap().value("colors").toMap();
                for(const QString field:{QString("text"),QString("muted"),QString("subtle")})QVERIFY2(ratio(colors.value(field).toString(),colors.value("panel").toString())>=4.5,qPrintable(base+" "+field));
                for(const QString group:{QString("lobe"),QString("stones")})for(const auto& color:catalog.value(group).toList())
                    QVERIFY2(ratio(color.toMap().value(base+"Text").toString(),colors.value("panel").toString())>=4.5,qPrintable(color.toMap().value("key").toString()));
            }
        }
        for(const auto& value:catalog.value("stones").toList()){const auto color=value.toMap();QVERIFY(QColor(color.value("color").toString()).hsvSaturationF()>.5);for(const QString base:{QString("white"),QString("black")})QVERIFY(ratio(color.value(base).toString(),color.value(base+"Ink").toString())>=4.5);}
    }
    void grayscalePreservesAlpha(){
        QImage image(2,1,QImage::Format_ARGB32);image.setPixel(0,0,qRgba(210,30,40,77));image.setPixel(1,0,qRgba(0,0,0,0));
        const auto gray=grayscaleImage(image);QCOMPARE(qAlpha(gray.pixel(0,0)),77);QCOMPARE(qAlpha(gray.pixel(1,0)),0);
        QCOMPARE(qRed(gray.pixel(0,0)),69);QCOMPARE(qRed(gray.pixel(0,0)),qGreen(gray.pixel(0,0)));
    }
    void translucentTextContrast(){
        auto luminance=[](const QColor& c){auto linear=[](double v){return v<=.04045?v/12.92:std::pow((v+.055)/1.055,2.4);};return .2126*linear(c.redF())+.7152*linear(c.greenF())+.0722*linear(c.blueF());};
        for(const auto& base:QStringList{"white","black"})for(const auto& option:themeCatalog().value(base).toList()){
            const auto palette=option.toMap().value("colors").toMap();const QColor panel(palette.value("panel").toString()),text(palette.value("text").toString());
            for(const double alpha:{.7,.78,.95})for(const double image:{0.0,1.0}){
                const QColor mixed=QColor::fromRgbF(alpha*panel.redF()+(1-alpha)*image,alpha*panel.greenF()+(1-alpha)*image,alpha*panel.blueF()+(1-alpha)*image);
                const double x=luminance(text),y=luminance(mixed);QVERIFY((std::max(x,y)+.05)/(std::min(x,y)+.05)>=4.5);
            }
        }
    }
#ifdef GESTART_HAS_HEIF
    void heifColorsAndUnicode(){
        QTemporaryDir dir;const QString path=dir.filePath("цвета.heic");
        QVERIFY(QFile::copy(QStringLiteral(GESTART_FIXTURE_DIR "/colors.heic"),path));
        QCOMPARE(imageNaturalSize(path),QSize(256,192));QString error;
        const auto image=decodeImage(path,{128,96},&error);QVERIFY2(!image.isNull(),qPrintable(error));
        QCOMPARE(image.size(),QSize(128,96));
        const auto red=image.pixelColor(25,25),green=image.pixelColor(100,25),blue=image.pixelColor(25,75);
        QVERIFY(qAbs(red.red()-210)<8 && qAbs(red.green()-30)<8 && qAbs(red.blue()-40)<8);
        QVERIFY(qAbs(green.red()-30)<8 && qAbs(green.green()-200)<8 && qAbs(green.blue()-40)<8);
        QVERIFY(qAbs(blue.red()-20)<8 && qAbs(blue.green()-30)<8 && qAbs(blue.blue()-200)<8);
    }
#endif
    void byteCacheAndInvalidation(){
        ImageCache cache;cache.limit=3000;
        QImage image(10,10,QImage::Format_ARGB32);image.fill(Qt::red);
        for(int i=0;i<20;++i)cache.put(QString::number(i)+"\nsize",image);
        QVERIFY(cache.bytes()<=cache.limit);QVERIFY(cache.get("0\nsize").isNull());
        QVERIFY(!cache.get("19\nsize").isNull());
        cache.invalidate("19");QVERIFY(cache.get("19\nsize").isNull());
        cache.clear();QCOMPARE(cache.bytes(),qint64(0));
        cache.put("new\nsize",image);QCOMPARE(cache.bytes(),qint64(400));
        QImage huge(100,100,QImage::Format_ARGB32);cache.put("huge",huge);
        QCOMPARE(cache.bytes(),qint64(400));
    }
    void sizedDecodeAndSvgAspect(){
        QTemporaryDir dir;QVERIFY(dir.isValid());
        QImage original(2000,1000,QImage::Format_RGB32);original.fill(Qt::blue);
        const QString path=dir.filePath("кириллица.png");QVERIFY(original.save(path));
        QString error;const auto image=decodeImage(path,{200,200},&error);
        QVERIFY2(!image.isNull(),qPrintable(error));QCOMPARE(image.size(),QSize(200,100));
        QFile svg(dir.filePath("test.svg"));QVERIFY(svg.open(QIODevice::WriteOnly));
        svg.write("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"800\" height=\"400\"><rect width=\"800\" height=\"400\" fill=\"red\"/></svg>");svg.close();
        QCOMPARE(decodeImage(svg.fileName(),{200,200}).size(),QSize(200,100));
        QVERIFY(!decodeImage(svg.fileName(),{0,0}).isNull()); // transient zero-sized layouts are not bad files
        QVERIFY(decodeImage(dir.filePath("missing.png"),{200,200},&error).isNull());QVERIFY(!error.isEmpty());
    }
    void scanningAndCancellation(){
        QTemporaryDir dir;QVERIFY(dir.isValid());
        QDir().mkpath(dir.filePath("nested"));
        QImage image(2,2,QImage::Format_RGB32);image.fill(Qt::green);
        QVERIFY(image.save(dir.filePath("a.png")));QVERIFY(image.save(dir.filePath("nested/b.jpg")));
        CatalogScanner scanner;QSignalSpy finished(&scanner,&CatalogScanner::finished);
        int batchCount=0;connect(&scanner,&CatalogScanner::batch,this,[&](quint64,QVector<MediaEntry> entries){batchCount+=entries.size();});
        const auto generation=scanner.scan({dir.path()});
        QTRY_COMPARE_WITH_TIMEOUT(finished.size(),1,5000);
        QCOMPARE(finished.first()[0].toULongLong(),generation);QCOMPARE(finished.first()[1].toInt(),2);QCOMPARE(batchCount,2);
        CatalogModel model;model.append({{dir.filePath("a.png"),"a.png","",0}});
        QCOMPARE(model.rowCount(),1);model.removePath(dir.filePath("a.png"));QCOMPARE(model.rowCount(),0);
        scanner.scan({dir.path()});scanner.cancel(); // destruction joins worker without force termination
    }
    void largeCatalogFilter(){
        QVector<MediaEntry> entries;entries.reserve(100000);
        for(int i=0;i<100000;++i)entries.append({QString("/references/ref_%1.jpg").arg(i),QString("ref_%1.jpg").arg(i),{},1});
        CatalogModel model;model.append(entries);QCOMPARE(model.rowCount(),100000);
        QSortFilterProxyModel filter;filter.setSourceModel(&model);filter.setFilterRole(CatalogModel::NameRole);
        filter.setFilterFixedString("ref_9999");QCOMPARE(filter.rowCount(),11);
        model.removePath("/references/ref_99999.jpg");QCOMPARE(filter.rowCount(),10);
    }
    void persistentEventsAndLegacyImport(){
        QTemporaryDir dir;QVERIFY(dir.isValid());
        {
            Store store(dir.path());QSignalSpy snapshots(&store,&Store::snapshot);
            QTRY_VERIFY_WITH_TIMEOUT(!snapshots.isEmpty(),5000);
            store.record("original.png","collection","shown");store.record("original.png","collection","completed",1000);
            store.refresh();QTRY_VERIFY_WITH_TIMEOUT(snapshots.size()>=2,5000);
            QCOMPARE(snapshots.last()[2].toMap().value("completed").toInt(),1);
            QCOMPARE(snapshots.last()[2].toMap().value("practiceMs").toInt(),1000);
            store.record("other.png","other","completed",200);store.clearHistory("collection");
            QTRY_VERIFY_WITH_TIMEOUT(snapshots.size()>=3,5000);
            QCOMPARE(snapshots.last()[2].toMap().value("completed").toInt(),1);
            QCOMPARE(snapshots.last()[2].toMap().value("practiceMs").toInt(),200);
            store.savePreferences({{"display_time",45},{"theme","light"}});
        }
        QCOMPARE(loadPreferences(dir.path()).value("display_time").toInt(),45);
        QFile bad(dir.filePath("settings.json"));QVERIFY(bad.open(QIODevice::WriteOnly));
        bad.write("{\"display_time\":\"invalid\",\"num_images\":-3,\"timer_volume\":200}");bad.close();
        const auto p=loadPreferences(dir.path());QCOMPARE(p.value("display_time").toInt(),30);
        QCOMPARE(p.value("num_images").toInt(),1);QCOMPARE(p.value("timer_volume").toInt(),100);
        QVERIFY(bad.open(QIODevice::WriteOnly));bad.write("{\"display_time\":true,\"num_images\":2.5,\"enable_breaks\":true}");bad.close();
        const auto strict=loadPreferences(dir.path());QCOMPARE(strict.value("display_time").toInt(),30);
        QCOMPARE(strict.value("num_images").toInt(),10);QVERIFY(strict.value("use_break").toBool());
        QTemporaryDir legacy;QFile history(legacy.filePath("history.json"));QVERIFY(history.open(QIODevice::WriteOnly));
        history.write("[\"a.jpg\",\"b.jpg\",\"a.jpg\"]");history.close();
        Store store(dir.path());QSignalSpy imported(&store,&Store::imported);
        store.importLegacy(legacy.path());QTRY_COMPARE_WITH_TIMEOUT(imported.size(),1,5000);QCOMPARE(imported.first()[0].toInt(),2);
        store.importLegacy(legacy.path());QTRY_COMPARE_WITH_TIMEOUT(imported.size(),2,5000);QCOMPARE(imported.last()[0].toInt(),0);
    }
};
QTEST_MAIN(IntegrationTests)
#include "integration_tests.moc"
