#include "collections.h"
#include <QDir>
#include <QSet>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>

static QString pathKey(const QString& value) {
    QString path=QDir::cleanPath(QDir::fromNativeSeparators(value));
#ifdef Q_OS_WIN
    path=path.toCaseFolded();
#endif
    return path;
}
QVariantMap makeCollection(QStringList paths,const QString& kind,const QString& name) {
    paths.removeAll(QString{});paths.removeDuplicates();
    for(auto& path:paths)path=QDir::cleanPath(QDir::fromNativeSeparators(path));
    auto keys=paths;for(auto& path:keys)path=pathKey(path);keys.sort();
    const QString id=kind=="folder" && keys.size()==1 ? "folder:"+keys.first()
        : "files:"+QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(QJsonArray::fromStringList(keys)).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
    return {{"id",id},{"paths",paths},{"kind",kind=="folder"?"folder":"files"},{"name",name.trimmed().left(100)},{"selected",true}};
}
QVariantList normalizeCollections(const QVariantList& values) {
    QVariantList result;QSet<QString> ids;
    for(const auto& value:values) {
        const auto item=value.toMap();QStringList paths;
        for(const auto& p:item.value("paths").toList())if(p.metaType().id()==QMetaType::QString && !p.toString().trimmed().isEmpty() && QDir::isAbsolutePath(p.toString()))paths.append(p.toString());
        if(paths.isEmpty())continue;
        const QString kind=item.value("kind").toString()=="folder" && paths.size()==1 ? "folder" : "files";
        auto normalized=makeCollection(paths,kind,item.value("name").toString());
        if(normalized.value("name").toString().isEmpty())normalized["name"]=kind=="folder"?QDir(paths.first()).dirName():"Selected images";
        const auto selected=item.value("selected");if(selected.metaType().id()==QMetaType::Bool)normalized["selected"]=selected;
        const QString id=normalized.value("id").toString();if(ids.contains(id))continue;
        ids.insert(id);result.append(normalized);
    }
    return result;
}
QStringList collectionPaths(const QVariantList& collections,bool selectedOnly) {
    QStringList result;QSet<QString> keys;
    for(const auto& value:collections){const auto item=value.toMap();if(selectedOnly&&!item.value("selected").toBool())continue;
        for(const auto& path:item.value("paths").toStringList())if(!keys.contains(pathKey(path))){keys.insert(pathKey(path));result.append(path);}}
    return result;
}
bool pathInSources(const QString& path,const QStringList& sources) {
    const QString key=pathKey(path);
    for(const auto& source:sources){const QString root=pathKey(source);const QString prefix=root.endsWith('/')?root:root+'/';
        if(key==root || key.startsWith(prefix))return true;}
    return false;
}
