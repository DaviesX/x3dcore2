#ifndef OBJDB_H
#define OBJDB_H

#include <set>
#include <queue>
#include <vector>
#include "tensor.h"
#include "scene.h"
#include "camera.h"
#include "obj.h"


namespace e8
{

class objdb
{
public:
        objdb();
        ~objdb();

        void            manage_root(if_obj* root);
        void            push_updates(if_scene* scene,
                                     if_camera* cam);
        void            clear();
private:
        void            mark_clean(if_obj* obj);

        std::set<if_obj*>       m_roots;
};

}

#endif // OBJDB_H