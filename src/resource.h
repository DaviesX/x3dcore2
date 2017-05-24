#ifndef RESOURCE_H
#define RESOURCE_H

#include <vector>
#include "geometry.h"
#include "material.h"
#include "light.h"

namespace e8util
{

class if_resource
{
public:
        if_resource();
        virtual ~if_resource();

        virtual std::vector<e8::if_geometry*>           load_geometries() = 0;
        virtual std::vector<e8::if_material*>           load_materials() = 0;
        virtual std::vector<e8::if_light*>              load_lights() = 0;
};

class cornell_scene: public if_resource
{
public:
        std::vector<e8::if_geometry*>           load_geometries() override;
        std::vector<e8::if_material*>           load_materials() override;
        std::vector<e8::if_light*>              load_lights() override;
};

}

#endif // RESOURCE_H
