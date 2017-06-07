#ifndef IF_FRAME_H
#define IF_FRAME_H

#include <QOpenGLWidget>
#include <vector>
#include "tensor.h"
#include "thread.h"


namespace e8
{

typedef e8util::vec<4, unsigned char>   pixel;

class if_frame
{
public:
        if_frame();
        virtual ~if_frame();

        virtual pixel           operator()(unsigned i, unsigned j) const = 0;
        virtual pixel&          operator()(unsigned i, unsigned j) = 0;

        virtual unsigned        width() const = 0;
        virtual unsigned        height() const = 0;

        virtual void            width(unsigned w) = 0;
        virtual void            height(unsigned h) = 0;

        virtual void            commit() = 0;
};

class ram_ogl_frame: public if_frame, public QOpenGLWidget
{
public:
        ram_ogl_frame(QWidget* parent);
        ~ram_ogl_frame();

        pixel           operator()(unsigned i, unsigned j) const override;
        pixel&          operator()(unsigned i, unsigned j) override;

        unsigned        width() const override;
        unsigned        height() const override;

        void            width(unsigned w) override;
        void            height(unsigned h) override;

        void            commit() override;

        void            initializeGL() override;
        void            resizeGL(int w, int h) override;
        void            paintGL() override;
private:
        unsigned                        m_w = 0;
        unsigned                        m_h = 0;
        e8util::mutex_t                 m_mutex;
        pixel*                          m_pixels[2];
        bool                            m_spin = 0;
};

}

#endif // IF_FRAME_H
