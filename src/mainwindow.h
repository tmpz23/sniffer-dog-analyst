#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "class/searchorchestrator.h"
#include "models/dbmanager.h"
#include "models/proxyqlistmodel.h"

#include <QMainWindow>
#include <QMutableListIterator>
#include <QFileDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        ProxyQListModel* pModel;
        QRegularExpression regex;
        QList<Pivot*> pivots;
        QList<Result*> results;
        QList<Proxy*> proxies;

        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;
        DbManager* dbManager;
        bool bPause;
        SearchOrchestrator* orchestrator;

        void loadDb();
        void showPivots();
        void showProxies();
        void showResults();

    private slots:
        void onStartClicked();
        void onPauseClicked();
        void onStopClicked();

        void onOrchestratorFinish();
        void onSetNbScrapper(int);
        void onSetProgress(int);
        void onProxiesChange();

        void onUpdatePivotsRegex();
        void onUpdateProxies();
        void onAppendLog(QString,LogColor);

        void onResetResults();
        void onExportResults();
        void onSearchResults();
};

#endif // MAINWINDOW_H
