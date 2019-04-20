#include <iostream>
#include <cassert>
#include <algorithm>
#include "scene.h"


e8::if_path_space::if_path_space()
{
}

e8::if_path_space::~if_path_space()
{
        std::set<if_material const*> mats;
        for (std::pair<obj_id_t, binded_geometry> p: m_geometries) {
                mats.insert(p.second.mat);
                delete p.second.geometry;
        }
        for (if_material const* mat: mats) {
                delete mat;
        }
        for (if_light const* light: m_lights) {
                delete light;
        }
}

void
e8::if_path_space::add_geometry(if_geometry const* geometry)
{
        if (m_geometries.find(geometry->id()) == m_geometries.end())
                m_bound = m_bound + geometry->aabb();
        m_geometries.insert(std::make_pair(geometry->id(),
                                           if_path_space::binded_geometry(geometry, nullptr, nullptr)));
}

void
e8::if_path_space::add_light(if_light const* light)
{
        m_lights.insert(light);
}

void
e8::if_path_space::bind(if_geometry const* geometry, if_material const* mat)
{
        m_geometries.at(geometry->id()).mat = mat;
}

void
e8::if_path_space::bind(if_geometry const* geometry, if_light const* light)
{
        m_geometries.at(geometry->id()).light = light;
}

e8util::aabb
e8::if_path_space::aabb() const
{
        return m_bound;
}

void
e8::if_path_space::load(e8util::if_resource* res)
{
        std::vector<if_geometry*> const& geos = res->load_geometries();
        std::vector<if_material*> const& mats = res->load_materials();
        std::vector<if_light*> const& lights = res->load_lights();
        std::vector<if_light*> const& v_lights = res->load_virtual_lights();

        for (if_geometry* geo: geos)
                add_geometry(geo);
        for (unsigned i = 0; i < mats.size(); i ++) {
                if (mats[i]) {
                        bind(geos[i], mats[i]);
                }
        }
        for (unsigned i = 0; i < lights.size(); i ++) {
                if (lights[i]) {
                        lights[i]->set_scene_boundary(m_bound);
                        add_light(lights[i]);
                        bind(geos[i], lights[i]);
                }
        }
        for (unsigned i = 0; i < v_lights.size(); i ++) {
                if (v_lights[i]) {
                        v_lights[i]->set_scene_boundary(m_bound);
                        add_light(v_lights[i]);
                }
        }
}

void
e8::if_path_space::load(if_obj* obj, e8util::mat44 const& trans)
{
        if_geometry* geo = static_cast<if_geometry*>(obj)->transform(trans);
        std::vector<if_obj*> mats = obj->get_children(typeid(if_material));
        std::vector<if_obj*> lights = obj->get_children(typeid(if_light));

        add_geometry(geo);
        // TODO: can be static_cast when if_material and if_light are if_objs.
        bind(geo, reinterpret_cast<if_material*>(mats[0]));
        bind(geo, reinterpret_cast<if_light*>(lights[0]));

        m_lights.insert(reinterpret_cast<if_light*>(lights[0]));
}

void
e8::if_path_space::unload(if_obj* obj)
{
        auto it = m_geometries.find(obj->id());
        if (it != m_geometries.end()) {
                m_geometries.erase(it);
        }
}

const std::type_info&
e8::if_path_space::support() const
{
        return typeid(if_geometry);
}


e8::linear_path_space_layout::linear_path_space_layout()
{
}

e8::linear_path_space_layout::~linear_path_space_layout()
{
}

void
e8::linear_path_space_layout::commit()
{
        m_cum_power.resize(m_lights.size());
        m_light_list.resize(m_lights.size());

        unsigned i = 0;
        m_total_power = 0;
        for (if_light const* light: m_lights) {
                m_total_power += light->power().norm();
                m_cum_power[i] = m_total_power;
                m_light_list[i] = light;
                i ++;
        }
}

e8::intersect_info
e8::linear_path_space_layout::intersect(e8util::ray const& r) const
{
        float const t_min = 1e-4f;
        float const t_max = 1000.0f;

        float t = INFINITY;
        binded_geometry const*  hit_geo = nullptr;
        triangle const*         hit_tri = nullptr;
        e8util::vec3            hit_b;

        for (auto it = m_geometries.begin(); it != m_geometries.end(); ++it) {
                if_geometry const* geo = it->second.geometry;

                std::vector<e8util::vec3> const&        verts = geo->vertices();
                std::vector<triangle> const&            tris = geo->triangles();

                for (triangle const& tri: tris) {
                        e8util::vec3 const& v0 = verts[tri(0)];
                        e8util::vec3 const& v1 = verts[tri(1)];
                        e8util::vec3 const& v2 = verts[tri(2)];

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
                std::vector<e8util::vec3> const& verts = hit_geo->geometry->vertices();
                e8util::vec3 const& v0 = verts[(*hit_tri)(0)];
                e8util::vec3 const& v1 = verts[(*hit_tri)(1)];
                e8util::vec3 const& v2 = verts[(*hit_tri)(2)];
                e8util::vec3 const& vertex = hit_b(0)*v0 + hit_b(1)*v1 + hit_b(2)*v2;

                std::vector<e8util::vec3> const& normals = hit_geo->geometry->normals();
                e8util::vec3 const& n0 = normals[(*hit_tri)(0)];
                e8util::vec3 const& n1 = normals[(*hit_tri)(1)];
                e8util::vec3 const& n2 = normals[(*hit_tri)(2)];
                e8util::vec3 const& normal = (hit_b(0)*n0 + hit_b(1)*n1 + hit_b(2)*n2).normalize();
                return intersect_info(t, vertex, normal, hit_geo->mat, hit_geo->light);
        } else {
                return intersect_info();
        }
}

bool
e8::linear_path_space_layout::has_intersect(e8util::ray const& r, float t_min, float t_max, float& t) const
{
        for (std::pair<obj_id_t, binded_geometry> p: m_geometries) {
                if_geometry const* geo = p.second.geometry;

                std::vector<e8util::vec3> const&        verts = geo->vertices();
                std::vector<triangle> const&            tris = geo->triangles();

                for (triangle const& tri: tris) {
                        e8util::vec3 const& v0 = verts[tri(0)];
                        e8util::vec3 const& v1 = verts[tri(1)];
                        e8util::vec3 const& v2 = verts[tri(2)];

                        e8util::vec3 b;
                        if (r.intersect(v0, v1, v2, t_min, t_max, b, t)) {
                                return true;
                        }
                }
        }
        return false;
}

e8::batched_geometry
e8::linear_path_space_layout::get_relevant_geometries(e8util::frustum const&) const
{
        throw std::string("Not implemented yet.");
}

std::vector<e8::if_light const*>
e8::linear_path_space_layout::get_relevant_lights(e8util::frustum const&) const
{
        throw std::string("Not implemented yet.");
}

e8::if_light const*
e8::linear_path_space_layout::sample_light(e8util::rng& rng, float& pdf) const
{
        assert(!m_light_list.empty());
        float e = rng.draw()*m_total_power;
        unsigned lo = 0;
        unsigned hi = static_cast<unsigned>(m_cum_power.size());
        while (lo < hi) {
                unsigned mi = (lo + hi) >> 1;
                if (m_cum_power[mi] < e)
                        lo = mi + 1;
                else
                        hi = mi;
        }
        pdf = m_light_list[lo]->power().norm()/m_total_power;
        return m_light_list[lo];
}


#define BVH_MAX_PRIMS           4
#define BVH_BUCKET_COUNT        20
#define BVH_RAY_TRIANGLE_COST   8
#define BVH_RAY_BOX_COST        1

e8::bvh_path_space_layout::bvh_path_space_layout()
{
}

e8::bvh_path_space_layout::~bvh_path_space_layout()
{
}

std::vector<e8::bvh_path_space_layout::bucket>
e8::bvh_path_space_layout::sah_buckets(std::vector<primitive_details> const& prims, unsigned start, unsigned end,
                                  unsigned axis, e8util::aabb const& bound, e8util::vec3 const& range)
{
        // construct bucket.
        std::vector<bucket> buckets(BVH_BUCKET_COUNT);

        float bucket_width = range(axis)/BVH_BUCKET_COUNT;
        for (unsigned i = start; i < end; i ++) {
                unsigned i_bucket = static_cast<unsigned>((prims[i].centroid(axis) - bound.min()(axis))/bucket_width);
                if (i_bucket == BVH_BUCKET_COUNT)
                        i_bucket = BVH_BUCKET_COUNT - 1;
                buckets[i_bucket].bound = buckets[i_bucket].bound + prims[i].bound;
                buckets[i_bucket].num_prims ++;
        }

        // compute cost.
        buckets[BVH_BUCKET_COUNT - 1].cost = INFINITY;
        for (unsigned i = 0; i < BVH_BUCKET_COUNT - 1; i ++) {
                e8util::aabb first_part, second_part;
                unsigned c_first = 0, c_second = 0;
                for (unsigned j = 0; j <= i; j ++) {
                        first_part = first_part + buckets[j].bound;
                        c_first += buckets[j].num_prims;
                }
                for (unsigned j = i + 1; j < BVH_BUCKET_COUNT; j ++) {
                        second_part = second_part + buckets[j].bound;
                        c_second += buckets[j].num_prims;
                }
                buckets[i].cost = BVH_RAY_BOX_COST + BVH_RAY_TRIANGLE_COST*(
                                + first_part.surf_area()/bound.surf_area() * c_first
                                + second_part.surf_area()/bound.surf_area() * c_second);
        }
        return buckets;
}

e8util::aabb
e8::bvh_path_space_layout::bound(std::vector<primitive_details> const& prims, unsigned start, unsigned end)
{
        e8util::aabb bound;
        for (unsigned i = start; i < end; i ++) {
                bound = bound + prims[i].bound;
        }
        return bound;
}

e8::bvh_path_space_layout::node*
e8::bvh_path_space_layout::bvh(std::vector<primitive_details>& prims, unsigned start, unsigned end, unsigned depth)
{
        if (end - start == 0) {
                // special (error) case: empty node.
                return nullptr;
        } else if (end - start == 1) {
                // base case.
                m_max_depth = std::max(m_max_depth, depth + 1);
                m_sum_depth += depth + 1;
                m_sum_depth2 += (depth + 1)*(depth + 1);
                m_num_paths ++;
                m_num_nodes ++;
                return new node(prims[start].bound, start, 1);
        } else {
                // decide which axis to split over.
                e8util::aabb const& b = bound(prims, start, end);
                e8util::vec3 const& range = b.max() - b.min();
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
                float cost_nonsplit = (end - start)*BVH_RAY_TRIANGLE_COST;

                std::vector<bucket> const& buckets = sah_buckets(prims, start, end, split_axis, b, range);
                unsigned split_antiaxis = 1;
                float cost_split = buckets[0].cost;
                for (unsigned i = 1; i < buckets.size() - 1; i ++) {
                        if (buckets[i].cost < cost_split) {
                                split_antiaxis = i;
                                cost_split = buckets[i].cost;
                        }
                }

                // decide whether to split based on cost and number of primitives.
                if (cost_split < cost_nonsplit ||
                    end - start > BVH_MAX_PRIMS) {
                        unsigned mid;

                        // split the primitives.
                        if (depth > std::log2(prims.size())) {
                                // objects may be too large, sah won't work well. Use median heuristics instead.
                                std::sort(prims.begin() + start, prims.begin() + end,
                                          [split_axis](primitive_details const& a,
                                                       primitive_details const& b) -> bool {
                                        return a.centroid(split_axis) < b.centroid(split_axis);

                                });
                                mid = (start + end) >> 1;
                        } else {
                                // use sah heuristics.
                                float split = b.min()(split_axis) + (split_antiaxis + 1)*range(split_axis)/BVH_BUCKET_COUNT;
                                auto it = std::partition(prims.begin() + start,
                                                         prims.begin() + end,
                                                         [split_axis, split](primitive_details const& a) -> bool {
                                        return a.centroid(split_axis) < split;
                                });
                                mid = static_cast<unsigned>(it - prims.begin());

                                // ensure each node has at least 1 element.
                                if (mid == start)
                                        mid ++;
                                else if (mid == end)
                                        mid --;
                                m_num_nodes ++;
                        }

                        return new node(b, split_axis,
                                        bvh(prims, start, mid, depth + 1),
                                        bvh(prims, mid, end, depth + 1));
                } else {
                        m_max_depth = std::max(m_max_depth, depth + 1);
                        m_sum_depth += depth + 1;
                        m_sum_depth2 += (depth + 1)*(depth + 1);
                        m_num_paths ++;
                        m_num_nodes ++;
                        return new node(b, start, static_cast<uint8_t>(end - start));
                }
        }
}

void
e8::bvh_path_space_layout::delete_bvh(node* bvh, unsigned depth)
{
        if (bvh != nullptr) {
                delete_bvh(bvh->children[0], depth + 1);
                delete bvh->children[0];
                delete_bvh(bvh->children[1], depth + 1);
                delete bvh->children[1];

                if (depth == 0)
                        delete bvh;
        }
}

void
e8::bvh_path_space_layout::flatten(std::vector<flattened_node>& bvh, node* bvh_node)
{
        if (bvh_node == nullptr) {
                // special (error) case.
                return ;
        } else if (bvh_node->num_prims > 0) {
                // exterior node.
                bvh.push_back(flattened_node(bvh_node->bound, bvh_node->prim_start, bvh_node->num_prims));
        } else {
                // interior node.
                unsigned p = static_cast<unsigned>(bvh.size());
                bvh.push_back(flattened_node());
                flatten(bvh, bvh_node->children[0]);
                bvh[p] = flattened_node(bvh_node->bound,
                                        bvh_node->split_axis,
                                        static_cast<unsigned>(bvh.size()),
                                        0x0);
                flatten(bvh, bvh_node->children[1]);
        }
}

void
e8::bvh_path_space_layout::commit()
{
        this->linear_path_space_layout::commit();

        m_mat_list.clear();
        m_light_list.clear();
        m_geo_list.clear();
        m_prims.clear();
        m_bvh.clear();

        // construct geometry, light and material mapping.
        std::map<if_geometry const*, unsigned int> geo_map;
        std::map<if_material const*, unsigned short> mat_map;
        std::map<if_light const*, unsigned short> light_map;
        for (std::pair<obj_id_t, binded_geometry> geo: m_geometries) {
                geo_map.insert(std::make_pair(geo.second.geometry, m_geo_list.size()));
                m_geo_list.push_back(geo.second.geometry);

                auto it = mat_map.find(geo.second.mat);
                if (it == mat_map.end()) {
                        mat_map.insert(std::make_pair(geo.second.mat, m_mat_list.size()));
                        m_mat_list.push_back(geo.second.mat);
                }
        }
        mat_map.insert(std::make_pair(nullptr, 0xFFFF));

        for (if_light const* light: m_lights) {
                light_map.insert(std::make_pair(light, m_light_list.size()));
                m_light_list.push_back(light);
        }
        light_map.insert(std::make_pair(nullptr, 0xFFFF));

        // construct primitive list.
        std::vector<primitive_details> prims;
        for (std::pair<obj_id_t, binded_geometry> p: m_geometries) {
                for (triangle const& tri: p.second.geometry->triangles()) {
                        prims.push_back(primitive_details(tri, p.second.geometry,
                                                          geo_map[p.second.geometry],
                                        mat_map[p.second.mat],
                                        light_map[p.second.light]));
                }
        }

        m_sum_depth = 0;
        m_sum_depth2 = 0;
        m_max_depth = 0;
        m_num_nodes = 0;

        node* tmp_bvh = bvh(prims, 0, static_cast<unsigned>(prims.size()), 0);
        flatten(m_bvh, tmp_bvh);
        delete_bvh(tmp_bvh, 0);

        // discards the details.
        m_prims.reserve(prims.size());
        for (unsigned i = 0; i < prims.size(); i ++) {
                m_prims.push_back(prims[i]);
        }
}

e8::intersect_info
e8::bvh_path_space_layout::intersect(e8util::ray const& r) const
{
        if (m_bvh.empty()) {
                return intersect_info();
        }

        float const t_min = 1e-4f;
        float const t_max = 1000.0f;

        float t = INFINITY;

        primitive const*        hit_prim = nullptr;
        e8util::vec3            hit_b;

        std::vector<unsigned> candids({0});
        while (!candids.empty()) {
                unsigned n = candids.back();
                candids.pop_back();

                if (m_bvh[n].num_prims > 0) {
                        // exterior node.
                        unsigned ps = m_bvh[n].prim_start;
                        unsigned pe = ps + m_bvh[n].num_prims;

                        for (unsigned i = ps; i < pe; i ++) {
                                primitive const& prim = m_prims[i];
                                std::vector<e8util::vec3> const& verts = m_geo_list[prim.i_geo]->vertices();

                                e8util::vec3 const& v0 = verts[prim.tri(0)];
                                e8util::vec3 const& v1 = verts[prim.tri(1)];
                                e8util::vec3 const& v2 = verts[prim.tri(2)];

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
                if_geometry const* hit_geo = m_geo_list[hit_prim->i_geo];
                std::vector<e8util::vec3> const& verts = hit_geo->vertices();
                e8util::vec3 const& v0 = verts[hit_prim->tri(0)];
                e8util::vec3 const& v1 = verts[hit_prim->tri(1)];
                e8util::vec3 const& v2 = verts[hit_prim->tri(2)];

                e8util::vec3 const& vertex = hit_b(0)*v0 + hit_b(1)*v1 + hit_b(2)*v2;

                std::vector<e8util::vec3> const& normals = hit_geo->normals();
                e8util::vec3 const& n0 = normals[hit_prim->tri(0)];
                e8util::vec3 const& n1 = normals[hit_prim->tri(1)];
                e8util::vec3 const& n2 = normals[hit_prim->tri(2)];
                e8util::vec3 const& normal = (hit_b(0)*n0 + hit_b(1)*n1 + hit_b(2)*n2).normalize();

                return intersect_info(t, vertex, normal,
                                      hit_prim->i_mat == 0xFFFF ? nullptr : m_mat_list[hit_prim->i_mat],
                                      hit_prim->i_light == 0xFFFF ? nullptr : m_light_list[hit_prim->i_light]);
        } else {
                return intersect_info();
        }
}

bool
e8::bvh_path_space_layout::has_intersect(e8util::ray const& r, float t_min, float t_max, float& t) const
{
        std::vector<unsigned> candids({0});
        while (!candids.empty()) {
                unsigned n = candids.back();
                candids.pop_back();

                if (m_bvh[n].num_prims > 0) {
                        // exterior node.
                        unsigned ps = m_bvh[n].prim_start;
                        unsigned pe = ps + m_bvh[n].num_prims;

                        for (unsigned i = ps; i < pe; i ++) {
                                primitive const& prim = m_prims[i];
                                std::vector<e8util::vec3> const& verts = m_geo_list[prim.i_geo]->vertices();

                                e8util::vec3 const& v0 = verts[prim.tri(0)];
                                e8util::vec3 const& v1 = verts[prim.tri(1)];
                                e8util::vec3 const& v2 = verts[prim.tri(2)];

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

unsigned
e8::bvh_path_space_layout::max_depth() const
{
        return m_max_depth;
}

float
e8::bvh_path_space_layout::avg_depth() const
{
        return static_cast<float>(m_sum_depth)/m_num_paths;
}

float
e8::bvh_path_space_layout::dev_depth() const
{
        float mu = avg_depth();
        return std::sqrt(static_cast<float>(m_sum_depth2)/m_num_paths - mu*mu);
}

unsigned
e8::bvh_path_space_layout::num_nodes() const
{
        return m_num_nodes;
}
