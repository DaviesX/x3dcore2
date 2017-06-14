#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "src/compositor.h"
#include "src/frame.h"
#include "testframe.h"

test::test_frame::test_frame()
{
}

test::test_frame::~test_frame()
{
}

void
test::test_frame::run() const
{
        cv::Mat img = cv::imread("test/Lenna.png", cv::IMREAD_COLOR);
        cv::Mat3f fimg;
        img.convertTo(fimg, CV_32FC3, 1.0f/255.0f);

        e8::aces_compositor com(fimg.size().width, fimg.size().height);
        fimg.forEach([&com](cv::Vec3f const& v, int const* p) {
                e8::rgba_color& c = com(p[1], p[0]);
                c(0) = v[2];
                c(1) = v[1];
                c(2) = v[0];
                c(3) = 1.0f;
        });

        e8::img_file_frame frame("Lenna2.png", com.width(), com.height());
        com.commit(&frame);
        frame.commit();
}
