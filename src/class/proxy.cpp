#include "proxy.h"

Proxy::Proxy(QString ip,QString port,QString protocol,bool googlecacheCaptcha){
    this->state = ProxyState::NotTested;
    this->ip = ip;
    this->port = port.toInt();
    this->protocol = protocol;
    this->googlecacheCaptcha = googlecacheCaptcha;
}
Proxy::~Proxy(){ //qDebug() << "proxy deleted";
}
