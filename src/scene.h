#ifndef SCENE_H
#define SCENE_H

#include <map>
#include "tensor.h"
#include "geometry.h"
#include "material.h"
#include "resource.h"
#include "light.h"

namespace e8
{

struct intersect_info
{
        bool                    valid;
        float                   t;
        e8util::vec3 const&     vertex;
        e8util::vec3 const&     normal;
        if_material const&      mat;
};

class if_scene
{
public:
        if_scene();

        virtual void                    update() = 0;
        virtual intersect_info          intersect(e8util::ray const& r) const = 0;
        virtual bool                    has_intersect(e8util::ray const& r, float t_min, float t_max, float& t) const = 0;

        virtual std::map<if_material const*,
                         if_geometry const*>    get_relevant_geometry(e8util::frustum const& frustum) const = 0;

        void                            add_geometry(if_geometry const* geometry);
        void                            add_material(if_material const* mat);
        void                            add_light(if_light const* light);
        void                            bind_material(if_geometry const* geometry, if_material const* mat);

        void                            load(e8util::if_resource* res);
};

}

#endif // SCENE_H
