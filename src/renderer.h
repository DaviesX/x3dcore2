#ifndef RENDERER_H
#define RENDERER_H

#include "scene.h"
#include "camera.h"
#include "frame.h"

namespace e8
{

class if_im_renderer
{
public:
        if_im_renderer();
        ~if_im_renderer();

        virtual void    render(if_scene const& scene, camera const& cam, if_frame& frame) const = 0;
};

}

#endif // IF_RENDERER_H
