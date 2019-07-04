#ifndef OBJDB_H
#define OBJDB_H

#include <set>
#include <map>
#include <memory>
#include "obj.h"


namespace e8
{

class objdb
{
public:
        objdb();
        ~objdb();

        void                    register_manager(std::unique_ptr<if_obj_manager> mgr);
        void                    unregister_manager_for(obj_protocol type);
        if_obj_manager*         manager_of(obj_protocol type) const;
        if_obj*                 manage_root(std::shared_ptr<if_obj> const& root);
        std::vector<if_obj*>    manage_roots(std::vector<std::shared_ptr<if_obj>> const& roots);
        void                    push_updates();
        void                    clear();

private:
        void                    push_updates(if_obj* obj,
                                             e8util::mat44 const& global_trans,
                                             bool is_dirty_anyway);

        std::set<std::shared_ptr<if_obj>>                       m_roots;
        std::map<obj_protocol, std::unique_ptr<if_obj_manager>> m_mgrs;
};

}

#endif // OBJDB_H
