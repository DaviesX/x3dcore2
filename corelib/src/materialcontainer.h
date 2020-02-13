#ifndef MATERIALCONTAINER_H
#define MATERIALCONTAINER_H

#include "material.h"
#include "obj.h"
#include <memory>
#include <optional>
#include <unordered_map>

namespace e8 {

/**
 * @brief The if_material_container class Serves as a container to store all the materials.
 */
class if_material_container : public if_obj_actuator {
  public:
    if_material_container();
    virtual ~if_material_container() override;

    /**
     * @brief find Fast lookup the loaded materials by an ID.
     * @param mat_id ID of the material to look for.
     * @return Reference to the material if it exits. If not, returns the reference to a failed safe
     * material.
     */
    if_material const &find(std::optional<obj_id_t> mat_id) const;

    void load(if_obj const &obj, e8util::mat44 const &trans) override;
    void unload(if_obj const &obj) override;
    obj_protocol support() const override;
    virtual void commit() override = 0;

  protected:
    // Stores all the materials.
    std::unordered_map<obj_id_t, std::shared_ptr<if_material>> m_mats;
    // Failed safe material.
    mat_fail_safe m_fail_safe;
};

/**
 * @brief The default_material_container class Default implementation of the if_material_container
 * interface.
 */
class default_material_container : public if_material_container {
  public:
    default_material_container();
    ~default_material_container() override;

    /**
     * @brief commit Does nothing.
     */
    void commit() override;
};

} // namespace e8

#endif // MATERIALCONTAINER_H
