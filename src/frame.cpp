#include "frame.h"


e8::if_frame::if_frame()
{
}

e8::if_frame::~if_frame()
{
}



e8::ram_ogl_frame::ram_ogl_frame(QWidget* parent):
        QOpenGLWidget(parent)
{
        m_mutex = e8util::mutex();
        m_pixels[0] = nullptr;
        m_pixels[1] = nullptr;
}

e8::ram_ogl_frame::~ram_ogl_frame()
{
        e8util::destroy(m_mutex);
        delete [] m_pixels[0];
        delete [] m_pixels[1];
}

e8::pixel
e8::ram_ogl_frame::operator()(unsigned i, unsigned j) const
{
        return m_pixels[m_spin][i + j*m_w];
}

e8::pixel&
e8::ram_ogl_frame::operator()(unsigned i, unsigned j)
{
        return m_pixels[m_spin][i + j*m_w];
}

unsigned
e8::ram_ogl_frame::width() const
{
        return m_w;
}

unsigned
e8::ram_ogl_frame::height() const
{
        return m_h;
}

void
e8::ram_ogl_frame::width(unsigned w)
{
        e8util::lock(m_mutex);
        if (m_w != w) {
                m_w = w;
                delete [] m_pixels[0];
                delete [] m_pixels[1];
                m_pixels[0] = new pixel [m_w*m_h];
                m_pixels[1] = new pixel [m_w*m_h];
        }
        e8util::unlock(m_mutex);
}

void
e8::ram_ogl_frame::height(unsigned h)
{
        e8util::lock(m_mutex);
        if (m_h != h) {
                m_h = h;
                delete [] m_pixels[0];
                delete [] m_pixels[1];
                m_pixels[0] = new pixel [m_w*m_h];
                m_pixels[1] = new pixel [m_w*m_h];
        }
        e8util::unlock(m_mutex);
}

void
e8::ram_ogl_frame::commit()
{
        m_spin = !m_spin;
}

void
e8::ram_ogl_frame::initializeGL()
{
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void
e8::ram_ogl_frame::resizeGL(int w, int h)
{
        width(w);
        height(h);
}

void
e8::ram_ogl_frame::paintGL()
{
        e8util::lock(m_mutex);
        glClear(GL_COLOR_BUFFER_BIT);
        if (m_w != 0 && m_h != 0)
                glDrawPixels(m_w, m_h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, m_pixels[!m_spin]);
        e8util::unlock(m_mutex);
}
