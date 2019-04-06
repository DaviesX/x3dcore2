#include <cassert>
#include "objdb.h"


e8::objdb::objdb()
{
}

e8::objdb::~objdb()
{
        clear();
}

void
e8::objdb::manage_root(if_obj* root)
{
        m_roots.insert(root);
}

void
e8::objdb::push_updates(if_scene* scene,
                        if_camera* cam)
{
        for (if_obj* obj: m_roots) {
                mark_clean(obj);
        }
}

void
e8::objdb::mark_clean(if_obj* obj)
{
}

void
e8::objdb::clear()
{
}
