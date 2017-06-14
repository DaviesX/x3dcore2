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

        virtual std::vector<e8util::vec3>       sample(e8util::rng& rng, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned n) const = 0;
};


class position_pathtracer: public if_pathtracer
{
public:
        position_pathtracer();
        ~position_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned n) const override;
};

class normal_pathtracer: public if_pathtracer
{
public:
        normal_pathtracer();
        ~normal_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned n) const override;
};

class direct_pathtracer: public if_pathtracer
{
public:
        direct_pathtracer();
        ~direct_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned n) const override;
};

}


#endif // IF_PATHTRACER_H
