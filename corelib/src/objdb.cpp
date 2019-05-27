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
e8::objdb::register_manager(std::unique_ptr<if_obj_manager> mgr)
{
        unregister_manager_for(mgr->support());
        m_mgrs.insert(std::make_pair(mgr->support(), std::move(mgr)));
}

void
e8::objdb::unregister_manager_for(obj_type type)
{
        auto it = m_mgrs.find(type);
        if (it != m_mgrs.end()) {
                visit_all_filtered(m_roots.begin(),
                                   m_roots.end(),
                                   [] (if_obj* obj) { obj->mark_dirty(); },
                                   std::set<obj_type> {type});
                m_mgrs.erase(it);
        }
}

e8::if_obj_manager*
e8::objdb::manager_for(obj_type type) const
{
        auto it = m_mgrs.find(type);
        if (it != m_mgrs.end()) {
                return it->second.get();
        } else {
                return nullptr;
        }
}

e8::if_obj*
e8::objdb::manage_root(if_obj* root)
{
        return *m_roots.insert(root).first;
}

std::vector<e8::if_obj*>
e8::objdb::manage_roots(std::vector<if_obj*> const& roots)
{
        std::vector<e8::if_obj*> result;
        for (if_obj* obj: roots) {
                result.push_back(manage_root(obj));
        }
        return result;
}

void
e8::objdb::push_updates()
{
        for (if_obj* obj: m_roots) {
                push_updates(obj,
                             e8util::mat44_scale(1.0f),
                             obj->dirty());
        }
        for (auto const& it: m_mgrs) {
                it.second->commit();
        }
}

void
e8::objdb::push_updates(if_obj* obj,
                        e8util::mat44 const& global_trans,
                        bool is_dirty_anyway)
{
        e8util::mat44 const& modified_trans = obj->blueprint_to_transform()*global_trans;
        if (obj->dirty() || is_dirty_anyway) {
                if_obj_manager* mgr = manager_for(obj->interface());
                if (mgr != nullptr) {
                        mgr->unload(obj);
                        mgr->load(obj, modified_trans);
                }
        }

        for (if_obj* child: obj->m_children) {
                push_updates(child,
                             modified_trans,
                             obj->dirty() || is_dirty_anyway);
        }

        obj->mark_clean();
}

void
e8::objdb::clear(if_obj* obj)
{
        for (if_obj* child: obj->m_children) {
                clear(child);
        }
        if_obj_manager* mgr = manager_for(obj->interface());
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
