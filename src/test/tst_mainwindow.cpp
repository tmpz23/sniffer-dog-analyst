#include <QtTest>

// add necessary includes here

class MainWindow : public QObject {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();
};

MainWindow::MainWindow()
{

}

MainWindow::~MainWindow()
{

}

void MainWindow::initTestCase()
{

}

void MainWindow::cleanupTestCase()
{

}

void MainWindow::test_case1()
{

}

QTEST_APPLESS_MAIN(MainWindow)
#include "tst_mainwindow.moc"
