#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "tensor.h"
#include "frame.h"

namespace e8
{

typedef e8util::vec<4, float>           rgba_color;

class if_compositor
{
public:
        if_compositor();
        virtual ~if_compositor();

        virtual rgba_color      operator()(unsigned i, unsigned j) const = 0;
        virtual rgba_color&     operator()(unsigned i, unsigned j) = 0;

        virtual unsigned        width() const = 0;
        virtual unsigned        height() const = 0;

        virtual void            width(unsigned w) = 0;
        virtual void            height(unsigned h) = 0;

        virtual void            commit(if_frame* frame) const = 0;
};

class aces_compositor: public if_compositor
{
};

}

#endif // COMPOSITOR_H
