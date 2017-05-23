#ifndef LIGHT_H
#define LIGHT_H

#include "tensor.h"

namespace e8
{

class if_light
{
public:
        if_light();
        ~if_light();

        virtual e8util::vec3    sample() const = 0;
};

}

#endif // LIGHT_H
