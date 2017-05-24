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


class sphere: public if_geometry
{
public:
        sphere(e8util::vec3 const& o, float r, unsigned res);
        ~sphere();

        std::vector<e8util::vec3> const&        vertices() const override;
        std::vector<e8util::vec3> const&        normals() const override;
        std::vector<triangle> const&            triangles() const override;
        e8util::aabb                            aabb() const override;
private:
        e8util::vec3    m_o;
        float           m_r;
        unsigned        m_res;
};

}

#endif // GEOMETRY_H
