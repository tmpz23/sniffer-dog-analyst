#include "pivot.h"

Pivot::Pivot(QString pivot){
    this->pivot = pivot;
    this->state = PivotState::NotProcessed;
    this->start = 0;
}
Pivot::~Pivot(){ qDebug() << "pivot deleted";
}
