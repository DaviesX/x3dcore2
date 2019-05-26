#include <algorithm>
#include <cassert>
#include "src/obj.h"
#include "src/objdb.h"
#include "src/geometry.h"
#include "testobjdb.h"


test::test_objdb::test_objdb()
{
}

test::test_objdb::~test_objdb()
{
}

class test_geo_manger : public e8::if_obj_manager
{
public:
        ~test_geo_manger();
        void                    load(e8::if_obj const* obj,
                                     e8util::mat44 const& trans) override;
        void                    unload(e8::if_obj const* obj) override;
        e8::obj_type            support() const override;
        void                    commit() override;

        unsigned                num_geos() const;
        e8::if_geometry*        find(std::string const& name) const;
        e8::if_geometry*        find(e8::obj_id_t const& id) const;
private:
        std::vector<e8::if_geometry*>   m_geos;
};

test_geo_manger::~test_geo_manger()
{
        for (e8::if_geometry* geo: m_geos) {
                delete geo;
        }
}

void
test_geo_manger::load(e8::if_obj const* obj,
                      e8util::mat44 const& trans)
{
        e8::if_geometry const* geo = static_cast<e8::if_geometry const*>(obj);
        m_geos.push_back(geo->transform(trans));
}

void
test_geo_manger::unload(e8::if_obj const* obj)
{
        auto it = std::find_if(m_geos.begin(),
                               m_geos.end(),
                               [obj] (e8::if_geometry* geo) {
                                return obj->id() == geo->id();
                               });
        if (it != m_geos.end()) {
                m_geos.erase(it);
        }
}

e8::obj_type
test_geo_manger::support() const
{
        return e8::obj_type::obj_type_geometry;
}

unsigned
test_geo_manger::num_geos() const
{
        return static_cast<unsigned>(m_geos.size());
}

e8::if_geometry*
test_geo_manger::find(std::string const& name) const
{
        auto it = std::find_if(m_geos.begin(),
                               m_geos.end(),
                               [&name] (e8::if_geometry* geo) {
                                return geo->name() == name;
                               });
        if (it != m_geos.end()) {
                return *it;
        } else {
                return nullptr;
        }
}

e8::if_geometry*
test_geo_manger::find(e8::obj_id_t const& id) const
{
        auto it = std::find_if(m_geos.begin(),
                                       m_geos.end(),
                                       [&id] (e8::if_geometry* geo) {
                                        return geo->id() == id;
                                       });
        if (it != m_geos.end()) {
                return *it;
        } else {
                return nullptr;
        }
}

void
test_geo_manger::commit() {}

void
test::test_objdb::run() const
{
        e8::if_geometry* root = new e8::triangle_fragment("tri",
                                                          e8util::vec3 {0, 0, 1},
                                                          e8util::vec3 {1, 1, -1},
                                                          e8util::vec3 {-1, -1, -1});
        root->init_blueprint(std::vector<e8::transform_stage_name_t> {"r"});
        root->update_stage(std::make_pair("r",
                                          e8util::mat44_rotate(e8util::deg2rad(90),
                                                               e8util::vec3 {0, 0, 1})));
        e8::if_geometry* child = new e8::triangle_fragment("tri2",
                                                           e8util::vec3 {0, 0, 1},
                                                           e8util::vec3 {1, 1, -1},
                                                           e8util::vec3 {-1, -1, -1});
        child->init_blueprint(std::vector<e8::transform_stage_name_t> {"r"});
        child->update_stage(std::make_pair("r",
                                           e8util::mat44_rotate(e8util::deg2rad(90),
                                                                e8util::vec3 {0, 0, 1})));
        root->add_child(child);

        e8::objdb db;
        db.register_manager(std::make_unique<test_geo_manger>());
        db.manage_root(root);
        db.push_updates();

        test_geo_manger* mgr = static_cast<test_geo_manger*>(
                                db.manager_for(e8::obj_type::obj_type_geometry));
        assert(mgr->num_geos() == 2);
        e8::if_geometry* t_root_id = mgr->find(root->id());
        e8::if_geometry* t_child_id = mgr->find(child->id());
        assert(t_root_id != nullptr);
        assert(t_child_id != nullptr);

        e8::if_geometry* t_root_n = mgr->find("tri");
        e8::if_geometry* t_child_n = mgr->find("tri2");
        assert(t_root_n != nullptr);
        assert(t_child_n != nullptr);
        assert(t_root_n == t_root_id);
        assert(t_child_n == t_child_id);

        // Expected geometries:
        e8::triangle_fragment expected_root("tri",
                                            e8util::vec3 {0, 0, 1},
                                            e8util::vec3 {-1, 1, -1},
                                            e8util::vec3 {1, -1, -1});
        e8::triangle_fragment expected_child("tri2",
                                             e8util::vec3 {0, 0, 1},
                                             e8util::vec3 {-1, -1, -1},
                                             e8util::vec3 {1, 1, -1});
        assert(e8util::equals(expected_root.vertices(), t_root_n->vertices()));
        assert(e8util::equals(expected_root.normals(), t_root_n->normals()));

        assert(e8util::equals(expected_child.vertices(), t_child_n->vertices()));
        assert(e8util::equals(expected_child.normals(), t_child_n->normals()));
}
