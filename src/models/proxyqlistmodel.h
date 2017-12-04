#ifndef PROXYQLISTMODEL_H
#define PROXYQLISTMODEL_H

#include "class/proxy.h"

#include <QObject>
#include <QAbstractTableModel>

class ProxyQListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ProxyQListModel(const QList<Proxy*>*);

    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void refresh();

private:
    const QList<Proxy*>* proxies;
};
#endif // PROXYQLISTMODEL_H
