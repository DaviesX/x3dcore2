#include "src/pathspace.h"
#include "src/pathtracer.h"
#include "src/resource.h"
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

void tst_pathtracer::unidirect_positive_radiance() {}

QTEST_APPLESS_MAIN(tst_pathtracer)

#include "tst_pathtracer.moc"
