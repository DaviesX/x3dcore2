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
        intersect_info(float t, e8util::vec3 const& vertex, e8util::vec3 const& normal, if_material const* mat, if_light const* light):
                valid(true), t(t), vertex(vertex), normal(normal), mat(mat), light(light)
        {
        }

        intersect_info():
                valid(false)
        {
        }

        bool                    valid;
        float                   t;
        e8util::vec3            vertex;
        e8util::vec3            normal;
        if_material const*      mat;
        if_light const*         light;
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
        virtual if_light const*                 sample_light(float& pdf) const = 0;

        void                                    add_geometry(if_geometry const* geometry);
        void                                    add_material(if_material const* mat);
        void                                    add_light(if_light const* light);
        void                                    bind(if_geometry const* geometry, if_material const* mat);
        void                                    bind(if_geometry const* geometry, if_light const* light);

        void                                    load(e8util::if_resource* res);
protected:
        struct binded_geometry
        {
                binded_geometry(if_geometry const* geometry, if_material const* mat, if_light const* light):
                        geometry(geometry), mat(mat), light(light)
                {
                }

                if_geometry const*      geometry;
                if_material const*      mat;
                if_light const*         light;
        };

        std::map<if_geometry const*, binded_geometry>   m_geometries;
        std::set<if_material const*>                    m_mats;
        std::set<if_light const*>                       m_lights;
};



class linear_scene_layout: public if_scene
{
public:
        linear_scene_layout();
        ~linear_scene_layout();

        void                            update() override;
        intersect_info                  intersect(e8util::ray const& r) const override;
        bool                            has_intersect(e8util::ray const& r, float t_min, float t_max, float& t) const override;
        batched_geometry                get_relevant_geometries(e8util::frustum const& frustum) const override;
        std::vector<if_light const*>    get_relevant_lights(e8util::frustum const& frustum) const override;
        if_light const*                 sample_light(float& pdf) const override;
private:
        std::vector<if_light const*>    m_light_list;
        std::vector<float>              m_cum_power;
        float                           m_total_power;
};


class bvh_scene_layout: public if_scene
{
public:
        bvh_scene_layout();
        ~bvh_scene_layout();
private:
        struct primitive
        {
                triangle        tri;
                unsigned short  i_mat;
                unsigned short  i_light;
                unsigned int    i_geo;
        };

        struct node
        {
               std::vector<primitive>   m_prims;
               e8util::aabb             m_bound;
               node*                    m_children[2];
        };

        std::vector<if_geometry const*>         m_geos;
        std::vector<if_material const*>         m_mats;
        std::vector<if_light const*>            m_lights;
};

}

#endif // SCENE_H
