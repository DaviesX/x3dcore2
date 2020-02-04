#include "objdb.h"
#include <type_traits>
#include <utility>

e8::objdb::objdb() {}

e8::objdb::~objdb() { clear(); }

void e8::objdb::register_actuator(std::unique_ptr<if_obj_actuator> actuator) {
    unregister_actuator_for(actuator->support());
    m_actuators.insert(std::make_pair(actuator->support(), std::move(actuator)));
}

void e8::objdb::unregister_actuator_for(obj_protocol type) {
    auto it = m_actuators.find(type);
    if (it != m_actuators.end()) {
        visit_all_filtered(
            m_roots.begin(), m_roots.end(), [](if_obj *obj) { obj->mark_dirty(); },
            std::set<obj_protocol>{type});
        m_actuators.erase(it);
    }
}

e8::if_obj_actuator *e8::objdb::actuator_of(obj_protocol type) const {
    auto it = m_actuators.find(type);
    if (it != m_actuators.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

e8::if_obj *e8::objdb::insert_root(std::shared_ptr<if_obj> const &root) {
    return m_roots.insert(root).first->get();
}

std::vector<e8::if_obj *>
e8::objdb::insert_roots(std::vector<std::shared_ptr<if_obj>> const &roots) {
    std::vector<e8::if_obj *> result;
    for (std::shared_ptr<if_obj> const &obj : roots) {
        result.push_back(insert_root(obj));
    }
    return result;
}

std::vector<e8::if_obj *> e8::objdb::find_obj(std::string const &name, obj_protocol type) {
    std::vector<e8::if_obj *> result;
    visit_all_filtered(
        m_roots.begin(), m_roots.end(),
        [&name, &result](e8::if_obj *obj) {
            if (obj->name() == name) {
                result.push_back(obj);
            }
        },
        std::set<obj_protocol>{type});
    return result;
}

void e8::objdb::push_updates() {
    for (std::shared_ptr<if_obj> const &obj : m_roots) {
        push_updates(obj.get(), e8util::mat44_scale(1.0f), obj->dirty());
    }
    for (auto const &it : m_actuators) {
        it.second->commit();
    }
}

void e8::objdb::push_updates(if_obj *obj, e8util::mat44 const &global_trans, bool is_dirty_anyway) {
    if (!obj->active()) {
        return;
    }

    e8util::mat44 const &modified_trans = obj->blueprint_to_transform() * global_trans;
    if (obj->dirty() || is_dirty_anyway) {
        if_obj_actuator *actuator = actuator_of(obj->protocol());
        if (actuator != nullptr) {
            actuator->unload(*obj);
            actuator->load(*obj, modified_trans);
        }
    }

    for (std::shared_ptr<if_obj> const &child : obj->m_children) {
        push_updates(child.get(), modified_trans, obj->dirty() || is_dirty_anyway);
    }

    obj->mark_clean();
}

void e8::objdb::clear() { m_roots.clear(); }
