#include "resulttest.h"

void ResultTest::getReverseHostname(){
    Result* result = new Result(0,QDate::currentDate(),"",QStringList());

    result->hostname = QUrl("http://1.2.3.4").host();
    QVERIFY(result->getReverseHostname() == "4.3.2.1");

    result->hostname = QUrl("ftp://1.2.3.4/.12").host();
    QVERIFY(result->getReverseHostname() == "4.3.2.1");

    result->hostname = QUrl("https://a.b.c.d/.12").host();
    QVERIFY(result->getReverseHostname() == "d.c.b.a");

    result->hostname = QUrl("https://a-csfaefglkn.d.e/.12\\+))=654.9/").host();
    QVERIFY(result->getReverseHostname() == "e.d.a-csfaefglkn");

    // Unicode and so on ?

    delete result;
}
void ResultTest::operator_equal(){
    Result* resultA = new Result(0,QDate::currentDate(),"",QStringList());
    Result* resultB = new Result(0,QDate::currentDate(),"",QStringList());

    resultA->hostname = QUrl("http://1.2.3.4").host();
    bool bRes = (*resultA == *resultB);
    QVERIFY(bRes == false);

    resultB->hostname = QUrl("http://1.2.3.4").host();
    bRes = (*resultA == *resultB);
    QVERIFY(bRes == true);

    resultA->hostname = QUrl("http://a.b.c.d").host();
    bRes = (*resultA == *resultB);
    QVERIFY(bRes == false);

    resultA->hostname = QUrl("http://a-csfaefglkn.d.e").host();
    resultB->hostname = QUrl("http://a-csfaefglkn.d.e").host();
    bRes = (*resultA == *resultB);
    QVERIFY(bRes == true);

    delete resultA;
    delete resultB;
}
void ResultTest::operator_lt(){
    Result* resultA = new Result(0,QDate::currentDate(),"",QStringList());
    Result* resultB = new Result(0,QDate::currentDate(),"",QStringList());

    resultA->hostname = QUrl("http://9.9.9.4").host();
    resultB->hostname = QUrl("http://1.1.1.5").host();
    bool bRes = (*resultA < *resultB);
    QVERIFY(bRes == true);

    resultB->hostname = QUrl("http://1.1.4.4").host();
    bRes = (*resultA < *resultB);
    QVERIFY(bRes == false);

    resultA->hostname = QUrl("http://z.b.c.d").host();
    bRes = (*resultA < *resultB);
    QVERIFY(bRes == false);

    resultB->hostname = QUrl("http://a.b.c.e").host();
    bRes = (*resultA < *resultB);
    QVERIFY(bRes == true);

    delete resultA;
    delete resultB;
}
void ResultTest::operator_gt(){
    Result* resultA = new Result(0,QDate::currentDate(),"",QStringList());
    Result* resultB = new Result(0,QDate::currentDate(),"",QStringList());

    resultA->hostname = QUrl("http://9.9.9.4").host();
    resultB->hostname = QUrl("http://1.1.1.5").host();
    bool bRes = (*resultA > *resultB);
    QVERIFY(bRes == false);

    resultB->hostname = QUrl("http://1.1.4.4").host();
    bRes = (*resultA > *resultB);
    QVERIFY(bRes == true);

    resultA->hostname = QUrl("http://z.b.c.d").host();
    bRes = (*resultA > *resultB);
    QVERIFY(bRes == true);

    resultB->hostname = QUrl("http://a.b.c.e").host();
    bRes = (*resultA > *resultB);
    QVERIFY(bRes == false);

    delete resultA;
    delete resultB;
}

QTEST_MAIN(ResultTest)
#include "moc_resulttest.cpp"
