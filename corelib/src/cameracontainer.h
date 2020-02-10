#ifndef CINEMATICS_H
#define CINEMATICS_H

#include "obj.h"
#include "tensor.h"
#include <map>
#include <memory>
#include <string>

namespace e8 {
class if_camera;
}

namespace e8 {

/**
 * @brief The if_multicam class Generic interface for camera management.
 */
class camera_container : public if_obj_actuator {
  public:
    /**
     * @brief camera_container
     * @param name Human readable name of this multi-camera manager.
     */
    camera_container(std::string const &name);
    virtual ~camera_container() override;

    /**
     * @brief active_cam The last camera that was set to active by the load() call.
     * @return Currently active camera, if exists. If not it returns the nullptr.
     */
    if_camera const *active_cam() const;

    /**
     * @brief commit Nothing to update.
     */
    void commit() override;

    /**
     * @brief load Loads the camera object and set it to the active state.
     * @param obj Camera object to be loaded.
     * @param trans Camera transformation.
     */
    void load(if_obj const &obj, e8util::mat44 const &trans) override;

    /**
     * @brief unload Unloading the currently activated camera will automatically de-activate the
     * camera before removal.
     * @param obj The camera to be unloaded.
     */
    void unload(if_obj const &obj) override;

    obj_protocol support() const override;
    std::string name() const;

  protected:
    std::string m_name;
    std::unique_ptr<if_camera> m_cam;
};

} // namespace e8

#endif // CINEMATICS_H
