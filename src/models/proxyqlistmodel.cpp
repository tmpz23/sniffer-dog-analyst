#include "proxyqlistmodel.h"

ProxyQListModel::ProxyQListModel(const QList<Proxy*>* proxies){
    this->proxies = proxies;
}

int ProxyQListModel::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return this->proxies->count();
}
int ProxyQListModel::columnCount(const QModelIndex& parent) const{
    Q_UNUSED(parent);
    return 6;
}
QVariant ProxyQListModel::data(const QModelIndex &index, int role) const{
    if(!index.isValid()) return QVariant();
    if(index.row() >= this->proxies->count()) return QVariant();

    if(role == Qt::DisplayRole){
        switch (index.column()){
            case 0: return this->proxies->at(index.row())->ip; break;
            case 1: return QString::number(this->proxies->at(index.row())->port); break;
            case 2: return this->proxies->at(index.row())->protocol; break;
            case 3:
                if(this->proxies->at(index.row())->state & ProxyState::InUse)
                    return "In use";
                else return "No";
                break;
            case 4: return QString::number(this->proxies->at(index.row())->googlecacheCaptcha); break;
            case 5:
                if(this->proxies->at(index.row())->state & ProxyState::NotTested) return "NotTested";
                if(this->proxies->at(index.row())->state & ProxyState::Invalid)   return "Invalid";
                if(this->proxies->at(index.row())->state & ProxyState::Valid)     return "Valid";
                break;
            default: break;
        }
    }
    return QVariant();
}

QVariant ProxyQListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role != Qt::DisplayRole) return QVariant();

    if(orientation == Qt::Horizontal){
        QString column_name = "";
        switch (section) {
            case 0: column_name="Ip"; break;
            case 1: column_name="Port"; break;
            case 2: column_name="Protocol"; break;
            case 3: column_name="In Use"; break;
            case 4: column_name="GCache Captcha"; break;
            case 5: column_name="State"; break;
            default: break;
        }
        return column_name;
    }
    return QVariant();
}

void ProxyQListModel::refresh(){
    emit(layoutChanged());
}
