#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "class/pivot.h"
#include "class/proxy.h"
#include "class/result.h"

#include <QString>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFileInfo>

class DbManager
{
public:
    DbManager(const QString& path);
    ~DbManager();
    bool isOpen() const;
    bool createDb();
    bool updatePivots(const QList<Pivot*>&);
    bool updateProxies(const QList<Proxy*>&);
    bool updateResults(const QList<Result*>&);
    bool updateRegex(const QString&);
    bool addPivots(const QList<Pivot*>&);
    bool addProxies(const QList<Proxy*>&);
    bool addResults(const QList<Result*>&);
    void deleteRegex();
    void deletePivots();
    void deleteResults();
    void deleteProxies();
    QString getRegex();
    void getPivots(QList<Pivot*>&);
    void getProxies(QList<Proxy*>&);
    void getResults(QList<Result*>&);

private:
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
