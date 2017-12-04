#include "googlecachepage.h"

GooglecachePage::GooglecachePage(){}
void GooglecachePage::load(QString html){
    this->html = html;

    if( this->html.split("<A HREF=\"http://ipv4.google.com/sorry/index?continue=").count() > 1){
        this->captcha = true;
        return;
    }else this->captcha = false;

    this->lastPage = (this->html.split("<span>Suivant</span></a></td></tr></table>").count() > 1) ? false : true;
}

bool GooglecachePage::isCaptchaPage(){ return this->captcha; }
bool GooglecachePage::isLastPage(){ return this->lastPage; }

QStringList GooglecachePage::getMatchingUrls(QRegularExpression regex){
    QStringList urls_list;
    QString res_div = "";
    QRegularExpressionMatch match;

    if(html.split("<div id=\"res\">").count() > 1){
        if(html.split("<div id=\"res\">").at(1).split("</div></td></tr></table></div></li></ol></div>").count() > 1)
            res_div = html.split("<div id=\"res\">").at(1).split("</div></td></tr></table></div></li></ol></div>").at(0);
        else return urls_list;
    }else return urls_list;

    if(res_div.split("<div class=\"g\"").count() > 0){
        foreach (QString res, res_div.split("<div class=\"g\"")){
            if(res.split("href=\"").count() > 1){
                QString url = res.split("href=\"").at(1).split("\"").at(0);
                match = regex.match(url);
                if(match.capturedLength()>0 && url.at(0) != '/') urls_list.append(url);
            }
        }
    }
    return urls_list;
}
