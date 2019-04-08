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
e8::objdb::push_updates()
{
        for (if_obj* obj: m_roots) {
                push_updates(obj, e8util::mat44_scale(1.0f));
        }
}

void
e8::objdb::push_updates(if_obj* obj, e8util::mat44 const& global_trans)
{
        e8util::mat44 local_trans = e8util::mat44_scale(1.0f);
        for (auto it = obj->m_blueprint.begin(); it != obj->m_blueprint.end(); ++it) {
                local_trans = it->second*local_trans;
        }
        e8util::mat44 const& modified_trans = local_trans*global_trans;
        if (obj->dirty()) {
                if_obj_manager* mgr = obj->manage_by();
                mgr->unload(obj);
                mgr->load(obj, modified_trans);
                obj->mark_clean();
        }
        for (if_obj* child: obj->m_children) {
                push_updates(child, modified_trans);
        }
}

void
e8::objdb::clear(if_obj* obj)
{
        for (if_obj* child: obj->m_children) {
                clear(child);
        }
        if_obj_manager* mgr = obj->manage_by();
        if (mgr != nullptr) {
                mgr->unload(obj);
        }
        delete obj;
}

void
e8::objdb::clear()
{
        for (if_obj* root: m_roots) {
                clear(root);
        }
        m_roots.clear();
}
