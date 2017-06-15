#ifndef IF_PATHTRACER_H
#define IF_PATHTRACER_H

#include <vector>
#include "tensor.h"
#include "scene.h"

namespace e8 {

class if_pathtracer
{
public:
        if_pathtracer();
        virtual ~if_pathtracer();

        virtual std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                                       std::vector<e8util::ray> const& rays,
                                                       if_scene const* scene,
                                                       unsigned n) const = 0;
};


class position_pathtracer: public if_pathtracer
{
public:
        position_pathtracer();
        ~position_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                               std::vector<e8util::ray> const& rays,
                                               if_scene const* scene,
                                               unsigned n) const override;
};

class normal_pathtracer: public if_pathtracer
{
public:
        normal_pathtracer();
        ~normal_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                               std::vector<e8util::ray> const& rays,
                                               if_scene const* scene,
                                               unsigned n) const override;
};

class direct_pathtracer: public if_pathtracer
{
public:
        direct_pathtracer();
        ~direct_pathtracer() override;

        virtual std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                                       std::vector<e8util::ray> const& rays,
                                                       if_scene const* scene,
                                                       unsigned n) const override;
protected:
        e8util::vec3                    sample_direct_illum(e8util::rng& rng,
                                                            e8util::vec3 const& o,
                                                            e8::intersect_info const& inf,
                                                            if_scene const* scene,
                                                            unsigned n) const;
};


class unidirect_pathtracer: public direct_pathtracer
{
public:
        unidirect_pathtracer();
        ~unidirect_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                               std::vector<e8util::ray> const& rays,
                                               if_scene const* scene,
                                               unsigned n) const override;
private:
        e8util::vec3                    sample_indirect_illum(e8util::rng& rng,
                                                              e8util::vec3 const& o,
                                                              e8::intersect_info const& info,
                                                              if_scene const* scene,
                                                              unsigned depth,
                                                              unsigned n,
                                                              unsigned m) const;
};

class bidirect_pathtracer: public if_pathtracer
{
public:
        bidirect_pathtracer();
        ~bidirect_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                               std::vector<e8util::ray> const& rays,
                                               if_scene const* scene,
                                               unsigned n) const override;
private:
        e8util::vec3                    sample_light_illum(e8util::rng& rng,
                                                           e8util::vec3 const& rad,
                                                           e8util::vec3 const& w,
                                                           e8::intersect_info const& info,
                                                           if_scene const* scene,
                                                           unsigned depth,
                                                           e8::intersect_info& terminate,
                                                           e8util::vec3& t) const;
        e8util::vec3                    sample_illum(e8util::rng& rng, e8util::vec3 const& o, e8::intersect_info const& info,
                                                     if_scene const* scene) const;
        e8util::vec3                    sample_indirect_illum(e8util::rng& rng,
                                                              e8util::vec3 const& o,
                                                              e8::intersect_info const& info,
                                                              if_scene const* scene,
                                                              unsigned depth) const;
};

}


#endif // IF_PATHTRACER_H
