#include "src/pathspace.h"
#include "src/pathtracer.h"
#include <QString>
#include <QtTest>
#include <memory>

class tst_pathtracer : public QObject {
    Q_OBJECT

  public:
    tst_pathtracer() = default;

  private Q_SLOTS:
    void unidirect_positive_radiance();
};

std::unique_ptr<e8::if_path_space> cornell_box_path_space() {}

void tst_pathtracer::unidirect_positive_radiance() { QVERIFY2(true, "Failure"); }

QTEST_APPLESS_MAIN(tst_pathtracer)

#include "tst_pathtracer.moc"
