#ifndef SEARCHORCHESTRATOR_H
#define SEARCHORCHESTRATOR_H

#include "class/pivot.h"
#include "class/proxy.h"
#include "class/result.h"
#include "class/scrapper.h"
#include "models/dbmanager.h"

#include <QString>
#include <QDebug>

class SearchOrchestrator : public QObject {
    Q_OBJECT

public:
    SearchOrchestrator(DbManager* dbManager, QList<Pivot*>* pivots, QRegularExpression* regex, QList<Proxy*>* proxies, QList<Result*>* results);
    ~SearchOrchestrator();

    void start(int);
    void pause(bool);
    void stop();

private:
    DbManager *dbManager;
    QList<Proxy*>* proxies;
    QList<Pivot*>* pivots;
    QRegularExpression* regex;
    QList<Result*>* results;
    QList<Scrapper*> scrappers;

    int total;
    int progress;
    QStringList urlsList;

    QString getRelativePath(QUrl);
    void processUrls();

private slots:
    void onSendData(QStringList urlsList);
    void onFinish(Scrapper* scrapper);
    void onAppendLog(QString log, LogColor color);
    void onIncrementProgress();
    void onProxiesChange();

signals:
    void appendLog(QString,LogColor);
    void proxiesChange();
    void setProgress(int);
    void setNbScrapper(int);
    
    void orchestratorFinish();

};

#endif // SEARCHORCHESTRATOR_H
