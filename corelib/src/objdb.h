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
        void            push_updates();
        void            clear();
private:
        void            push_updates(if_obj* obj, e8util::mat44 const& global_trans);
        void            clear(if_obj* obj);

        std::set<if_obj*>       m_roots;
};

}

#endif // OBJDB_H
