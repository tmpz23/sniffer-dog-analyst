#include "result.h"

Result::Result(int id, QDate date_add, QString hostname, QStringList urls_parts_list){ // Class to manage results
    this->id = id;
    this->date_add = date_add;
    this->hostname = hostname;
    this->urls_parts_list = urls_parts_list;
}

Result::~Result(){ //qDebug() << "Result deleted";
}

QString Result::getReverseHostname() const {
    QStringList _hostname = this->hostname.split('.');
    QString tmp;
    for(int i=0;i<(_hostname.count()/2);++i){
        tmp=_hostname[i];
        _hostname[i]=_hostname[_hostname.count()-i-1];
        _hostname[_hostname.count()-i-1]=tmp;
    }
    return _hostname.join('.');
}
