#ifndef RESULTTEST_H
#define RESULTTEST_H


#include "class/result.h"

#include <QString>
#include <QDebug>
#include <QtTest>

class ResultTest: public QObject {
    Q_OBJECT
private slots:
    void initTestCase() { qDebug("Result class tests init"); }
    void getReverseHostname();
    void operator_equal();
    void operator_lt();
    void operator_gt();
    void cleanupTestCase() { qDebug("Result class tests end"); }
};

#endif // RESULTTEST_H
