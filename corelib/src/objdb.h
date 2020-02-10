#ifndef OBJDB_H
#define OBJDB_H

#include "obj.h"
#include "tensor.h"
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace e8 {

/**
 * @brief The objdb class The front-end storage for all objects and manager for their states.
 */
class objdb {
  public:
    objdb();
    ~objdb();

    /**
     * @brief register_actuator Register a back-end object and state manager for an object protocol.
     * If an actuator has already been registered, it will override with the new actuator.
     * @param actuator Acuator to be registered.
     */
    void register_actuator(std::unique_ptr<if_obj_actuator> actuator);

    /**
     * @brief unregister_actuator_for Unregister an actuator responsible for the specified object
     * protocol.
     * @param type The protocol to unregister the actuator for.
     */
    void unregister_actuator_for(obj_protocol type);

    /**
     * @brief actuator_of Query the currently registered actuator for the specified object protocol.
     * @param type The protocol of the actuator to query for.
     * @return The registered actuator if the it exists. If not, the nullptr.
     */
    if_obj_actuator *actuator_of(obj_protocol type) const;

    /**
     * @brief insert_root Stores the root object identified by its ID. If the ID has already
     * existed, it will override the old root object.
     * @param root Root to be stored.
     * @return raw pointer to the root stored.
     */
    if_obj *insert_root(std::shared_ptr<if_obj> const &root);

    /**
     * @brief insert_roots Much like the above, it stores multiple roots at once.
     */
    std::vector<if_obj *> insert_roots(std::vector<std::shared_ptr<if_obj>> const &roots);

    /**
     * @brief find_obj Scan the entire object DB and look for objects with the specified name and
     * protocol.
     * TODO: The search complexity is linear, but it's possible to make it more efficient via maps
     * and lazy deletion.
     * @param name Name of the objects to filter for.
     * @param type Protocol of the objects to filter for.
     * @return All objects that satisfy the above constraints.
     */
    std::vector<if_obj *> find_obj(std::string const &name, obj_protocol type);

    /**
     * @brief push_updates Push all the active dirty nodes to the underlying registered managers.
     * A node is dirty if its state is set to dirty or the parent node is dirty. A node is inactive
     * if its state is set to inactive or the parent node is inactive.
     */
    void push_updates();
    void clear();

  private:
    void push_updates(if_obj *obj, e8util::mat44 const &global_trans, bool is_dirty_anyway);

    std::set<std::shared_ptr<if_obj>> m_roots;
    std::map<obj_protocol, std::unique_ptr<if_obj_actuator>> m_actuators;
};

} // namespace e8

#endif // OBJDB_H
