#ifndef OBJ_H
#define OBJ_H


#include <set>
#include <queue>
#include <vector>
#include <exception>
#include "tensor.h"


namespace e8
{

class incompat_obj_exception: public std::exception
{
public:
        incompat_obj_exception(std::type_info const& expected_type,
                               std::type_info const& actual_type);
        ~incompat_obj_exception();
        char const*     what() const noexcept;
private:
        std::type_info const&   m_expected_type;
        std::type_info const&   m_actual_type;
};

class if_obj;

class if_obj_manager
{
public:
        if_obj_manager();
        virtual ~if_obj_manager();

        virtual void                    load(if_obj const* obj, e8util::mat44 const& trans) = 0;
        virtual void                    unload(if_obj const* obj) = 0;
        virtual const std::type_info&   support() const = 0;
};

typedef uint32_t                                                obj_id_t;
typedef std::string                                             transofrm_stage_name_t;
typedef std::pair<transofrm_stage_name_t, e8util::mat44>        transform_stage_t;
typedef std::deque<transform_stage_t>                           transform_blueprint_t;

class if_obj
{
        friend class objdb;
public:
        virtual ~if_obj();

        virtual std::type_info const&   interface() const = 0;

        obj_id_t                        id() const;
        std::type_info const&           type() const;
        bool                            dirty() const;
        if_obj_manager*                 manage_by() const;
        void                            manage_by(if_obj_manager* mgr);

        void                            init_blueprint(std::vector<transofrm_stage_name_t> const& stages);
        bool                            update_stage(transform_stage_t const& stage);

        bool                            add_child(if_obj* child);
        bool                            remove_child(if_obj* child);
        std::vector<if_obj*>            get_children(std::type_info const& interface_type) const;
protected:
        if_obj();
        if_obj(obj_id_t id);
        void                    mark_dirty();
        void                    mark_clean();
private:
        obj_id_t                        m_id;
        transform_blueprint_t           m_blueprint;
        if_obj_manager*                 m_mgr;
        if_obj*                         m_parent;
        std::set<if_obj*>               m_children;
        bool                            m_dirty;
};

template<class T>
class if_operable_obj: public if_obj
{
public:
        if_operable_obj() {}
        if_operable_obj(obj_id_t id): if_obj(id) {}
        virtual ~if_operable_obj() {}

        virtual T*      transform(e8util::mat44 const& trans) const = 0;
};

}

#endif // OBJ_H
