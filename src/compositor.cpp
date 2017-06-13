#include <algorithm>
#include <cmath>
#include "tensor.h"
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



e8::aces_compositor::aces_compositor(unsigned width, unsigned height):
        if_compositor(width, height)
{
}

e8::aces_compositor::~aces_compositor()
{
}

void
e8::aces_compositor::commit(if_frame* frame) const
{
        float e = m_is_auto_exposure ? exposure() : m_e;
        for (unsigned j = 0; j < m_h; j ++) {
                for (unsigned i = 0; i < m_w; i ++) {
                        (*frame)(i, j) = pixel_of(aces_tonemap((*this)(i,j), e));
                }
        }
}

void
e8::aces_compositor::enable_auto_exposure(bool s)
{
        m_is_auto_exposure = s;
}

void
e8::aces_compositor::exposure(float e)
{
        m_e = e;
}

e8::pixel
e8::aces_compositor::pixel_of(rgba_color const& c) const
{
        unsigned char r = static_cast<unsigned char>(std::pow(CLAMP(c(0), 0.0f, 1.0f), 1.0f/2.2f)*255.0f);
        unsigned char g = static_cast<unsigned char>(std::pow(CLAMP(c(1), 0.0f, 1.0f), 1.0f/2.2f)*255.0f);
        unsigned char b = static_cast<unsigned char>(std::pow(CLAMP(c(2), 0.0f, 1.0f), 1.0f/2.2f)*255.0f);
        unsigned char a = static_cast<unsigned char>(c(3)*255.0f);
        return e8::pixel({r, g, b, a});
}

e8::rgba_color
e8::aces_compositor::aces_tonemap(rgba_color const& c, float exposure) const
{
        const float A = 2.51;
        const float B = 0.03;
        const float C = 2.43;
        const float D = 0.59;
        const float E = 0.14;

        e8util::vec3 const& color = c.trunc()*exposure;
        e8util::vec3 const& ldr = color * (color * A + B) / (color * (color * C + D) + E);
        return ldr.homo(c(3));
}

float
e8::aces_compositor::luminance(rgba_color const& c) const
{
        return c(0)*0.299f + c(1)*0.587f + c(2)*0.114f;
}

float
e8::aces_compositor::exposure() const
{
        float sum = 0;
        for (unsigned i = 0; i < m_w*m_h; i ++) {
                sum += std::log(luminance(m_fbuffer[i]) + 1e-3f);
        }
        sum /= (m_w*m_h);
        return std::exp(sum);
}
