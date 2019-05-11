#include <cassert>
#include "thread.h"
#include "obj.h"


e8::incompat_obj_exception::incompat_obj_exception(std::type_info const& expected_type,
                                                   std::type_info const& actual_type):
        m_expected_type(expected_type),
        m_actual_type(actual_type)
{
}

e8::incompat_obj_exception::~incompat_obj_exception()
{
}

char const*
e8::incompat_obj_exception::what() const noexcept
{
        return (std::string("Incompatiable obj support. Expected: ") + m_expected_type.name()
                + ", Actual: " + m_actual_type.name()).c_str();
}


e8::if_obj_manager::if_obj_manager()
{
}

e8::if_obj_manager::~if_obj_manager()
{
}


static e8::obj_id_t             g_obj_id_counter = 101;
static e8util::mutex_t          g_obj_id_counter_mutex = e8util::mutex();

static e8::obj_id_t
next_obj_id()
{
        e8::obj_id_t next_id;
        e8util::lock(g_obj_id_counter_mutex);
        next_id = g_obj_id_counter ++;
        e8util::unlock(g_obj_id_counter_mutex);
        return next_id;
}


e8::if_obj::if_obj():
        if_obj(next_obj_id())
{
}

e8::if_obj::if_obj(obj_id_t id):
        m_id(id),
        m_parent(nullptr),
        m_dirty(true)
{
}

e8::if_obj::~if_obj()
{
}

e8::obj_id_t
e8::if_obj::id() const
{
        return m_id;
}

void
e8::if_obj::init_blueprint(std::vector<transform_stage_name_t> const& stages)
{
        assert(std::set<std::string>(stages.begin(), stages.end()).size() == stages.size());
        m_blueprint.clear();
        for (unsigned i = 0; i < stages.size(); i ++) {
                m_blueprint.push_back(std::make_pair(stages[i],
                                                     e8util::mat44_scale(1.0f)));
        }
}

bool
e8::if_obj::update_stage(transform_stage_t const& stage)
{
        for (auto it = m_blueprint.begin(); it != m_blueprint.end(); ++ it) {
                if (it->first == stage.first &&
                                it->second != stage.second) {
                        it->second = stage.second;
                        mark_dirty();
                        return true;
                }
        }
        return false;
}

void
e8::if_obj::mark_dirty()
{
        m_dirty = true;
}

void
e8::if_obj::mark_clean()
{
        m_dirty = false;
}

bool
e8::if_obj::dirty() const
{
        return m_dirty;
}

bool
e8::if_obj::add_child(if_obj* child)
{
        auto it = m_children.find(child);
        if (it == m_children.end()) {
                child->m_parent = this;
                m_children.insert(child);
                return true;
        } else {
                return false;
        }
}

bool
e8::if_obj::remove_child(if_obj* child)
{
        auto it = m_children.find(child);
        if (it != m_children.end()) {
                child->m_parent = nullptr;
                m_children.erase(it);
                return true;
        } else {
                return false;
        }
}


std::vector<e8::if_obj*>
e8::if_obj::get_children(std::type_info const& interface_type) const
{
        std::vector<e8::if_obj*> result;
        for (if_obj* obj: m_children) {
                if (obj->interface() == interface_type) {
                        result.push_back(obj);
                }
        }
        return result;
}

std::set<e8::if_obj*>
e8::if_obj::get_children() const
{
        return m_children;
}

std::type_info const&
e8::null_obj::interface() const
{
        return typeid(null_obj);
}
