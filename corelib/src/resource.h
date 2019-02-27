#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <vector>
#include "geometry.h"
#include "material.h"
#include "light.h"
#include "camera.h"


namespace e8util
{

class if_resource
{
public:
        if_resource();
        virtual ~if_resource();

        virtual std::vector<e8::if_geometry*>           load_geometries() const;
        virtual std::vector<e8::if_material*>           load_materials() const;
        virtual std::vector<e8::if_light*>              load_lights() const;
        virtual e8::if_camera*                          load_camera() const;
        virtual bool                                    save_geometries(std::vector<e8::if_geometry*> const& geometries);
};

class cornell_scene: public if_resource
{
public:
        cornell_scene();
        std::vector<e8::if_geometry*>   load_geometries() const override;
        std::vector<e8::if_material*>   load_materials() const override;
        std::vector<e8::if_light*>      load_lights() const override;
        e8::if_camera*                  load_camera() const override;
};

class wavefront_obj: public if_resource
{
public:
        wavefront_obj(std::string const& location);
        ~wavefront_obj() override;

        std::vector<e8::if_geometry*>   load_geometries() const override;
        bool                            save_geometries(std::vector<e8::if_geometry*> const& geometries) override;
private:
        std::string     m_location;
};


}

#endif // RESOURCE_H
