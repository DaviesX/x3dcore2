#ifndef SCENE_H
#define SCENE_H

#include <set>
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
        intersect_info(bool valid, float t, e8util::vec3 const& vertex, e8util::vec3 const& normal, if_material const& mat):
                valid(valid), t(t), vertex(vertex), normal(normal), mat(mat)
        {
        }

        bool                    valid;
        float                   t;
        e8util::vec3 const&     vertex;
        e8util::vec3 const&     normal;
        if_material const&      mat;
};

typedef std::map<if_material const*, std::vector<if_geometry const*>>   batched_geometry;

class if_scene
{
public:
        if_scene();
        virtual ~if_scene();

        virtual void                            update() = 0;
        virtual intersect_info                  intersect(e8util::ray const& r) const = 0;
        virtual bool                            has_intersect(e8util::ray const& r, float t_min, float t_max, float& t) const = 0;
        virtual batched_geometry                get_relevant_geometries(e8util::frustum const& frustum) const = 0;
        virtual std::vector<if_light const*>    get_relevant_lights(e8util::frustum const& frustum) const = 0;

        std::vector<if_light const*>            get_lights() const;
        void                                    add_geometry(if_geometry const* geometry);
        void                                    add_material(if_material const* mat);
        void                                    add_light(if_light const* light);

        void                                    load(e8util::if_resource* res);
protected:
        std::set<if_geometry const*>    m_geometries;
        std::set<if_material const*>    m_mats;
        std::set<if_light const*>       m_lights;
};


class basic_scene: public if_scene
{
};

}

#endif // SCENE_H
