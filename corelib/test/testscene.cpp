#include "testscene.h"
#include "src/frame.h"
#include "src/pipeline.h"
#include <iostream>

test::test_path_space::test_path_space() {}

test::test_path_space::~test_path_space() {}

void test::test_path_space::run() const {
    unsigned const width = 1024;
    unsigned const height = 768;

    e8::img_file_frame img("test_bidirect.png", width, height);
    e8::pt_render_pipeline pipeline(&img);

    e8util::flex_config config = pipeline.config_protocol();
    config.enum_sel["path_space"] = "static_bvh";
    config.enum_sel["path_tracer"] = "normal";
    config.int_val["samples_per_frame"] = 5;
    pipeline.update_pipeline(config);

    pipeline.render_frame();

    e8::bvh_path_space_layout *scene = static_cast<e8::bvh_path_space_layout *>(
        pipeline.objdb().manager_of(e8::obj_protocol::obj_protocol_geometry));
    std::cout << "max_depth: " << scene->max_depth() << std::endl;
    std::cout << "avg_depth: " << scene->avg_depth() << std::endl;
    std::cout << "dev_depth: " << scene->dev_depth() << std::endl;
    std::cout << "num_nodes: " << scene->num_nodes() << std::endl;
}
