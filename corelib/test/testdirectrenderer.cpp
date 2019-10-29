#include "testdirectrenderer.h"
#include "src/frame.h"
#include "src/pipeline.h"

test::test_direct_renderer::test_direct_renderer() {}

test::test_direct_renderer::~test_direct_renderer() {}

void test::test_direct_renderer::run() const {
    unsigned const width = 800;
    unsigned const height = 600;

    e8::img_file_frame img("test_direct.png", width, height);
    e8::pt_render_pipeline pipeline(&img);

    e8util::flex_config config = pipeline.config_protocol();
    config.enum_sel["path_tracer"] = "direct";
    config.int_val["samples_per_frame"] = 10;
    pipeline.update_pipeline(config);

    pipeline.render_frame();
}
