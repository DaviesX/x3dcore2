#include "frame.h"


e8::if_frame::surface::surface()
{
}

e8::if_frame::surface::~surface()
{
        delete [] pixels;
}

void
e8::if_frame::surface::resize(unsigned w, unsigned h)
{
        if (this->w != w || this->h != h) {
                this->w = w;
                this->h = h;
                delete [] pixels;
                pixels = new pixel [w*h];
        }
}


e8::if_frame::if_frame()
{
        m_mutex = e8util::mutex();
}

e8::if_frame::~if_frame()
{
        e8util::destroy(m_mutex);
}

e8::if_frame::surface
e8::if_frame::front() const
{
        return m_surface[!m_spin];
}

e8::if_frame::surface
e8::if_frame::back() const
{
        return m_surface[m_spin];
}

void
e8::if_frame::swap()
{
        e8util::lock(m_mutex);
        back().resize(front().w, front().h);
        m_spin = !m_spin;
        e8util::unlock(m_mutex);
}

e8::pixel
e8::if_frame::operator()(unsigned i, unsigned j) const
{
        return back().pixels[i + j*back().w];
}

e8::pixel&
e8::if_frame::operator()(unsigned i, unsigned j)
{
        return back().pixels[i + j*back().w];
}

unsigned
e8::if_frame::width() const
{
        return back().w;
}

unsigned
e8::if_frame::height() const
{
        return back().h;
}

void
e8::if_frame::resize(unsigned w, unsigned h)
{
        // Changes only the front buffer.
        e8util::lock(m_mutex);
        front().resize(w, h);
        e8util::unlock(m_mutex);
}



e8::ram_ogl_frame::ram_ogl_frame(QWidget* parent):
        QOpenGLWidget(parent)
{
}

e8::ram_ogl_frame::~ram_ogl_frame()
{
}

void
e8::ram_ogl_frame::commit()
{
        swap();
}

void
e8::ram_ogl_frame::initializeGL()
{
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void
e8::ram_ogl_frame::resizeGL(int w, int h)
{
        static_cast<if_frame*>(this)->resize(w, h);
}

void
e8::ram_ogl_frame::paintGL()
{
        e8util::lock(m_mutex);
        glClear(GL_COLOR_BUFFER_BIT);
        if (front().w != 0 && front().h != 0)
                glDrawPixels(front().w, front().h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, front().pixels);
        e8util::unlock(m_mutex);
}
