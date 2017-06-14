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
        virtual if_light const*                 sample_light(e8util::rng& rng, float& pdf) const = 0;

        void                                    add_geometry(if_geometry const* geometry);
        void                                    add_material(if_material const* mat);
        void                                    add_light(if_light const* light);
        void                                    bind(if_geometry const* geometry, if_material const* mat);
        void                                    bind(if_geometry const* geometry, if_light const* light);
        e8util::aabb                            aabb() const;

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
        e8util::aabb                                    m_bound;
};



class linear_scene_layout: public if_scene
{
public:
        linear_scene_layout();
        ~linear_scene_layout();

        virtual void                            update() override;
        virtual intersect_info                  intersect(e8util::ray const& r) const override;
        virtual bool                            has_intersect(e8util::ray const& r, float t_min, float t_max, float& t) const override;
        virtual batched_geometry                get_relevant_geometries(e8util::frustum const& frustum) const override;
        virtual std::vector<if_light const*>    get_relevant_lights(e8util::frustum const& frustum) const override;
        virtual if_light const*                 sample_light(e8util::rng& rng, float& pdf) const override;
private:
        std::vector<if_light const*>    m_light_list;
        std::vector<float>              m_cum_power;
        float                           m_total_power;
};


class bvh_scene_layout: public linear_scene_layout
{
public:
        bvh_scene_layout();
        ~bvh_scene_layout();

        void                    update() override;
        intersect_info          intersect(e8util::ray const& r) const override;
        bool                    has_intersect(e8util::ray const& r, float t_min, float t_max, float& t) const override;

        unsigned                max_depth() const;
        float                   avg_depth() const;
        float                   dev_depth() const;
        unsigned                num_nodes() const;
private:
        struct primitive
        {
                primitive(triangle const& tri, unsigned i_geo, unsigned short i_mat, unsigned short i_light):
                        tri(tri), i_geo(i_geo), i_mat(i_mat), i_light(i_light)
                {
                }

                triangle        tri;
                unsigned int    i_geo;
                unsigned short  i_mat;
                unsigned short  i_light;
        };

        struct primitive_details: public primitive
        {
                primitive_details(triangle const& tri, e8::if_geometry const* geo, unsigned i_geo, unsigned short i_mat, unsigned short i_light):
                        primitive(tri, i_geo, i_mat, i_light)
                {
                        std::vector<e8util::vec3> const& verts = geo->vertices();
                        e8util::vec3 const& v0 = verts[tri(0)];
                        e8util::vec3 const& v1 = verts[tri(1)];
                        e8util::vec3 const& v2 = verts[tri(2)];
                        bound = bound + v0;
                        bound = bound + v1;
                        bound = bound + v2;

                        centroid = (bound.max() + bound.min())/2.0f;

                        surf_area = 0.5f*(v1 - v0).outer(v2 - v0).norm();
                }

                float           surf_area;
                e8util::aabb    bound;
                e8util::vec3    centroid;
        };

        struct node
        {
                node(e8util::aabb const& bound, unsigned prim_start, unsigned char num_prims):
                        bound(bound), split_axis(-1), prim_start(prim_start), num_prims(num_prims)
                {
                        children[0] = nullptr;
                        children[1] = nullptr;
                }

                node(e8util::aabb const& bound, unsigned char split_axis, node* left, node* right):
                        bound(bound), split_axis(split_axis), prim_start(-1), num_prims(0)
                {
                        children[0] = left;
                        children[1] = right;
                }

                e8util::aabb    bound;
                node*           children[2];
                unsigned char   split_axis;
                unsigned        prim_start;
                unsigned char   num_prims;
        };

        struct flattened_node
        {
                flattened_node()
                {
                }

                flattened_node(e8util::aabb const& bound, unsigned char split_axis, unsigned next_child, unsigned):
                        bound(bound), num_prims(0), split_axis(split_axis), next_child(next_child), prim_start(-1)
                {
                }

                flattened_node(e8util::aabb const& bound, unsigned prim_start, unsigned char num_prims):
                        bound(bound), num_prims(num_prims), split_axis(-1), next_child(-1), prim_start(prim_start)
                {
                }

                e8util::aabb    bound;
                unsigned char   num_prims;
                unsigned char   split_axis;
                unsigned char   __pad;
                unsigned        next_child;
                unsigned        prim_start;
        };

        struct bucket
        {
                bucket()
                {
                }

                float           cost = 0.0f;
                unsigned        num_prims = 0;
                e8util::aabb    bound;
        };

        std::vector<bucket>     sah_buckets(std::vector<primitive_details> const& prims, unsigned start, unsigned end,
                                             unsigned axis, e8util::aabb const& bound, e8util::vec3 const& range);
        e8util::aabb            bound(std::vector<primitive_details> const& prims, unsigned start, unsigned end);
        node*                   bvh(std::vector<primitive_details>& prims, unsigned start, unsigned end, unsigned depth);
        void                    delete_bvh(node* bvh, unsigned depth);
        void                    flatten(std::vector<flattened_node>& bvh, node* bvh_node);

        unsigned                                m_max_depth = 0;
        unsigned                                m_sum_depth2 = 0;
        unsigned                                m_sum_depth = 0;
        unsigned                                m_num_paths = 0;
        unsigned                                m_num_nodes = 0;

        std::vector<flattened_node>             m_bvh;

        std::vector<primitive>                  m_prims;
        std::vector<if_geometry const*>         m_geo_list;
        std::vector<if_material const*>         m_mat_list;
        std::vector<if_light const*>            m_light_list;
};

}

#endif // SCENE_H
