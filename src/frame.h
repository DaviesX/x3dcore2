#ifndef IF_FRAME_H
#define IF_FRAME_H

#include "tensor.h"

namespace e8
{

class if_frame
{
public:
        if_frame();
        ~if_frame();

        virtual e8util::vec4    operator()(unsigned i, unsigned j) const = 0;
        virtual e8util::vec4&   operator()(unsigned i, unsigned j) = 0;

        virtual unsigned        width() const = 0;
        virtual unsigned        height() const = 0;

        virtual void            commit() = 0;
};

}

#endif // IF_FRAME_H
