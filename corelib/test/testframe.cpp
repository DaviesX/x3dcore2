#include "testframe.h"
#include "src/compositor.h"
#include "src/frame.h"
#include <QImage>

test::test_frame::test_frame() {}

test::test_frame::~test_frame() {}

void test::test_frame::run() const {
    QImage qim(QString("test/Lenna.png"));

    e8::aces_compositor com(static_cast<unsigned>(qim.width()),
                            static_cast<unsigned>(qim.height()));

    for (unsigned j = 0; j < static_cast<unsigned>(qim.height()); j++) {
        for (unsigned i = 0; i < static_cast<unsigned>(qim.width()); i++) {
            e8::rgba_color &c = com(i, j);
            int r, g, b;
            qim.pixelColor(static_cast<int>(i), static_cast<int>(j)).getRgb(&r, &g, &b);
            c(0) = r / 255.0f;
            c(1) = g / 255.0f;
            c(2) = b / 255.0f;
            c(3) = 1.0f;
        }
    }

    e8::img_file_frame frame("Lenna2.png", com.width(), com.height());
    com.commit(&frame);
    frame.commit();
}
