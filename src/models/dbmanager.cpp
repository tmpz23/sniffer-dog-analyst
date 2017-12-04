#include "dbmanager.h"

DbManager::DbManager(const QString &path){
    QFileInfo dbFile(path);
    bool bNewDb = false;

    if(!dbFile.exists() || !dbFile.isFile()){
        qDebug() << "no db found : creating new database";
        bNewDb = true;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()) qDebug() << "Error: connection with database fail";
    else qDebug() << "Database: connection ok";

    if(bNewDb) this->createDb();

    //m_db.exec("PRAGMA synchronous = OFF");
    //m_db.exec("PRAGMA journal_mode = MEMORY"); <- danger with results lost
    m_db.exec("PRAGMA locking_mode = EXCLUSIVE");
}
DbManager::~DbManager(){ if (m_db.isOpen()) m_db.close(); }
bool DbManager::isOpen() const { return m_db.isOpen(); }

bool DbManager::createDb(){
    QSqlQuery query;
    m_db.exec("BEGIN EXCLUSIVE");
    bool success = (
        query.exec("CREATE TABLE pivots_table (content TEXT NOT NULL);") &&
        query.exec("CREATE TABLE regex_table (content TEXT);") &&
        query.exec("CREATE TABLE results_table (id INTEGER PRIMARY KEY, date_add TEXT, hostname TEXT);") &&
        query.exec("CREATE TABLE urls_parts_table (id_result INTEGER, url_part TEXT, UNIQUE(id_result, url_part) ON CONFLICT REPLACE);") &&
        query.exec("CREATE TABLE proxies_table (ip TEXT, port TEXT, protocol TEXT);")
    );
    m_db.exec("COMMIT");

    return success;
}

bool DbManager::updateRegex(const QString& content){
   bool success = false;
   QSqlQuery query;

   query.prepare("INSERT INTO regex_table ( content ) VALUES ( :content )");
   query.bindValue(":content",content.simplified());

   m_db.exec("BEGIN EXCLUSIVE");
   this->deleteRegex(); 
   if(query.exec()) success = true;
   else qDebug() << "addRegex error:  " << query.lastError();
   m_db.exec("COMMIT");

   return success;
}

bool DbManager::updatePivots(const QList<Pivot*>& pivots){
   bool success = false;

   m_db.exec("BEGIN EXCLUSIVE");
   this->deletePivots();
   success = this->addPivots(pivots);
   m_db.exec("COMMIT");

   return success;
}

bool DbManager::updateResults(const QList<Result*>& results){
    bool success;

    m_db.exec("BEGIN EXCLUSIVE");
    this->deleteResults();
    success = this->addResults(results);
    m_db.exec("COMMIT");

    return success;
}

bool DbManager::updateProxies(const QList<Proxy*>& proxies){
    bool success;

    m_db.exec("BEGIN EXCLUSIVE");
    this->deleteProxies();
    success = this->addProxies( proxies );
    m_db.exec("COMMIT");

    return success;
}

bool DbManager::addPivots(const QList<Pivot*>& pivots){
   bool success = true;
   QSqlQuery query;
   query.prepare("INSERT INTO pivots_table (content) VALUES (:content)");

   for(QList<Pivot*>::const_iterator p = pivots.begin(); p != pivots.end(); ++p){
       query.bindValue(":content", (*p)->pivot );
       if(!query.exec()) {
        success = false;
        qDebug() << "addPivots error: " << query.lastError();
       }
   }
   return success;
}

bool DbManager::addResults(const QList<Result*>& results){
   bool success = true;
   int id_result;
   QSqlQuery query;
   QSqlQuery query2;
   query.prepare("INSERT INTO results_table (date_add, hostname) VALUES (:date_add, :hostname)");
   query2.prepare("INSERT INTO urls_parts_table (id_result, url_part) VALUES (:id_result, :url_part)");

   for(QList<Result*>::const_iterator iRes = results.begin(); iRes != results.end(); ++iRes){
       query.bindValue(":date_add", (*iRes)->date_add.toString("yyyy-MM-dd") );
       query.bindValue(":hostname", (*iRes)->hostname );

       if(query.exec()){
           id_result = query.lastInsertId().toInt();

           foreach(QString url_part, (*iRes)->urls_parts_list){
               query2.bindValue(":id_result", id_result );
               query2.bindValue(":url_part", url_part );
               if(!query2.exec()){
                    success = false;
                    qDebug() << "addResults url_parts error: " << query2.lastError();
                    qDebug() << url_part;
               }
           }
       }else{
           success = false;
           qDebug() << "addResults error:  " << query.lastError();
       }
   }
   return success;
}

bool DbManager::addProxies(const QList<Proxy*>& proxies){
   bool success = true;
   QSqlQuery query;

   query.prepare("INSERT INTO proxies_table (ip, port, protocol) VALUES (:ip, :port, :protocol)");

   for(QList<Proxy*>::const_iterator p = proxies.begin(); p != proxies.end(); ++p){
       //qDebug() << "ip: "+p.ip +" port: "+ QString::number(p.port) +" protocol: "+ p.protocol;
       query.bindValue(":ip", (*p)->ip );
       query.bindValue(":port", (*p)->port );
       query.bindValue(":protocol", (*p)->protocol );
       if(!query.exec()) {
        success = false;
        qDebug() << "addResults error:  " << query.lastError();
       }
   }
   return success;
}

QString DbManager::getRegex(){
   QSqlQuery query("SELECT content FROM regex_table");
   return query.next() ? query.value(0).toString() : QString("");
}

void DbManager::getPivots(QList<Pivot*>& pivots){
    QSqlQuery query;

    if(pivots.count() > 0){
      Pivot* _pivot;
      QMutableListIterator<Pivot*> mi(pivots);
      while(mi.hasNext()){
          _pivot = mi.next();
          mi.remove();
          delete _pivot;
      }
    }

    query.exec("SELECT content FROM pivots_table");
    while(query.next())
         pivots << new Pivot(query.value(0).toString());
}

void DbManager::getResults(QList<Result*>& results){
    QSqlQuery query;
    QSqlQuery query2;

    if(results.count() > 0){
      Result* _result;
      QMutableListIterator<Result*> mi(results);
      while(mi.hasNext()){
          _result = mi.next();
          mi.remove();
          delete _result;
      }
    }

   query2.prepare("SELECT * FROM urls_parts_table WHERE id_result = :id_result");

   query.exec("SELECT * FROM results_table");
   while (query.next()){
        QStringList urls_parts;

        // Get urls_parts
        query2.bindValue(":id_result", query.value(0).toInt());
        query2.exec();
        while(query2.next()) urls_parts << QString(query2.value(1).toString());

        // append the result to the list
        results << new Result(
           query.value(0).toInt(), // id
           QDate().fromString(query.value(1).toString(), "yyyy-MM-dd"), // date_add
           query.value(2).toString(), // hostname
           urls_parts // urls_parts_list
        );
   }
}

void DbManager::getProxies(QList<Proxy*>& proxies){
    QSqlQuery query;

    if(proxies.count() > 0){
      Proxy* _proxy;
      QMutableListIterator<Proxy*> mi(proxies);
      while(mi.hasNext()){
          _proxy = mi.next();
          mi.remove();
          delete _proxy;
      }
    }

    query.exec("SELECT * FROM proxies_table");
    while(query.next()) {
         proxies << new Proxy(
            query.value(0).toString(), // ip
            query.value(1).toString(), // port
            query.value(2).toString(), // protocol
            false // googlecacheCaptcha
         );
    }
}

void DbManager::deleteRegex(){
   QSqlQuery query;
   query.prepare("DELETE FROM regex_table");
   query.exec();
}

void DbManager::deletePivots(){
   QSqlQuery query;
   query.prepare("DELETE FROM pivots_table");
   query.exec();
}

void DbManager::deleteResults(){
   QSqlQuery query;
   query.prepare("DELETE FROM results_table");
   query.exec();
   query.prepare("DELETE FROM urls_parts_table");
   query.exec();
}

void DbManager::deleteProxies(){
    QSqlQuery query;
    query.prepare("DELETE FROM proxies_table");
    query.exec();
}
