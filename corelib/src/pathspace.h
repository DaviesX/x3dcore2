#ifndef SCENE_H
#define SCENE_H

#include "geometry.h"
#include "light.h"
#include "material.h"
#include "obj.h"
#include "tensor.h"
#include <map>
#include <memory>
#include <stdint.h>
#include <utility>
#include <vector>

namespace e8 {

struct intersect_info {
    intersect_info(float t, e8util::vec3 const &vertex, e8util::vec3 const &normal,
                   e8util::vec2 const &uv, if_material const *mat, if_light const *light)
        : valid(true), t(t), vertex(vertex), normal(normal), uv(uv), mat(mat), light(light) {}

    intersect_info() : valid(false), mat(nullptr), light(nullptr) {}

    bool valid;
    float t;
    e8util::vec3 vertex;
    e8util::vec3 normal;
    e8util::vec2 uv;
    if_material const *mat;
    if_light const *light;
};

typedef std::map<if_material const *, std::vector<if_geometry const *>> batched_geometry;

class if_path_space : public if_obj_manager {
  public:
    if_path_space();
    virtual ~if_path_space() override;

    virtual void commit() override = 0;
    virtual intersect_info intersect(e8util::ray const &r) const = 0;
    virtual bool has_intersect(e8util::ray const &r, float t_min, float t_max, float &t) const = 0;
    virtual batched_geometry get_relevant_geometries(e8util::frustum const &frustum) const = 0;
    virtual std::vector<if_light const *>
    get_relevant_lights(e8util::frustum const &frustum) const = 0;

    //        void                                    add_geometry(std::unique_ptr<if_geometry>&
    //        geometry);
    //        void                                    bind(if_geometry const* geometry, if_material
    //        const* mat);
    //        void                                    bind(if_geometry const* geometry, if_light
    //        const* light);
    e8util::aabb aabb() const;

    void load(if_obj const &obj, e8util::mat44 const &trans) override;
    obj_protocol support() const override;
    void unload(if_obj const &obj) override;

  protected:
    struct binded_geometry {
        binded_geometry(std::unique_ptr<if_geometry const> &geometry,
                        std::unique_ptr<if_material const> &mat,
                        std::unique_ptr<if_light const> &light)
            : geometry(std::move(geometry)), mat(std::move(mat)), light(std::move(light)) {}

        std::unique_ptr<if_geometry const> geometry;
        std::unique_ptr<if_material const> mat;
        std::unique_ptr<if_light const> light;
    };

    std::map<obj_id_t, binded_geometry> m_geometries;
    e8util::aabb m_bound;
};

class linear_path_space_layout : public if_path_space {
  public:
    linear_path_space_layout();
    ~linear_path_space_layout() override;

    virtual void commit() override;
    virtual intersect_info intersect(e8util::ray const &r) const override;
    virtual bool has_intersect(e8util::ray const &r, float t_min, float t_max,
                               float &t) const override;
    virtual batched_geometry get_relevant_geometries(e8util::frustum const &frustum) const override;
    virtual std::vector<if_light const *>
    get_relevant_lights(e8util::frustum const &frustum) const override;
};

class bvh_path_space_layout : public linear_path_space_layout {
  public:
    bvh_path_space_layout();
    ~bvh_path_space_layout() override;

    void commit() override;
    intersect_info intersect(e8util::ray const &r) const override;
    bool has_intersect(e8util::ray const &r, float t_min, float t_max, float &t) const override;

    unsigned max_depth() const;
    float avg_depth() const;
    float dev_depth() const;
    unsigned num_nodes() const;

  private:
    struct primitive {
        primitive(triangle const &tri, unsigned i_geo, unsigned short i_mat, unsigned short i_light)
            : tri(tri), i_geo(i_geo), i_mat(i_mat), i_light(i_light) {}

        triangle tri;
        unsigned int i_geo;
        unsigned short i_mat;
        unsigned short i_light;
    };

    struct primitive_details : public primitive {
        primitive_details(triangle const &tri, e8::if_geometry const *geo, unsigned i_geo,
                          unsigned short i_mat, unsigned short i_light)
            : primitive(tri, i_geo, i_mat, i_light) {
            std::vector<e8util::vec3> const &verts = geo->vertices();
            e8util::vec3 const &v0 = verts[tri(0)];
            e8util::vec3 const &v1 = verts[tri(1)];
            e8util::vec3 const &v2 = verts[tri(2)];
            bound = bound + v0;
            bound = bound + v1;
            bound = bound + v2;

            centroid = (bound.max() + bound.min()) / 2.0f;

            surf_area = 0.5f * (v1 - v0).outer(v2 - v0).norm();
        }

        float surf_area;
        e8util::aabb bound;
        e8util::vec3 centroid;
    };

    struct node {
        node(e8util::aabb const &bound, unsigned prim_start, unsigned char num_prims)
            : bound(bound), split_axis(static_cast<uint8_t>(-1)), prim_start(prim_start),
              num_prims(num_prims) {
            children[0] = nullptr;
            children[1] = nullptr;
        }

        node(e8util::aabb const &bound, unsigned char split_axis, node *left, node *right)
            : bound(bound), split_axis(split_axis), prim_start(static_cast<uint8_t>(-1)),
              num_prims(0) {
            children[0] = left;
            children[1] = right;
        }

        e8util::aabb bound;
        node *children[2];
        uint8_t split_axis;
        uint32_t prim_start;
        uint8_t num_prims;
    };

    struct flattened_node {
        flattened_node() {}

        flattened_node(e8util::aabb const &bound, unsigned char split_axis, unsigned next_child,
                       unsigned)
            : bound(bound), num_prims(0), split_axis(split_axis), next_child(next_child),
              prim_start(0XFFFFFFFF) {}

        flattened_node(e8util::aabb const &bound, unsigned prim_start, unsigned char num_prims)
            : bound(bound), num_prims(num_prims), split_axis(0XFF), next_child(0XFFFFFFFF),
              prim_start(prim_start) {}

        e8util::aabb bound;
        unsigned char num_prims;
        unsigned char split_axis;
        unsigned char __pad;
        unsigned next_child;
        unsigned prim_start;
    };

    struct bucket {
        bucket() {}

        float cost = 0.0f;
        unsigned num_prims = 0;
        e8util::aabb bound;
    };

    std::vector<bucket> sah_buckets(std::vector<primitive_details> const &prims, unsigned start,
                                    unsigned end, unsigned axis, e8util::aabb const &bound,
                                    e8util::vec3 const &range);
    e8util::aabb bound(std::vector<primitive_details> const &prims, unsigned start, unsigned end);
    node *bvh(std::vector<primitive_details> &prims, unsigned start, unsigned end, unsigned depth);
    void delete_bvh(node *bvh, unsigned depth);
    void flatten(std::vector<flattened_node> &bvh, node *bvh_node);

    unsigned m_max_depth = 0;
    unsigned m_sum_depth2 = 0;
    unsigned m_sum_depth = 0;
    unsigned m_num_paths = 0;
    unsigned m_num_nodes = 0;

    std::vector<flattened_node> m_bvh;

    std::vector<primitive> m_prims;
    std::vector<if_geometry const *> m_geo_list;
    std::vector<if_material const *> m_mat_list;
    std::vector<if_light const *> m_light_list;
};

} // namespace e8

#endif // SCENE_H
