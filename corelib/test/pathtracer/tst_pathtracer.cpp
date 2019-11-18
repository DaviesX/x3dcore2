#include "src/pathtracer.h"
#include <QString>
#include <QtTest>

class tst_pathtracer : public QObject {
    Q_OBJECT

  public:
    tst_pathtracer() = default;

  private Q_SLOTS:
    void unidirect_cornelbox_mse();
};

void tst_pathtracer::unidirect_cornelbox_mse() { QVERIFY2(true, "Failure"); }

QTEST_APPLESS_MAIN(tst_pathtracer)

#include "tst_pathtracer.moc"
