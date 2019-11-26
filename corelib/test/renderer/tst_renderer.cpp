#include <QtTest>

// add necessary includes here

class cornellbox_rendering : public QObject
{
    Q_OBJECT

public:
    cornellbox_rendering();
    ~cornellbox_rendering();

private slots:
    void test_case1();

};

cornellbox_rendering::cornellbox_rendering()
{

}

cornellbox_rendering::~cornellbox_rendering()
{

}

void cornellbox_rendering::test_case1()
{

}

QTEST_APPLESS_MAIN(cornellbox_rendering)

#include "tst_cornellbox_rendering.moc"
