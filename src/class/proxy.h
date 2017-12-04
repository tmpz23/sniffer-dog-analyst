#ifndef PROXY_H
#define PROXY_H

#include <QString>
#include <QDebug>

enum ProxyState : unsigned int{
    NotTested = 1,
    InUse = 2,
    Valid = 4,
    Invalid = 8
};

class Proxy{
public:
    Proxy(QString,QString,QString,bool);
    ~Proxy();
    unsigned int state;
    
    QString ip;
    int port;
    QString protocol;
    bool googlecacheCaptcha;
};

#endif // PROXY_H
