#ifndef PIVOT_H
#define PIVOT_H

#include <QString>
#include <QDebug>

enum class PivotState{
    NotProcessed,
    Loaded,
    Processed
};

class Pivot{
public:
    Pivot(QString);
    ~Pivot();
    QString pivot;
    PivotState state;
    int start;
};

#endif // PIVOT_H
