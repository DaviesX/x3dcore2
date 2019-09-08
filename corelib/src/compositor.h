#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "frame.h"
#include "tensor.h"

namespace e8 {

typedef e8util::vec<4, float> rgba_color;

class if_compositor
{
public:
    if_compositor(unsigned width, unsigned height);
    virtual ~if_compositor();

    rgba_color operator()(unsigned i, unsigned j) const;
    rgba_color &operator()(unsigned i, unsigned j);

    unsigned width() const;
    unsigned height() const;

    void resize(unsigned w, unsigned h);

    virtual void commit(if_frame *frame) const = 0;

protected:
    pixel pixel_of(rgba_color const &c) const;

    unsigned m_w;
    unsigned m_h;
    rgba_color *m_fbuffer;
};

class clamp_compositor : public if_compositor
{
public:
    clamp_compositor(unsigned width, unsigned height);
    virtual ~clamp_compositor() override;

    void commit(if_frame *frame) const override;
};

class aces_compositor : public if_compositor
{
public:
    aces_compositor(unsigned width, unsigned height);
    virtual ~aces_compositor() override;

    void commit(if_frame *frame) const override;
    void enable_auto_exposure(bool s);
    void exposure(float e);

private:
    rgba_color aces_tonemap(rgba_color const &c, float exposure) const;
    float luminance(rgba_color const &c) const;
    float exposure() const;

    float m_e = 0.0f;
    bool m_is_auto_exposure = true;
};

} // namespace e8

#endif // COMPOSITOR_H
