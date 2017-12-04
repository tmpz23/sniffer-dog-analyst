#include "mainwindow.h"
#include "ui_mainwindow.h"
bool comp_ppiv(const Pivot *r1, const Pivot *r2){ return ( r1->pivot < r2->pivot ); }

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    this->ui->setupUi(this);

    QString path_db = "sniffer_dog_analyst.db";
    this->dbManager = new DbManager(path_db);
    this->loadDb();
    this->orchestrator = new SearchOrchestrator(this->dbManager, &this->pivots, &this->regex, &this->proxies, &this->results);

    this->pModel = new ProxyQListModel( &this->proxies );
    this->ui->tableView_proxies->setModel(pModel);
    this->pModel->refresh();

    this->ui->pushButton_pause->setEnabled(false);
    this->ui->pushButton_stop->setEnabled(false);
    this->ui->lineEdit_nbScrappers->setText("5");
    this->ui->lineEdit_regex->setText(this->regex.pattern());
    this->ui->textBrowser_results->setOpenLinks(false);

    this->showProxies();
    this->showPivots();
    this->showResults();

    connect(this->ui->pushButton_update_PR,SIGNAL(clicked()),    this,SLOT(onUpdatePivotsRegex()));
    connect(this->ui->pushButton_updateProxies,SIGNAL(clicked()),this,SLOT(onUpdateProxies()));

    connect(this->ui->pushButton_resetResults,SIGNAL(clicked()), this,SLOT(onResetResults()));
    connect(this->ui->pushButton_searchResults,SIGNAL(clicked()),this,SLOT(onSearchResults()));
    connect(this->ui->pushButton_exportResults,SIGNAL(clicked()),this,SLOT(onExportResults()));

    connect(this->ui->pushButton_search,SIGNAL(clicked()),       this,SLOT(onStartClicked()));
    connect(this->ui->pushButton_pause,SIGNAL(clicked()),        this,SLOT(onPauseClicked()));
    connect(this->ui->pushButton_stop,SIGNAL(clicked()),         this,SLOT(onStopClicked()));

    // Orchestrator
    connect(this->orchestrator,SIGNAL(appendLog(QString,LogColor)),this,SLOT(onAppendLog(QString,LogColor)));
    connect(this->orchestrator,SIGNAL(orchestratorFinish()),       this,SLOT(onOrchestratorFinish()));

    connect(this->orchestrator,SIGNAL(setNbScrapper(int)),         this,SLOT(onSetNbScrapper(int))); // updateNbScrapper(int)
    connect(this->orchestrator,SIGNAL(setProgress(int)),           this,SLOT(onSetProgress(int)));   // updateProgress()
}
MainWindow::~MainWindow(){
    // Delete pivots
    if(this->pivots.count() > 0){
      Pivot* _pivot;
      QMutableListIterator<Pivot*> miPi(this->pivots);
      while(miPi.hasNext()){
          _pivot = miPi.next();
          miPi.remove();
          delete _pivot;
      }
    }

    // Delete proxies
    if(this->proxies.count() > 0){
      Proxy* _proxy;
      QMutableListIterator<Proxy*> miProx(this->proxies);
      while(miProx.hasNext()){
          _proxy = miProx.next();
          miProx.remove();
          delete _proxy;
      }
    }

    // Delete results
    if(this->results.count() > 0){
      Result* _result;
      QMutableListIterator<Result*> miRe(this->results);
      while(miRe.hasNext()){
          _result = miRe.next();
          miRe.remove();
          delete _result;
      }
    }

    this->orchestrator->deleteLater();
    delete this->pModel;
    delete this->dbManager;
    delete ui;
}

void MainWindow::loadDb(){
    this->regex.setPattern(this->dbManager->getRegex()); // Load Regex
    this->dbManager->getPivots(this->pivots); // Load Pivots
    this->dbManager->getResults(this->results); // Load Results
    this->dbManager->getProxies(this->proxies); // Load Proxies
}
void MainWindow::showPivots(){
    std::sort(this->pivots.begin(),this->pivots.end(),comp_ppiv);

    QString pivots_txt = "";
    for(QList<Pivot*>::const_iterator iPiv = this->pivots.begin(); iPiv != this->pivots.end(); ++iPiv)
        pivots_txt += (*iPiv)->pivot+'\n';

    this->ui->textEdit_pivots->setText(pivots_txt.trimmed());
}
void MainWindow::showProxies(){
    QString proxies_txt = "";
    for(QList<Proxy*>::const_iterator iProx = this->proxies.begin(); iProx != this->proxies.end(); ++iProx)
        proxies_txt += (*iProx)->ip+'\t'+QString::number((*iProx)->port)+'\t'+(*iProx)->protocol+'\n';

    this->ui->textEdit_proxies->setText(proxies_txt.trimmed());
}
void MainWindow::showResults(){
    std::sort(this->results.begin(),this->results.end());
    QString results_txt = "";
    int MAX_DISPLAY = 1000;
    int displayed = 0;

    for(QList<Result*>::const_iterator iRes = this->results.begin(); iRes != this->results.end(); ++iRes){
        results_txt += "<span style='color:blue;'>"+(*iRes)->hostname.toUtf8()+"</span>"+
            (*iRes)->date_add.toString(" <span style='color:orange;'>(dd-MM-yyyy)</span>") + "<br />";

        foreach(QString url_part, (*iRes)->urls_parts_list){
            results_txt += "<span style='color:grey;'>|-----"+url_part.toUtf8()+"</span><br />";
            displayed +=1;
            if(displayed >= MAX_DISPLAY) break;
        }
        if(displayed >= MAX_DISPLAY) break;
    }
    this->ui->label_results->setText(QString::number(displayed)+
        " lines displayed (total hostnames : "+QString::number(this->results.count())+")");
    this->ui->textBrowser_results->setText(results_txt);
}

void MainWindow::onStartClicked(){
    // Ui
    this->ui->pushButton_search->setEnabled(false);
    this->ui->pushButton_update_PR->setEnabled(false);
    this->ui->pushButton_pause->setEnabled(true);
    this->ui->pushButton_stop->setEnabled(true);
    this->ui->pushButton_updateProxies->setEnabled(false);
    this->ui->progressBar->setValue(0);

    this->bPause = false;

    this->orchestrator->start(this->ui->lineEdit_nbScrappers->text().toInt());
}
void MainWindow::onPauseClicked(){
    this->bPause = !this->bPause;
    this->orchestrator->pause(this->bPause);
    if(this->bPause) this->ui->pushButton_pause->setText("Resume");
    else this->ui->pushButton_pause->setText("Pause");
}
void MainWindow::onStopClicked(){
    this->ui->pushButton_stop->setEnabled(false);
    this->orchestrator->stop();
}

void MainWindow::onOrchestratorFinish(){
    this->onAppendLog("The scrapping process is now finished.",LogColor::Green);
    if(this->bPause){
        this->bPause = false;
        this->ui->pushButton_pause->setText("Pause");
    }
    this->ui->pushButton_search->setEnabled(true);
    this->ui->pushButton_update_PR->setEnabled(true);
    this->ui->pushButton_pause->setEnabled(false);
    this->ui->pushButton_stop->setEnabled(false);
    this->ui->pushButton_updateProxies->setEnabled(true);

    this->showResults();
}
// Slot activated when "update" on configuration tab is clicked
void MainWindow::onUpdatePivotsRegex(){
    QStringList newPivots;

    QString regex = QString("%1").arg(this->ui->lineEdit_regex->text());
    regex = (!regex.isEmpty()) ? QString("%1").arg(this->ui->lineEdit_regex->text()).simplified():".";
    this->regex.setPattern( regex );

    if(!this->regex.isValid()) this->regex.setPattern(".");

    foreach(QString pivot, this->ui->textEdit_pivots->toPlainText().split('\n'))
        if(!pivot.simplified().isEmpty()) newPivots << pivot.simplified();

    newPivots.removeDuplicates();

    Pivot* _pivot;
    QMutableListIterator<Pivot*> mi(this->pivots);

    while(mi.hasNext()){
        _pivot = mi.next();
        if(newPivots.contains(_pivot->pivot)) // Don't add new pivots if it has been processed
            newPivots.removeAll( _pivot->pivot );
        else{ // Remove old ones when it is not in new list
            mi.remove();
            delete _pivot;
        }
    }

    foreach(QString pivot, newPivots) // We add new ones
        this->pivots << new Pivot(pivot);

    this->dbManager->updateRegex(this->regex.pattern());
    this->dbManager->updatePivots(this->pivots);

    this->ui->lineEdit_regex->setText(this->regex.pattern());
    this->showPivots();
}
// Slot activated when "update" on proxies tab is clicked
void MainWindow::onUpdateProxies(){
    Proxy* _proxy;
    QMutableListIterator<Proxy*> mi(this->proxies);

    while(mi.hasNext()){
        _proxy = mi.next();
        mi.remove();
        delete _proxy;
    }

    if(this->ui->textEdit_proxies->toPlainText().split('\n').count() > 0){
        QStringList proxList = this->ui->textEdit_proxies->toPlainText().split('\n');
        proxList.removeDuplicates();

        foreach(QString proxy, proxList){
            if(proxy.split('\t').count() > 2)
                this->proxies << new Proxy(
                                     proxy.split('\t').at(0), // ip
                                     proxy.split('\t').at(1), // port
                                     proxy.split('\t').at(2), // protocol
                                     false); // googlecacheCaptcha
        }
    }

    this->dbManager->updateProxies(this->proxies);
    this->showProxies(); // Not reloaded from db to show what was saved but from memory
    this->pModel->refresh();
}
void MainWindow::onProxiesChange(){
    this->pModel->refresh();
}
// a modif
void MainWindow::onSetNbScrapper(int nbScrappers){
    this->onAppendLog("["+QString::number(nbScrappers)+"] scrappers on work",LogColor::Green);
    this->ui->label_nbProxiesInUse->setText("["+QString::number(nbScrappers)+"] scrappers");
}
void MainWindow::onSetProgress(int progress){ // onUpdateProgress
    this->ui->progressBar->setValue(progress);
}
void MainWindow::onAppendLog(QString logs, LogColor color = LogColor::Default){
    switch(color){
        case LogColor::Blue :   logs="<span style='color:blue;'>"+   logs+"</span>"; break;
        case LogColor::Green :  logs="<span style='color:green;'>"+  logs+"</span>"; break;
        case LogColor::Yellow : logs="<span style='color:#FFA500;'>"+logs+"</span>"; break;
        case LogColor::Red :    logs="<span style='color:red;'>"+    logs+"</span>"; break;
        case LogColor::Grey :   logs="<span style='color:grey;'>"+   logs+"</span>"; break;
        default: break;
    }
    this->ui->textBrowser_logs->append("<span style='color:grey;'>"+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"</span> "+logs);
}
void MainWindow::onResetResults(){
    this->dbManager->deleteResults();
    this->dbManager->getResults(this->results);
    this->showResults();
}
void MainWindow::onExportResults(){
    QString filename = QFileDialog::getSaveFileName( // getting the filename (full path)
                this, "CSV Filename",
                QDate::currentDate().toString("yyyy-MM-dd")+".csv",
                "CSV files (.csv);;Zip files (.zip, *.7z)", 0, 0);
    QFile data(filename);

    if(data.open(QFile::WriteOnly|QFile::Truncate)){
        QTextStream output(&data);
        QRegularExpressionMatch match;
        QString results_txt_tmp = "";
        bool bResMatch = false;
        QRegularExpression search(this->ui->lineEdit_searchResults->text());

        output.setCodec("UTF-8");

        if(!search.isValid() || (this->ui->lineEdit_searchResults->text() == "")) search.setPattern(".");

        for(QList<Result*>::const_iterator iRes = this->results.begin(); iRes != this->results.end(); ++iRes){
            bResMatch = false;
            results_txt_tmp = "";
            foreach (QString url_part, (*iRes)->urls_parts_list){
                match = search.match((*iRes)->hostname+url_part);
                if(match.capturedLength()>0) bResMatch=true;
                results_txt_tmp +=
                        (*iRes)->date_add.toString("yyyy-MM-dd")+'\t'+
                        (*iRes)->hostname.toUtf8()+'\t'+
                        url_part.toUtf8()+'\n';
            }
            if(bResMatch) output << results_txt_tmp;
        }
    }
}
void MainWindow::onSearchResults(){
    QRegularExpression search(this->ui->lineEdit_searchResults->text());
    if(!search.isValid() || (this->ui->lineEdit_searchResults->text() == "")) search.setPattern(".");

    QString results_txt = "";

    QRegularExpressionMatch match;
    QString results_txt_tmp = "";
    bool bResMatch = false;
    int MAX_DISPLAY = 1000;
    int displayed = 0;
    int displayed_tmp = 0;

    for(QList<Result*>::const_iterator iRes = this->results.begin(); iRes != this->results.end(); ++iRes){
        bResMatch = false;
        displayed_tmp = 0;
        results_txt_tmp = "<span style='color:blue;'>"+(*iRes)->hostname.toUtf8()+"</span>"+
            (*iRes)->date_add.toString(" <span style='color:orange;'>(dd-MM-yyyy)</span>") + "<br />";
        foreach (QString url_part, (*iRes)->urls_parts_list){
            match = search.match((*iRes)->hostname+url_part);
            if(match.capturedLength()>0) bResMatch=true;

            results_txt_tmp += "<span style='color:grey;'>|-----"+url_part.toUtf8()+"</span><br />";
            displayed_tmp +=1;
            if( (displayed_tmp + displayed) >= MAX_DISPLAY ) break;
        }
        if(bResMatch){
            results_txt += results_txt_tmp;
            displayed += displayed_tmp;
        }
        if(displayed >= MAX_DISPLAY) break;
    }
    this->ui->label_results->setText(QString::number(displayed)+
        " lines displayed (total hostnames : "+QString::number(this->results.count())+")");
    this->ui->textBrowser_results->setText(results_txt);
}
