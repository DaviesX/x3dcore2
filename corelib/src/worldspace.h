#ifndef GEOEMTRYBUF_H
#define GEOEMTRYBUF_H

#include "obj.h"

namespace e8 {

/**
 * @brief The if_world_space class Holds all the geometries and is able to tell whether the geometry
 * lies in some 3D subregion of the entire world.
 */
class if_world_space : public if_obj_actuator {
  public:
    if_world_space() = default;
    ~if_world_space() override = default;

    void load(if_obj const &obj, e8util::mat44 const &trans) override;
    void unload(if_obj const &obj) override;
    obj_protocol support() const override;
    void commit() override;

  private:
};

} // namespace e8

#endif // GEOEMTRYBUF_H
