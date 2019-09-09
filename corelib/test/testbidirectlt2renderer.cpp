#include "testbidirectlt2renderer.h"
#include "src/frame.h"
#include "src/pipeline.h"

test::test_bidirect_lt2_renderer::test_bidirect_lt2_renderer() {}

test::test_bidirect_lt2_renderer::~test_bidirect_lt2_renderer() {}

void test::test_bidirect_lt2_renderer::run() const {
    unsigned const width = 800;
    unsigned const height = 600;

    e8::img_file_frame img("test_bidirect_lt2.png", width, height);
    e8::pt_render_pipeline pipeline(&img);

    e8util::flex_config config = pipeline.config_protocol();
    config.enum_sel["path_tracer"] = "bidirectional_lt2";
    config.int_val["samples_per_frame"] = 10;
    pipeline.update_pipeline(config);

    pipeline.render_frame();
}
