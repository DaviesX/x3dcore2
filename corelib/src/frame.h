#ifndef IF_FRAME_H
#define IF_FRAME_H

#include "tensor.h"
#include "thread.h"
#include <QOpenGLWidget>
#include <vector>

namespace e8 {

typedef e8util::vec<4, unsigned char> pixel;

// Thread-safe frame buffer.
class if_frame {
  public:
    if_frame();
    virtual ~if_frame();

    pixel operator()(unsigned i, unsigned j) const;
    pixel &operator()(unsigned i, unsigned j);

    unsigned width() const;
    unsigned height() const;

    void rescale(unsigned w, unsigned h);

    virtual void commit() = 0;

  protected:
    struct surface {
        surface();
        ~surface();

        pixel *pixels = nullptr;
        unsigned w = 0;
        unsigned h = 0;

        void resize(unsigned w, unsigned h);
    };

    surface &front();
    surface &back();
    surface const &front() const;
    surface const &back() const;
    void swap();

    e8util::mutex_t m_mutex;
    surface m_surface[2];
    bool m_spin = 0;
};

class ram_ogl_frame : public if_frame, public QOpenGLWidget {
  public:
    ram_ogl_frame(QWidget *parent);
    ~ram_ogl_frame() override;

    void commit() override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

  private:
    unsigned *m_gl_pixels;
};

class img_file_frame : public if_frame {
  public:
    img_file_frame(std::string const &file_name, unsigned width, unsigned height);
    ~img_file_frame() override;

    void commit() override;

  private:
    std::string m_file_name;
};

} // namespace e8

#endif // IF_FRAME_H
