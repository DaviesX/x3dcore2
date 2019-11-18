#include "light.h"
#include "lightsources.h"
#include "material.h"
#include "pathtracer.h"
#include <algorithm>
#include <ext/alloc_traits.h>
#include <memory>

e8::if_path_space::if_path_space() {}

e8::if_path_space::~if_path_space() {}

e8util::aabb e8::if_path_space::aabb() const { return m_bound; }

void e8::if_path_space::load(if_obj const &obj, e8util::mat44 const &trans) {
    std::unique_ptr<if_geometry const> geo = static_cast<if_geometry const &>(obj).transform(trans);
    std::vector<if_obj *> mats = obj.get_children(obj_protocol::obj_protocol_material);
    std::vector<if_obj *> lights = obj.get_children(obj_protocol::obj_protocol_light);

    assert(mats.size() == 1);
    std::unique_ptr<if_material const> mat = static_cast<if_material *>(mats[0])->copy();

    std::unique_ptr<if_light const> light;
    if (!lights.empty()) {
        assert(lights.size() == 1);
        light = static_cast<if_light *>(lights[0])->copy();
    }

    m_bound = m_bound + geo->aabb();
    m_geometries.insert(std::make_pair(obj.id(), binded_geometry(geo, mat, light)));
}

void e8::if_path_space::unload(if_obj const &obj) {
    auto it = m_geometries.find(obj.id());
    if (it != m_geometries.end()) {
        m_geometries.erase(it);
    }
}

e8::obj_protocol e8::if_path_space::support() const { return obj_protocol::obj_protocol_geometry; }

e8::linear_path_space_layout::linear_path_space_layout() {}

e8::linear_path_space_layout::~linear_path_space_layout() {}

void e8::linear_path_space_layout::commit() {}

e8::intersect_info e8::linear_path_space_layout::intersect(e8util::ray const &r) const {
    float const t_min = 1e-4f;
    float const t_max = 1000.0f;

    float t = INFINITY;
    binded_geometry const *hit_geo = nullptr;
    triangle const *hit_tri = nullptr;
    e8util::vec3 hit_b;

    for (auto it = m_geometries.begin(); it != m_geometries.end(); ++it) {
        if_geometry const *geo = it->second.geometry.get();

        std::vector<e8util::vec3> const &verts = geo->vertices();
        std::vector<triangle> const &tris = geo->triangles();

        for (triangle const &tri : tris) {
            e8util::vec3 const &v0 = verts[tri(0)];
            e8util::vec3 const &v1 = verts[tri(1)];
            e8util::vec3 const &v2 = verts[tri(2)];

            float t0;
            e8util::vec3 b;
            if (r.intersect(v0, v1, v2, t_min, t_max, b, t0) && t0 < t) {
                hit_b = b;
                hit_geo = &it->second;
                hit_tri = &tri;
                t = t0;
            }
        }
    }

    if (hit_geo != nullptr) {
        std::vector<e8util::vec3> const &verts = hit_geo->geometry->vertices();
        e8util::vec3 const &v0 = verts[(*hit_tri)(0)];
        e8util::vec3 const &v1 = verts[(*hit_tri)(1)];
        e8util::vec3 const &v2 = verts[(*hit_tri)(2)];
        e8util::vec3 const &vertex = hit_b(0) * v0 + hit_b(1) * v1 + hit_b(2) * v2;

        std::vector<e8util::vec3> const &normals = hit_geo->geometry->normals();
        e8util::vec3 const &n0 = normals[(*hit_tri)(0)];
        e8util::vec3 const &n1 = normals[(*hit_tri)(1)];
        e8util::vec3 const &n2 = normals[(*hit_tri)(2)];
        e8util::vec3 const &normal = (hit_b(0) * n0 + hit_b(1) * n1 + hit_b(2) * n2).normalize();

        std::vector<e8util::vec2> const &texcoords = hit_geo->geometry->texcoords();
        e8util::vec2 uv;
        if (!texcoords.empty()) {
            e8util::vec2 uv0 = texcoords[(*hit_tri)(0)];
            e8util::vec2 uv1 = texcoords[(*hit_tri)(1)];
            e8util::vec2 uv2 = texcoords[(*hit_tri)(2)];
            uv = (hit_b(0) * uv0 + hit_b(1) * uv1 + hit_b(2) * uv2).normalize();
        }
        return intersect_info(t, vertex, normal, uv, hit_geo->mat.get(), hit_geo->light.get());
    } else {
        return intersect_info();
    }
}

bool e8::linear_path_space_layout::has_intersect(e8util::ray const &r, float t_min, float t_max,
                                                 float &t) const {
    for (std::pair<obj_id_t const, binded_geometry> const &p : m_geometries) {
        if_geometry const *geo = p.second.geometry.get();

        std::vector<e8util::vec3> const &verts = geo->vertices();
        std::vector<triangle> const &tris = geo->triangles();

        for (triangle const &tri : tris) {
            e8util::vec3 const &v0 = verts[tri(0)];
            e8util::vec3 const &v1 = verts[tri(1)];
            e8util::vec3 const &v2 = verts[tri(2)];

            e8util::vec3 b;
            if (r.intersect(v0, v1, v2, t_min, t_max, b, t)) {
                return true;
            }
        }
    }
    return false;
}

e8::batched_geometry
e8::linear_path_space_layout::get_relevant_geometries(e8util::frustum const &) const {
    throw std::string("Not implemented yet.");
}

std::vector<e8::if_light const *>
e8::linear_path_space_layout::get_relevant_lights(e8util::frustum const &) const {
    throw std::string("Not implemented yet.");
}

#define BVH_MAX_PRIMS 4
#define BVH_BUCKET_COUNT 20
#define BVH_RAY_TRIANGLE_COST 8
#define BVH_RAY_BOX_COST 1

e8::bvh_path_space_layout::bvh_path_space_layout() {}

e8::bvh_path_space_layout::~bvh_path_space_layout() {}

std::vector<e8::bvh_path_space_layout::bucket>
e8::bvh_path_space_layout::sah_buckets(std::vector<primitive_details> const &prims, unsigned start,
                                       unsigned end, unsigned axis, e8util::aabb const &bound,
                                       e8util::vec3 const &range) {
    // construct bucket.
    std::vector<bucket> buckets(BVH_BUCKET_COUNT);

    float bucket_width = range(axis) / BVH_BUCKET_COUNT;
    for (unsigned i = start; i < end; i++) {
        unsigned i_bucket =
            static_cast<unsigned>((prims[i].centroid(axis) - bound.min()(axis)) / bucket_width);
        if (i_bucket == BVH_BUCKET_COUNT)
            i_bucket = BVH_BUCKET_COUNT - 1;
        buckets[i_bucket].bound = buckets[i_bucket].bound + prims[i].bound;
        buckets[i_bucket].num_prims++;
    }

    // compute cost.
    buckets[BVH_BUCKET_COUNT - 1].cost = INFINITY;
    for (unsigned i = 0; i < BVH_BUCKET_COUNT - 1; i++) {
        e8util::aabb first_part, second_part;
        unsigned c_first = 0, c_second = 0;
        for (unsigned j = 0; j <= i; j++) {
            first_part = first_part + buckets[j].bound;
            c_first += buckets[j].num_prims;
        }
        for (unsigned j = i + 1; j < BVH_BUCKET_COUNT; j++) {
            second_part = second_part + buckets[j].bound;
            c_second += buckets[j].num_prims;
        }
        buckets[i].cost =
            BVH_RAY_BOX_COST +
            BVH_RAY_TRIANGLE_COST * (+first_part.surf_area() / bound.surf_area() * c_first +
                                     second_part.surf_area() / bound.surf_area() * c_second);
    }
    return buckets;
}

e8util::aabb e8::bvh_path_space_layout::bound(std::vector<primitive_details> const &prims,
                                              unsigned start, unsigned end) {
    e8util::aabb bound;
    for (unsigned i = start; i < end; i++) {
        bound = bound + prims[i].bound;
    }
    return bound;
}

e8::bvh_path_space_layout::node *
e8::bvh_path_space_layout::bvh(std::vector<primitive_details> &prims, unsigned start, unsigned end,
                               unsigned depth) {
    if (end - start == 0) {
        // special (error) case: empty node.
        return nullptr;
    } else if (end - start == 1) {
        // base case.
        m_max_depth = std::max(m_max_depth, depth + 1);
        m_sum_depth += depth + 1;
        m_sum_depth2 += (depth + 1) * (depth + 1);
        m_num_paths++;
        m_num_nodes++;
        return new node(prims[start].bound, start, 1);
    } else {
        // decide which axis to split over.
        e8util::aabb const &b = bound(prims, start, end);
        e8util::vec3 const &range = b.max() - b.min();
        unsigned char split_axis;
        if (range(0) > range(1) && range(0) > range(2)) {
            split_axis = 0;
        } else if (range(1) > range(2)) {
            split_axis = 1;
        } else {
            split_axis = 2;
        }

        // compute the surface area heuristics.
        // split axis into buckets, then find the lowest cost split.
        float cost_nonsplit = (end - start) * BVH_RAY_TRIANGLE_COST;

        std::vector<bucket> const &buckets = sah_buckets(prims, start, end, split_axis, b, range);
        unsigned split_antiaxis = 1;
        float cost_split = buckets[0].cost;
        for (unsigned i = 1; i < buckets.size() - 1; i++) {
            if (buckets[i].cost < cost_split) {
                split_antiaxis = i;
                cost_split = buckets[i].cost;
            }
        }

        // decide whether to split based on cost and number of primitives.
        if (cost_split < cost_nonsplit || end - start > BVH_MAX_PRIMS) {
            unsigned mid;

            // split the primitives.
            if (depth > std::log2(prims.size())) {
                // objects may be too large, sah won't work well. Use median heuristics instead.
                std::sort(
                    prims.begin() + start, prims.begin() + end,
                    [split_axis](primitive_details const &a, primitive_details const &b) -> bool {
                        return a.centroid(split_axis) < b.centroid(split_axis);
                    });
                mid = (start + end) >> 1;
            } else {
                // use sah heuristics.
                float split = b.min()(split_axis) +
                              (split_antiaxis + 1) * range(split_axis) / BVH_BUCKET_COUNT;
                auto it = std::partition(prims.begin() + start, prims.begin() + end,
                                         [split_axis, split](primitive_details const &a) -> bool {
                                             return a.centroid(split_axis) < split;
                                         });
                mid = static_cast<unsigned>(it - prims.begin());

                // ensure each node has at least 1 element.
                if (mid == start)
                    mid++;
                else if (mid == end)
                    mid--;
                m_num_nodes++;
            }

            return new node(b, split_axis, bvh(prims, start, mid, depth + 1),
                            bvh(prims, mid, end, depth + 1));
        } else {
            m_max_depth = std::max(m_max_depth, depth + 1);
            m_sum_depth += depth + 1;
            m_sum_depth2 += (depth + 1) * (depth + 1);
            m_num_paths++;
            m_num_nodes++;
            return new node(b, start, static_cast<uint8_t>(end - start));
        }
    }
}

void e8::bvh_path_space_layout::delete_bvh(node *bvh, unsigned depth) {
    if (bvh != nullptr) {
        delete_bvh(bvh->children[0], depth + 1);
        delete bvh->children[0];
        delete_bvh(bvh->children[1], depth + 1);
        delete bvh->children[1];

        if (depth == 0)
            delete bvh;
    }
}

void e8::bvh_path_space_layout::flatten(std::vector<flattened_node> &bvh, node *bvh_node) {
    if (bvh_node == nullptr) {
        // special (error) case.
        return;
    } else if (bvh_node->num_prims > 0) {
        // exterior node.
        bvh.push_back(flattened_node(bvh_node->bound, bvh_node->prim_start, bvh_node->num_prims));
    } else {
        // interior node.
        unsigned p = static_cast<unsigned>(bvh.size());
        bvh.push_back(flattened_node());
        flatten(bvh, bvh_node->children[0]);
        bvh[p] = flattened_node(bvh_node->bound, bvh_node->split_axis,
                                static_cast<unsigned>(bvh.size()), 0x0);
        flatten(bvh, bvh_node->children[1]);
    }
}

void e8::bvh_path_space_layout::commit() {
    this->linear_path_space_layout::commit();

    m_mat_list.clear();
    m_light_list.clear();
    m_geo_list.clear();
    m_prims.clear();
    m_bvh.clear();

    // construct geometry, light and material mapping.
    std::map<if_geometry const *, unsigned int> geo2ind;
    std::map<if_material const *, unsigned short> mat2ind;
    std::map<if_light const *, unsigned short> light2ind;
    for (std::pair<obj_id_t const, binded_geometry> const &geo : m_geometries) {
        geo2ind.insert(std::make_pair(geo.second.geometry.get(), m_geo_list.size()));
        m_geo_list.push_back(geo.second.geometry.get());

        if (geo.second.mat != nullptr) {
            auto mat_it = mat2ind.find(geo.second.mat.get());
            if (mat_it == mat2ind.end()) {
                mat2ind.insert(std::make_pair(geo.second.mat.get(), m_mat_list.size()));
                m_mat_list.push_back(geo.second.mat.get());
            }
        }

        if (geo.second.light != nullptr) {
            auto light_it = light2ind.find(geo.second.light.get());
            if (light_it == light2ind.end()) {
                light2ind.insert(std::make_pair(geo.second.light.get(), m_light_list.size()));
                m_light_list.push_back(geo.second.light.get());
            }
        }
    }
    mat2ind.insert(std::make_pair(nullptr, 0xFFFF));
    light2ind.insert(std::make_pair(nullptr, 0xFFFF));

    // construct primitive list.
    std::vector<primitive_details> prims;
    for (std::pair<obj_id_t const, binded_geometry> const &p : m_geometries) {
        for (triangle const &tri : p.second.geometry->triangles()) {
            prims.push_back(
                primitive_details(tri, p.second.geometry.get(), geo2ind[p.second.geometry.get()],
                                  mat2ind[p.second.mat.get()], light2ind[p.second.light.get()]));
        }
    }

    m_sum_depth = 0;
    m_sum_depth2 = 0;
    m_max_depth = 0;
    m_num_nodes = 0;

    node *tmp_bvh = bvh(prims, 0, static_cast<unsigned>(prims.size()), 0);
    flatten(m_bvh, tmp_bvh);
    delete_bvh(tmp_bvh, 0);

    // discards the details.
    m_prims.reserve(prims.size());
    for (unsigned i = 0; i < prims.size(); i++) {
        m_prims.push_back(prims[i]);
    }
}

e8::intersect_info e8::bvh_path_space_layout::intersect(e8util::ray const &r) const {
    if (m_bvh.empty()) {
        return intersect_info();
    }

    float const t_min = 1e-4f;
    float const t_max = 1000.0f;

    float t = INFINITY;

    primitive const *hit_prim = nullptr;
    e8util::vec3 hit_b;

    std::vector<unsigned> candids({0});
    while (!candids.empty()) {
        unsigned n = candids.back();
        candids.pop_back();

        if (m_bvh[n].num_prims > 0) {
            // exterior node.
            unsigned ps = m_bvh[n].prim_start;
            unsigned pe = ps + m_bvh[n].num_prims;

            for (unsigned i = ps; i < pe; i++) {
                primitive const &prim = m_prims[i];
                std::vector<e8util::vec3> const &verts = m_geo_list[prim.i_geo]->vertices();

                e8util::vec3 const &v0 = verts[prim.tri(0)];
                e8util::vec3 const &v1 = verts[prim.tri(1)];
                e8util::vec3 const &v2 = verts[prim.tri(2)];

                e8util::vec3 b;
                float t0;
                if (r.intersect(v0, v1, v2, t_min, t_max, b, t0) && t0 < t) {
                    t = t0;
                    hit_prim = &prim;
                    hit_b = b;
                }
            }
        } else {
            // interior node.
            unsigned left = n + 1;
            unsigned right = m_bvh[n].next_child;

            float t0, t1;
            if (m_bvh[left].bound.intersect(r, t_min, t_max, t0, t1)) {
                candids.push_back(left);
            }

            if (m_bvh[right].bound.intersect(r, t_min, t_max, t0, t1)) {
                candids.push_back(right);
            }
        }
    }

    if (hit_prim) {
        if_geometry const *hit_geo = m_geo_list[hit_prim->i_geo];
        std::vector<e8util::vec3> const &verts = hit_geo->vertices();
        e8util::vec3 const &v0 = verts[hit_prim->tri(0)];
        e8util::vec3 const &v1 = verts[hit_prim->tri(1)];
        e8util::vec3 const &v2 = verts[hit_prim->tri(2)];

        e8util::vec3 const &vertex = hit_b(0) * v0 + hit_b(1) * v1 + hit_b(2) * v2;

        std::vector<e8util::vec3> const &normals = hit_geo->normals();
        e8util::vec3 const &n0 = normals[hit_prim->tri(0)];
        e8util::vec3 const &n1 = normals[hit_prim->tri(1)];
        e8util::vec3 const &n2 = normals[hit_prim->tri(2)];
        e8util::vec3 const &normal = (hit_b(0) * n0 + hit_b(1) * n1 + hit_b(2) * n2).normalize();

        std::vector<e8util::vec2> const &texcoords = hit_geo->texcoords();
        e8util::vec2 uv;
        if (!texcoords.empty()) {
            e8util::vec2 uv0 = texcoords[hit_prim->tri(0)];
            e8util::vec2 uv1 = texcoords[hit_prim->tri(1)];
            e8util::vec2 uv2 = texcoords[hit_prim->tri(2)];
            uv = (hit_b(0) * uv0 + hit_b(1) * uv1 + hit_b(2) * uv2).normalize();
        }

        return intersect_info(t, vertex, normal, uv,
                              hit_prim->i_mat == 0xFFFF ? nullptr : m_mat_list[hit_prim->i_mat],
                              hit_prim->i_light == 0xFFFF ? nullptr
                                                          : m_light_list[hit_prim->i_light]);
    } else {
        return intersect_info();
    }
}

bool e8::bvh_path_space_layout::has_intersect(e8util::ray const &r, float t_min, float t_max,
                                              float &t) const {
    std::vector<unsigned> candids({0});
    while (!candids.empty()) {
        unsigned n = candids.back();
        candids.pop_back();

        if (m_bvh[n].num_prims > 0) {
            // exterior node.
            unsigned ps = m_bvh[n].prim_start;
            unsigned pe = ps + m_bvh[n].num_prims;

            for (unsigned i = ps; i < pe; i++) {
                primitive const &prim = m_prims[i];
                std::vector<e8util::vec3> const &verts = m_geo_list[prim.i_geo]->vertices();

                e8util::vec3 const &v0 = verts[prim.tri(0)];
                e8util::vec3 const &v1 = verts[prim.tri(1)];
                e8util::vec3 const &v2 = verts[prim.tri(2)];

                e8util::vec3 b;
                if (r.intersect(v0, v1, v2, t_min, t_max, b, t)) {
                    return true;
                }
            }
        } else {
            // interior node.
            unsigned left = n + 1;
            unsigned right = m_bvh[n].next_child;

            float t0, t1;
            if (m_bvh[left].bound.intersect(r, t_min, t_max, t0, t1)) {
                candids.push_back(left);
            }

            if (m_bvh[right].bound.intersect(r, t_min, t_max, t0, t1)) {
                candids.push_back(right);
            }
        }
    }
    return false;
}

unsigned e8::bvh_path_space_layout::max_depth() const { return m_max_depth; }

float e8::bvh_path_space_layout::avg_depth() const {
    return static_cast<float>(m_sum_depth) / m_num_paths;
}

float e8::bvh_path_space_layout::dev_depth() const {
    float mu = avg_depth();
    return std::sqrt(static_cast<float>(m_sum_depth2) / m_num_paths - mu * mu);
}

unsigned e8::bvh_path_space_layout::num_nodes() const { return m_num_nodes; }
