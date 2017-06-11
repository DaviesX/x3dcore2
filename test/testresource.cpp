#include <string>
#include "../src/resource.h"
#include "testresource.h"

test::test_resource::test_resource()
{
}

test::test_resource::~test_resource()
{
}

void
test::test_resource::run() const
{
        e8util::cornell_scene scene;
        std::vector<e8::if_geometry*> geos = scene.load_geometries();
        for (unsigned i = 0; i < geos.size(); i ++) {
                e8util::wavefront_obj obj(std::to_string(i) + ".obj");
                obj.save_geometries(std::vector<e8::if_geometry*>({geos[i]}));
                delete geos[i];
        }
}
