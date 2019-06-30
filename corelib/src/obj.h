#ifndef OBJ_H
#define OBJ_H

#include <memory>
#include <set>
#include <queue>
#include <vector>
#include <exception>
#include "tensor.h"


namespace e8
{

enum obj_type {
        obj_type_null,
        obj_type_geometry,
        obj_type_camera,
        obj_type_light,
        obj_type_material,
};

class incompat_obj_exception: public std::exception
{
public:
        incompat_obj_exception(obj_type expected_type,
                               obj_type actual_type);
        ~incompat_obj_exception();
        char const*     what() const noexcept;
private:
        obj_type        m_expected_type;
        obj_type        m_actual_type;
};

class if_obj;

class if_obj_manager
{
public:
        if_obj_manager();
        virtual ~if_obj_manager();

        virtual void                    load(if_obj const* obj, e8util::mat44 const& trans) = 0;
        virtual void                    unload(if_obj const* obj) = 0;
        virtual obj_type                support() const = 0;
        virtual void                    commit() = 0;
};

typedef uint64_t                                                obj_id_t;
typedef std::string                                             transform_stage_name_t;
typedef std::pair<transform_stage_name_t, e8util::mat44>        transform_stage_t;
typedef std::deque<transform_stage_t>                           transform_blueprint_t;

class if_obj
{
        friend class objdb;
public:
        virtual ~if_obj();

        virtual obj_type                interface() const = 0;

        obj_id_t                        id() const;
        bool                            dirty() const;

        void                            init_blueprint(std::vector<transform_stage_name_t> const& stages);
        bool                            update_stage(transform_stage_t const& stage);
        e8util::mat44                   blueprint_to_transform() const;

        bool                            add_child(if_obj* child);
        bool                            remove_child(if_obj* child);
        std::vector<if_obj*>            get_children(obj_type const& interface_type) const;
        std::set<if_obj*>               get_children() const;
protected:
        if_obj();
        if_obj(obj_id_t id);
        void                    mark_dirty();
        void                    mark_clean();
private:
        obj_id_t                        m_id;
        transform_blueprint_t           m_blueprint;
        if_obj*                         m_parent;
        std::set<if_obj*>               m_children;
        bool                            m_dirty;
        char                            m_padding[7];
};

template<class T>
class if_copyable_obj: public if_obj
{
public:
        if_copyable_obj() {}
        if_copyable_obj(obj_id_t id): if_obj(id) {}
        virtual ~if_copyable_obj() {}
        virtual std::unique_ptr<T>      copy() const = 0;
};

class null_obj: public if_obj
{
public:
        obj_type        interface() const override;
};

template<class T>
class if_operable_obj: public if_copyable_obj<T>
{
public:
        if_operable_obj() {}
        if_operable_obj(obj_id_t id): if_copyable_obj<T>(id) {}
        virtual ~if_operable_obj() {}

        virtual std::unique_ptr<T>      copy() const override = 0;
        virtual std::unique_ptr<T>      transform(e8util::mat44 const& trans) const = 0;
};

struct obj_visitor
{
        void operator()(if_obj*) {}
};


template<typename ReadOp>
void
visit_filtered(if_obj* obj,
               ReadOp op,
               std::set<obj_type> const& interfaces = std::set<obj_type>())
{
        op(obj);
        for (if_obj* child: obj->get_children()) {
                if (interfaces.empty() ||
                    interfaces.find(obj->interface()) != interfaces.end()) {
                        visit_filtered(child, op, interfaces);
                }
        }
}


template<typename ReadOp, typename Iterator>
void
visit_all_filtered(Iterator const& begin,
                   Iterator const& end,
                   ReadOp op,
                   std::set<obj_type> const& interfaces = std::set<obj_type>())
{
        for (Iterator it = begin; it != end; ++ it) {
                if (interfaces.empty() ||
                    interfaces.find((*it)->interface()) != interfaces.end()) {
                        visit_filtered(*it, op, interfaces);
                }
        }
}

}

#endif // OBJ_H
