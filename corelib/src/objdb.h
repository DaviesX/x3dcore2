#ifndef OBJDB_H
#define OBJDB_H

#include <set>
#include <map>
#include "obj.h"


namespace e8
{

class objdb
{
public:
        objdb();
        ~objdb();

        void                    load_manager(if_obj_manager* mgr);
        if_obj_manager*         manager_of_interface(std::type_info const& interface) const;
        if_obj*                 manage_root(if_obj* root);
        void                    push_updates();
        void                    clear();

private:
        void                    push_updates(if_obj* obj, e8util::mat44 const& global_trans);
        void                    clear(if_obj* obj);

        std::set<if_obj*>                       m_roots;
        std::map<std::string, if_obj_manager*>  m_mgrs;
};

}

#endif // OBJDB_H
