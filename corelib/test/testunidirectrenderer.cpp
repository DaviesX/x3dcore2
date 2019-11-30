#include "testunidirectrenderer.h"
#include "src/frame.h"
#include "src/pipeline.h"

test::test_unidirect_renderer::test_unidirect_renderer() {}

test::test_unidirect_renderer::~test_unidirect_renderer() {}

void test::test_unidirect_renderer::run() const {
    unsigned const width = 800;
    unsigned const height = 600;

    e8::img_file_frame img("test_unidirect.png", width, height);
    e8::pt_render_pipeline pipeline(&img);

    e8util::flex_config config = pipeline.config_protocol();
    config.enum_sel["path_tracer"] = "unidirectional";
    config.int_val["samples_per_frame"] = 128;
    config.bool_val["firefly_filter"] = false;
    pipeline.update_pipeline(config);

    pipeline.render_frame();
}
