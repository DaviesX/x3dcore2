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
        ~if_pathtracer();

        virtual std::vector<e8util::vec3>       sample(e8util::rng& rng, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned n) const = 0;
};


class direct_pathtracer: public if_pathtracer
{
public:
        direct_pathtracer();
        ~direct_pathtracer();

        std::vector<e8util::vec3>       sample(e8util::rng& rng, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned n) const override;
};

}


#endif // IF_PATHTRACER_H
