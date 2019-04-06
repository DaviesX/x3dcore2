#ifndef OBJ_H
#define OBJ_H


#include <set>
#include <queue>
#include <vector>
#include "tensor.h"


namespace e8
{

class if_obj;

class if_obj_container
{
public:
        void    load(if_obj* obj);
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

        obj_id_t                id() const;
        std::type_info const&   type() const;
        bool                    dirty() const;

        void                    init_blueprint(std::vector<transofrm_stage_name_t> const& stages);
        bool                    update_stage(transform_stage_t const& stage);

        bool                    add_child(if_obj* child);
        bool                    remove_child(if_obj* child);

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
