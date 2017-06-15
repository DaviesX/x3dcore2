#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
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

e8::if_frame::surface&
e8::if_frame::front()
{
        return m_surface[!m_spin];
}

e8::if_frame::surface&
e8::if_frame::back()
{
        return m_surface[m_spin];
}

e8::if_frame::surface const&
e8::if_frame::front() const
{
        return m_surface[!m_spin];
}

e8::if_frame::surface const&
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
        if (front().w != 0 && front().h != 0) {
                unsigned* gl_pixels = new unsigned[front().w * front().h];
                for (unsigned j = 0; j < front().h; j ++) {
                        for (unsigned i = 0; i < front().w; i ++) {
                                unsigned* gl_pixel = &gl_pixels[i + (front().h - 1- j)*front().w];
                                pixel const& internal_pixel = front().pixels[i + j*front().w];
                                *gl_pixel = internal_pixel(0) << 24 |
                                                                 internal_pixel(1) << 16 |
                                                                 internal_pixel(2) << 8 |
                                                                 internal_pixel(3);
                        }
                }
                glDrawPixels(front().w, front().h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, gl_pixels);
        }
        e8util::unlock(m_mutex);
}


e8::img_file_frame::img_file_frame(std::string const& file_name, unsigned width, unsigned height):
        m_file_name(file_name)
{
        resize(width, height);
        swap();
}

e8::img_file_frame::~img_file_frame()
{
}

void
e8::img_file_frame::commit()
{
        cv::Mat4b pixels(height(), width());
        pixels.forEach([this](cv::Vec4b& v, int const* p) {
                e8::pixel const& pixel = (*this)(p[1], p[0]);
                v[0] = pixel(2);
                v[1] = pixel(1);
                v[2] = pixel(0);
                v[3] = pixel(3);
        });
        cv::imwrite(m_file_name, pixels);
        swap();
}
