#ifndef CAMERA_H
#define CAMERA_H


#include <vector>
#include "tensor.h"


namespace e8
{

class if_camera
{
public:
        if_camera();
        ~if_camera();

        virtual std::vector<e8util::ray>        sample(unsigned x, unsigned y, unsigned w, unsigned h, unsigned n) const = 0;
        virtual e8util::mat44                   projection() const = 0;
};

}

#endif // CAMERA_H
