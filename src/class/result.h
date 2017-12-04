#ifndef RESULT_H
#define RESULT_H

#include <QDate>
#include <QDebug>

class Result {
public:
    Result(int id, QDate date_add, QString hostname, QStringList urls_parts_list);
    ~Result();
    int id;
    QDate date_add;
    QString hostname;
    QStringList urls_parts_list;
    
    qint8 validation_state; // Not used
    
    QString getReverseHostname() const;
    inline bool operator== (const Result &r) const{ return ( this->hostname == r.hostname ); }
    inline bool operator< (const Result &r) const{
        qDebug() << "< appelé";
        return ( this->getReverseHostname() < r.getReverseHostname() ); }
    inline bool operator> (const Result &r) const{ return ( this->getReverseHostname() > r.getReverseHostname() ); }
private:
};

#endif // RESULT_H
