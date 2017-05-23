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

        virtual std::vector<e8util::vec3>       sample(std::vector<e8util::ray> const& rays, if_scene const& scene, unsigned n) const = 0;
};

}


#endif // IF_PATHTRACER_H
