#ifndef SCRAPPER_H
#define SCRAPPER_H

#include "helpers/logcolor.h"
//#include "helpers/qreplytimeout.h"
#include "class/result.h"
#include "class/proxy.h"
#include "class/pivot.h"
#include "class/googlecachepage.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QTimer>
#include <QRegularExpression>
#include <QMutexLocker>

enum ScrapperState : unsigned int {
    Start = 1,
    SendRequest = 2,
    Stop = 4,
    WaitSendRequest = 8,
    BeginSendRequest = 16,
    Wait = 32,
    WaitOnReply = 64,
    Disconnected = 128
};
enum ScrapperFlags : unsigned int {
    SearchProxy = 1,
    Pause = 2
};

class Scrapper : public QObject {
    Q_OBJECT

public:
    void start();
    void pause(bool bPause);
    void stop();

    Scrapper(QRegularExpression regex,QList<Pivot*>& pivots,QList<Proxy*>& proxies);
    ~Scrapper();
private:
    unsigned int state;
    unsigned int flags;

    QNetworkAccessManager *netManager;
    GooglecachePage* gp;
    
    // Passed by reference in constructor and shared between scrappers;
    QRegularExpression regex;
    QList<Pivot*> pivots;
    QList<Proxy*> proxies;

    QNetworkProxy qProxy;
    Proxy* proxy_in_use; // Store the proxy flaged with inUse;
    Pivot* pivot_in_use; // Store the pivot flaged with state

    QTimer *timer; // Wait between requests

    Pivot* getPivot(); // getNextPivot
    void wait();
    bool dispatch(unsigned int nextState);
    void finish();

    mutable QMutex mutex;

private slots:
    void sendRequest();
    void onReply(QNetworkReply*);

signals:
    void appendLog(QString,LogColor);
    void sendData(QStringList urlsList);
    void incrementProgress();
    void finish(Scrapper*);

    void proxiesChange();
};

#endif // SCRAPPER_H
