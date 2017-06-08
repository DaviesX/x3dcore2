#include "compositor.h"


e8::if_compositor::if_compositor(unsigned width, unsigned height):
        m_w(width), m_h(height), m_fbuffer(new rgba_color [width*height])
{
}

e8::if_compositor::~if_compositor()
{
        delete [] m_fbuffer;
}

e8::rgba_color
e8::if_compositor::operator()(unsigned i, unsigned j) const
{
        return m_fbuffer[i + j*m_w];
}

e8::rgba_color&
e8::if_compositor::operator()(unsigned i, unsigned j)
{
        return m_fbuffer[i + j*m_w];
}

unsigned
e8::if_compositor::width() const
{
        return m_w;
}

unsigned
e8::if_compositor::height() const
{
        return m_h;
}

void
e8::if_compositor::resize(unsigned w, unsigned h)
{
        m_w = w;
        m_h = h;
        delete [] m_fbuffer;
        m_fbuffer = new rgba_color [w*h];
}
