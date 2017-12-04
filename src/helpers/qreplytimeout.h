#ifndef QREPLYTIMEOUT_H
#define QREPLYTIMEOUT_H

#include <QNetworkReply>
#include <QTimer>

class QReplyTimeout : public QObject {
Q_OBJECT
public:
    QReplyTimeout(QNetworkReply*, const int);

private slots:
    void timeout();
};

#endif // QREPLYTIMEOUT_H
