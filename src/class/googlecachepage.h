#ifndef GOOGLECACHEPAGE_H
#define GOOGLECACHEPAGE_H

#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

class GooglecachePage
{
public:
    GooglecachePage();
    void load(QString); // load html
    bool isCaptchaPage();
    bool isLastPage();
    QStringList getMatchingUrls(QRegularExpression);
private:
    QString html;
    bool captcha;
    bool lastPage;
};

#endif // GOOGLECACHEPAGE_H
