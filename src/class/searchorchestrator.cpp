#include "class/searchorchestrator.h"

SearchOrchestrator::SearchOrchestrator(DbManager* dbManager, QList<Pivot*>* pivots, QRegularExpression* regex, QList<Proxy*>* proxies, QList<Result*>* results){
    this->dbManager = dbManager;
    this->pivots = pivots;
    this->regex = regex;
    this->proxies = proxies;
    this->results = results;

    srand(time(NULL));
}

SearchOrchestrator::~SearchOrchestrator(){}

// Get full relative path from url
QString SearchOrchestrator::getRelativePath(QUrl url){
    QString url_part = url.path(QUrl::EncodeUnicode);

    url_part += url.hasQuery() ? '?'+url.query(QUrl::EncodeUnicode) : "";
    url_part += url.hasFragment() ? '#'+url.fragment(QUrl::EncodeUnicode) : "";

    return url_part;
}


void SearchOrchestrator::start(int nbScrappers){
    if(this->proxies->count() < 1){
        emit(appendLog("Stoping : no proxies",LogColor::Red));
        emit(orchestratorFinish());
        return;
    }

    // For progress bar
    this->progress = -1;

    for(QList<Pivot*>::const_iterator iPiv = this->pivots->begin(); iPiv != this->pivots->end(); ++iPiv)
        if( (*iPiv)->state == PivotState::Processed ) progress++;
    
    this->total = this->pivots->count();
    this->onIncrementProgress();

    if(this->total < 1){
        emit(appendLog("Stoping : no pivots available for search",LogColor::Red));
        emit(orchestratorFinish());
        return;
    }
    if(this->progress == this->total){
        emit(appendLog("Stoping : all pivots are already processed",LogColor::Red));
        emit(orchestratorFinish());
        return;
    }
    nbScrappers = ((nbScrappers % 1000)>0)?(nbScrappers % 1000):1; // try/catch à utiliser pour les exeptions QThread, à vérif pool de threads ?
    
    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(this->pivots->begin(),this->pivots->end(),g);

    for(int i=0; i!=nbScrappers;++i){
        Scrapper* scrapper = new Scrapper(*this->regex, *this->pivots, *this->proxies);
        this->scrappers << scrapper;

        connect(scrapper,SIGNAL(appendLog(QString,LogColor)), this,SLOT(onAppendLog(QString,LogColor)));
        connect(scrapper,SIGNAL(proxiesChange()),             this,SLOT(onProxiesChange()));
        connect(scrapper,SIGNAL(incrementProgress()),         this,SLOT(onIncrementProgress()));
        
        connect(scrapper,SIGNAL(finish(Scrapper*)),           this,SLOT(onFinish(Scrapper*)));
        connect(scrapper,SIGNAL(sendData(QStringList)),       this,SLOT(onSendData(QStringList)));

        scrapper->start();
    }
    emit(appendLog("Search init with "+QString::number(this->scrappers.count())+" Scrappers for "+
                   QString::number(this->pivots->count())+" pivots ...",
                   LogColor::Green));
    emit(setNbScrapper(this->scrappers.count()));
}
void SearchOrchestrator::pause(bool bPause){
    for(QList<Scrapper*>::iterator iScrap = this->scrappers.begin(); iScrap != this->scrappers.end(); ++iScrap)
        (*iScrap)->pause(bPause);
}
void SearchOrchestrator::stop(){
    for(QList<Scrapper*>::iterator iScrap = this->scrappers.begin(); iScrap != this->scrappers.end(); ++iScrap)
        (*iScrap)->stop();
}

void SearchOrchestrator::onFinish(Scrapper* scrapper){
    this->scrappers.removeOne(scrapper);
    scrapper->deleteLater();

    emit(setNbScrapper(this->scrappers.count()));

    if(this->scrappers.count() == 0){
        this->processUrls();
        this->dbManager->updateResults(*this->results);
        emit(orchestratorFinish());
    }
}

void SearchOrchestrator::processUrls(){
    if(this->urlsList.count() <= 0) return;

    QDate current_date = QDate::currentDate();
    QString new_hostname;
    QString new_url_part;
    bool bInResults;

    this->urlsList.removeDuplicates();
    foreach(QUrl url, this->urlsList){
        new_hostname = url.host();
        new_url_part = this->getRelativePath(url);
        bInResults = false;

        for(QList<Result*>::iterator iRes = this->results->begin(); iRes != this->results->end(); ++iRes){
            if((*iRes)->hostname == new_hostname){
                bInResults = true;
                if(! (*iRes)->urls_parts_list.contains(new_url_part, Qt::CaseInsensitive)){
                    (*iRes)->urls_parts_list << new_url_part;
                    (*iRes)->urls_parts_list.sort();
                }
                break;
            }
        }

        if(! bInResults)
            *(this->results) << new Result(0,current_date,new_hostname,QStringList(new_url_part));
    }
}

void SearchOrchestrator::onSendData(QStringList urlsList){
    this->urlsList << urlsList;
}

void SearchOrchestrator::onAppendLog(QString log,LogColor color){
    emit(appendLog(log,color));
}
void SearchOrchestrator::onProxiesChange(){
    emit(proxiesChange());
}
void SearchOrchestrator::onIncrementProgress(){
    this->progress += 1;
    emit(setProgress(this->progress*100 / this->total));
    if(this->progress == this->total) this->stop();
}
