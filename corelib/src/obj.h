#ifndef OBJ_H
#define OBJ_H

#include "tensor.h"
#include <exception>
#include <memory>
#include <set>
#include <vector>
#include <queue>

namespace e8 {

enum obj_protocol {
    obj_protocol_null,
    obj_protocol_geometry,
    obj_protocol_camera,
    obj_protocol_light,
    obj_protocol_material,
};

class incompat_obj_exception : public std::exception
{
public:
    incompat_obj_exception(obj_protocol expected_type, obj_protocol actual_type);
    ~incompat_obj_exception();
    char const *what() const noexcept;

private:
    obj_protocol m_expected_type;
    obj_protocol m_actual_type;
};

class if_obj;

class if_obj_manager
{
public:
    if_obj_manager();
    virtual ~if_obj_manager();

    virtual void load(if_obj const &obj, e8util::mat44 const &trans) = 0;
    virtual void unload(if_obj const &obj) = 0;
    virtual obj_protocol support() const = 0;
    virtual void commit() = 0;
};

typedef uint64_t obj_id_t;
typedef std::string transform_stage_name_t;
typedef std::pair<transform_stage_name_t, e8util::mat44> transform_stage_t;
typedef std::deque<transform_stage_t> transform_blueprint_t;

class if_obj
{
    friend class objdb;

public:
    virtual ~if_obj();

    virtual obj_protocol protocol() const = 0;

    obj_id_t id() const;
    bool dirty() const;

    void init_blueprint(std::vector<transform_stage_name_t> const &stages);
    bool update_stage(transform_stage_t const &stage);
    e8util::mat44 blueprint_to_transform() const;

    bool add_child(std::shared_ptr<if_obj> const &child);
    bool remove_child(std::shared_ptr<if_obj> const &child);
    std::vector<if_obj *> get_children(obj_protocol const &interface_type) const;
    std::set<std::shared_ptr<if_obj>> get_children() const;

protected:
    if_obj();
    if_obj(obj_id_t id);
    void mark_dirty();
    void mark_clean();

private:
    obj_id_t m_id;
    transform_blueprint_t m_blueprint;
    std::set<std::shared_ptr<if_obj>> m_children;
    bool m_dirty;
    char m_padding[7];
};

template<class T>
class if_copyable_obj : public if_obj
{
public:
    if_copyable_obj() {}
    if_copyable_obj(obj_id_t id) : if_obj(id) {}
    virtual ~if_copyable_obj() {}

    virtual obj_protocol protocol() const override = 0;
    virtual std::unique_ptr<T> copy() const = 0;
};

class null_obj : public if_obj
{
public:
    obj_protocol protocol() const override;
};

template<class T>
class if_operable_obj : public if_copyable_obj<T>
{
public:
    if_operable_obj() {}
    if_operable_obj(obj_id_t id) : if_copyable_obj<T>(id) {}
    virtual ~if_operable_obj() {}

    virtual obj_protocol protocol() const override = 0;
    virtual std::unique_ptr<T> copy() const override = 0;
    virtual std::unique_ptr<T> transform(e8util::mat44 const &trans) const = 0;
};

struct obj_visitor
{
    void operator()(if_obj *) {}
};

template<typename ReadOp>
void visit_filtered(if_obj *obj,
                    ReadOp op,
                    std::set<obj_protocol> const &protocol = std::set<obj_protocol>())
{
    op(obj);
    for (std::shared_ptr<if_obj> const &child : obj->get_children()) {
        if (protocol.empty() || protocol.find(obj->protocol()) != protocol.end()) {
            visit_filtered(child.get(), op, protocol);
        }
    }
}

template<typename ReadOp, typename Iterator>
void visit_all_filtered(Iterator const &begin,
                        Iterator const &end,
                        ReadOp op,
                        std::set<obj_protocol> const &protocol = std::set<obj_protocol>())
{
    for (Iterator it = begin; it != end; ++it) {
        if (protocol.empty() || protocol.find((*it)->protocol()) != protocol.end()) {
            visit_filtered((*it).get(), op, protocol);
        }
    }
}

} // namespace e8

#endif // OBJ_H
