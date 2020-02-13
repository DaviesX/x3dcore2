#include "materialcontainer.h"

e8::if_material_container::if_material_container() : m_fail_safe("fail_safe") {}

e8::if_material_container::~if_material_container() {}

e8::if_material const &e8::if_material_container::find(std::optional<obj_id_t> mat_id) const {
    if (!mat_id.has_value()) {
        return m_fail_safe;
    }

    auto it = m_mats.find(*mat_id);
    if (it == m_mats.end()) {
        return m_fail_safe;
    }
    return *it->second;
}

void e8::if_material_container::load(if_obj const &obj, e8util::mat44 const & /*trans*/) {
    if_material const &mat = static_cast<if_material const &>(obj);
    std::shared_ptr<if_material> copy = mat.copy();
    m_mats.insert(std::make_pair(mat.id(), copy));
}

void e8::if_material_container::unload(if_obj const &obj) {
    auto it = m_mats.find(obj.id());
    if (it != m_mats.end()) {
        m_mats.erase(it);
    }
}

e8::obj_protocol e8::if_material_container::support() const {
    return obj_protocol::obj_protocol_material;
}

e8::default_material_container::default_material_container() {}

e8::default_material_container::~default_material_container() {}

void e8::default_material_container::commit() {}
