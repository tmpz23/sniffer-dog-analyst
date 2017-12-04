#include "scrapper.h"

Scrapper::Scrapper(QRegularExpression regex,QList<Pivot*>& pivots,QList<Proxy*>& proxies){
    this->netManager = new QNetworkAccessManager(this);
    this->gp = new GooglecachePage();
    this->timer = new QTimer(this);

    this->flags = ScrapperFlags::SearchProxy;
    this->state = ScrapperState::Start;

    this->regex = regex;
    this->pivots = pivots;
    this->proxies = proxies;
    
    this->proxy_in_use = NULL;
    this->pivot_in_use = NULL;
    
    this->timer->setSingleShot(true);

    connect(this->timer,SIGNAL(timeout()),                      this,SLOT(sendRequest()));
    connect(this->netManager,SIGNAL(finished(QNetworkReply*)),  this,SLOT(onReply(QNetworkReply*)));
}
Scrapper::~Scrapper(){
    delete this->netManager;
    delete this->gp;
    delete this->timer;

    if(this->proxy_in_use != NULL) if(this->proxy_in_use->state & ProxyState::InUse)
        this->proxy_in_use->state &= ~ProxyState::InUse;
    if(this->pivot_in_use != NULL) this->pivot_in_use->state = PivotState::NotProcessed;
}

// ScrapperState::Start
void Scrapper::start(){
    this->mutex.lock();
    for(QList<Proxy*>::iterator iProx = this->proxies.begin(); iProx != this->proxies.end(); ++iProx){
        if( // If proxy is available
            ((*iProx)->state == ProxyState::NotTested || (*iProx)->state == ProxyState::Valid) &&
            !(*iProx)->googlecacheCaptcha
        ){
            (*iProx)->state |= ProxyState::InUse;
            this->mutex.unlock();
            this->proxy_in_use = (*iProx);

            this->qProxy.setType(QNetworkProxy::HttpProxy);
            this->qProxy.setHostName(this->proxy_in_use->ip);
            this->qProxy.setPort(this->proxy_in_use->port);
            QNetworkProxy::setApplicationProxy(this->qProxy);

            emit(proxiesChange());

            this->dispatch(ScrapperState::SendRequest);
            return;
        }
    }
    this->mutex.unlock();

    // Can't find any proxy
    emit(appendLog("No proxy available for this scrapper : stop the search and update proxy list ;",LogColor::Red));
    this->dispatch(ScrapperState::Stop);
    return;
}

// ScrapperState::Wait
void Scrapper::wait(){
    unsigned int time_rand = (rand() % 3000 + 3000);
    this->timer->start(time_rand);

    this->dispatch(ScrapperState::WaitSendRequest);
}

// ScrapperState::SendRequest
void Scrapper::sendRequest(){
    if(!this->dispatch(ScrapperState::BeginSendRequest)) return;

    QUrl url;

    if(this->flags & ScrapperFlags::SearchProxy){
        url = QUrl("http://www.police-nationale.net/policier-maitre-chien/");
        emit appendLog("[PROXY TEST] ip: "+
                    this->qProxy.hostName()+":"+QString::number(this->qProxy.port())+" -> "+url.toDisplayString(QUrl::FullyDecoded),
                    LogColor::Yellow);
    }else{
        if(this->pivot_in_use == NULL){
            this->pivot_in_use = this->getPivot(); // Get new one
            // No Pivot available : wait and test again later ...
            if(this->pivot_in_use == NULL){
                this->dispatch(ScrapperState::Wait);
                return;
            }
        }
        QString start_param = (this->pivot_in_use->start > 0)? "&start="+QString::number(this->pivot_in_use->start * 100):"";
        QString pivot = QUrl::toPercentEncoding(this->pivot_in_use->pivot.replace(' ','+'));
        url = QUrl("http://webcache.googleusercontent.com/custom?q=-%22&q=cache:&num=100&q=custom_api_key%22&q="+pivot+start_param);

        emit appendLog("[Page "+ QString::number(this->pivot_in_use->start)+ "] "+ url.toDisplayString(QUrl::FullyDecoded),
                       LogColor::Grey);
    }

    QNetworkRequest request;
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:57.0) Gecko/20100101 Firefox/57.0");
    request.setUrl(url);

    this->netManager->get(request);
    this->dispatch(ScrapperState::WaitOnReply);
}

// ScrapperState::OnReply
void Scrapper::onReply(QNetworkReply* reply){
    if(reply->error()){
        emit appendLog("[PROXY NOK] ip: "+this->qProxy.hostName()+":"+QString::number(this->qProxy.port())+
                    " : "+reply->errorString(),
                    LogColor::Red);
        reply->deleteLater();

        this->proxy_in_use->state = ProxyState::Invalid;
        this->proxy_in_use = NULL;
        emit(proxiesChange());

        if(this->pivot_in_use != NULL){
            this->pivot_in_use->state = PivotState::NotProcessed;
            this->pivot_in_use = NULL;
        }

        this->dispatch(ScrapperState::Start);
        return;
    }


    QByteArray bytes = reply->readAll();
    QString html = QString::fromUtf8(bytes.data(), bytes.size());
    reply->deleteLater();

    if(this->flags & ScrapperFlags::SearchProxy){
        if(QString::number(html.split("stup").count()) > 1){ // If the proxy is ok
            this->proxy_in_use->state = ProxyState::Valid | ProxyState::InUse;
            
            emit(appendLog("[PROXY OK] ip: "+this->qProxy.hostName()+":"+QString::number(this->qProxy.port()),
                        LogColor::Blue));
            emit(proxiesChange());
            this->flags &= ~ScrapperFlags::SearchProxy;

            this->dispatch(ScrapperState::SendRequest);
            return;
        }else{
            this->proxy_in_use->state = ProxyState::Invalid;
            this->proxy_in_use = NULL;
            emit(proxiesChange());
            this->dispatch(ScrapperState::Start);
            return;
        }
    }

    this->gp->load(html);

    if(gp->isCaptchaPage()){ // Update the current proxy and change to new
        emit appendLog("Captcha on proxy :"+this->proxy_in_use->ip+":"+QString::number(this->proxy_in_use->port)+" -> searching a new proxy",
                       LogColor::Red);
        this->proxy_in_use->googlecacheCaptcha = true;
        this->proxy_in_use->state &= ~ProxyState::InUse;
        this->proxy_in_use = NULL;

        emit proxiesChange();

        this->pivot_in_use->state = PivotState::NotProcessed;
        this->pivot_in_use = NULL;

        this->dispatch(ScrapperState::Start);
        return;
    }

    // Retrieve all urls matching regex from html
    QStringList urlsList = gp->getMatchingUrls(this->regex);

    emit appendLog("new matches:"+QString::number(urlsList.count()),LogColor::Blue);

    if(gp->isLastPage()){ // End of Google Search scraping for this pivot
        this->pivot_in_use->state = PivotState::Processed;
        this->pivot_in_use = NULL;
        emit(incrementProgress());
    }else
        this->pivot_in_use->start += 1;
    
    emit(sendData(urlsList));

    this->dispatch(ScrapperState::SendRequest);
    return;
}

void Scrapper::pause(bool bPause){
    if(bPause)
        this->flags |= ScrapperFlags::Pause;
    else{
        this->flags &= ~ScrapperFlags::Pause;
        this->dispatch(this->state);
    }
}
void Scrapper::stop(){
    if(this->state == ScrapperState::WaitOnReply || this->state == ScrapperState::WaitSendRequest){
        disconnect(this->timer,SIGNAL(timeout()),                      this,SLOT(sendRequest()));
        disconnect(this->netManager,SIGNAL(finished(QNetworkReply*)),  this,SLOT(onReply(QNetworkReply*)));
        this->state = ScrapperState::Stop;
        this->dispatch(ScrapperState::Disconnected);
        return;
    }
    this->state = ScrapperState::Stop;
}

void Scrapper::finish(){
    if(this->pivot_in_use != NULL)
        this->pivot_in_use->state = PivotState::NotProcessed;
    if(this->proxy_in_use != NULL)
        this->proxy_in_use->state &= ~ProxyState::InUse;
    emit(finish(this));
}

Pivot* Scrapper::getPivot(){
    this->mutex.lock();
    for(QList<Pivot*>::iterator iPiv = this->pivots.begin(); iPiv != this->pivots.end(); ++iPiv){
        if((*iPiv)->state == PivotState::NotProcessed){
            (*iPiv)->state = PivotState::Loaded;
            this->mutex.unlock();
            return *iPiv;
        }
    }
    this->mutex.unlock();
    return NULL;
}

bool Scrapper::dispatch(unsigned int nextState){
    //qDebug() << "this->state : "+QString::number(this->state)+" | nextState : "+QString::number(nextState);
    if(this->state == ScrapperState::Disconnected) return false;
    if(this->state == ScrapperState::Stop){
        if(nextState == ScrapperState::Disconnected)
            this->state = nextState;
        this->finish();
        return false;
    }

    if(nextState != ScrapperState::BeginSendRequest) this->state = nextState;
    if(this->flags & ScrapperFlags::Pause) return false;

    switch(nextState){
        case(ScrapperState::BeginSendRequest) : return true; break;
        case(ScrapperState::Start)            : this->start(); break;
        case(ScrapperState::Stop)             : this->finish(); break;
        case(ScrapperState::SendRequest)      : this->sendRequest(); break;
        case(ScrapperState::Wait)             : this->wait(); break;
        case(ScrapperState::WaitSendRequest)  : break;
        case(ScrapperState::WaitOnReply)      : break;
    }
    return false;
}
