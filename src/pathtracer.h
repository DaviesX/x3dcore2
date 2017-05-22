#ifndef IF_PATHTRACER_H
#define IF_PATHTRACER_H

#include <vector>
#include "tensor.h"

namespace e8 {

class if_pathtracer
{
public:
        if_pathtracer();
        std::vector<e8util::vec3>       sample(std::vector<e8util::ray> const& rays) const;
};

}


#endif // IF_PATHTRACER_H
