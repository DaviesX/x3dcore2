#include "cinematics.h"
#include "camera.h"
#include <utility>

e8::if_cinematics::if_cinematics(std::string const &name) : m_name(name) {}

e8::if_cinematics::~if_cinematics() {}

void e8::if_cinematics::load(if_obj const &obj, e8util::mat44 const &trans) {
    if_camera const &cam = static_cast<if_camera const &>(obj);
    m_cams[obj.id()] = cam.transform(trans);
}

void e8::if_cinematics::unload(if_obj const &obj) {
    auto it = m_cams.find(obj.id());
    if (it != m_cams.end()) {
        m_cams.erase(it);
    }
}

e8::obj_protocol e8::if_cinematics::support() const { return obj_protocol::obj_protocol_camera; }

std::string e8::if_cinematics::name() const { return m_name; }

e8::stationary_cam_controller::stationary_cam_controller(std::string const &name)
    : if_cinematics(name) {}

e8::stationary_cam_controller::~stationary_cam_controller() {}

e8::if_camera *e8::stationary_cam_controller::main_cam() const {
    if (!m_cams.empty()) {
        return m_cams.begin()->second.get();
    } else {
        return nullptr;
    }
}

void e8::stationary_cam_controller::commit() {}
