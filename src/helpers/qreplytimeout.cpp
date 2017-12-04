#include "qreplytimeout.h"

/**
    Usage:
    new QReplyTimeout(yourReply, msec);

    When the timeout is reached the given reply is closed if still running
*/


QReplyTimeout::QReplyTimeout(QNetworkReply* reply, const int timeout) : QObject(reply) {
    Q_ASSERT(reply);
    if(reply) QTimer::singleShot(timeout, this, SLOT(timeout()));
}

void QReplyTimeout::timeout() {
    QNetworkReply* reply = static_cast<QNetworkReply*>(parent());
    if(reply->isRunning()) reply->close();
}
