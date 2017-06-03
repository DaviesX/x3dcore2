#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "tensor.h"
#include "material.h"

namespace e8
{

typedef e8util::vec<3, unsigned> triangle;

class if_geometry
{
public:
        if_geometry();
        virtual ~if_geometry();

        void                                            bind(if_material const& mat);
        if_material const&                              material() const;

        virtual std::vector<e8util::vec3> const&        vertices() const = 0;
        virtual std::vector<e8util::vec3> const&        normals() const = 0;
        virtual std::vector<triangle> const&            triangles() const = 0;
        virtual e8util::aabb                            aabb() const = 0;

protected:
        if_material const*    m_mat;
};


class trimesh: public if_geometry
{
public:
        trimesh();
        virtual ~trimesh();

        virtual std::vector<e8util::vec3> const&        vertices() const override;
        virtual std::vector<e8util::vec3> const&        normals() const override;
        virtual std::vector<triangle> const&            triangles() const override;
        virtual e8util::aabb                            aabb() const override;

        void                                            compute_aabb();
protected:
        std::vector<e8util::vec3>       m_verts;
        std::vector<e8util::vec3>       m_norms;
        std::vector<triangle>           m_tris;
        e8util::aabb                    m_aabb;
};


class uv_sphere: public trimesh
{
public:
        uv_sphere(e8util::vec3 const& o, float r, unsigned const res);
        ~uv_sphere();
};



}

#endif // GEOMETRY_H
