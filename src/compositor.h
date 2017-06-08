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
        if_compositor(unsigned width, unsigned height);
        virtual ~if_compositor();

        rgba_color      operator()(unsigned i, unsigned j) const;
        rgba_color&     operator()(unsigned i, unsigned j);

        unsigned        width() const;
        unsigned        height() const;

        void            resize(unsigned w, unsigned h);

        virtual void    commit(if_frame* frame) const = 0;
protected:
        unsigned        m_w;
        unsigned        m_h;
        rgba_color*     m_fbuffer;
};

class aces_compositor: public if_compositor
{
public:
        aces_compositor();
        ~aces_compositor();

        void            commit(if_frame* frame) const override;
        void            enable_auto_exposure(bool s);
        void            set_exposure(float e);
private:
};

}

#endif // COMPOSITOR_H
