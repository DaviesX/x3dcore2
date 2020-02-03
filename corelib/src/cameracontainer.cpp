#include "cameracontainer.h"
#include "camera.h"
#include "util.h"
#include <utility>

e8::camera_container::camera_container(std::string const &name) : m_name(name) {}

e8::camera_container::~camera_container() {}

e8::if_camera const *e8::camera_container::active_cam() const { return m_cam.get(); }

void e8::camera_container::load(if_obj const &obj, e8util::mat44 const &trans) {
    if_camera const &cam = static_cast<if_camera const &>(obj);
    m_cam = cam.transform(trans);
}

void e8::camera_container::unload(if_obj const &obj) {
    if (m_cam != nullptr && m_cam->id() == obj.id()) {
        m_cam = nullptr;
    }
}

void e8::camera_container::commit() {}

e8::obj_protocol e8::camera_container::support() const {
    return e8::obj_protocol::obj_protocol_camera;
}

std::string e8::camera_container::name() const { return m_name; }
