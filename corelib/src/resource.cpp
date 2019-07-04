#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#pragma GCC diagnostic ignored "-Wshift-negative-value"

#include "../thirdparty/tinygltf/tiny_gltf.h"

#pragma GCC diagnostic pop

#undef TINYGLTF_IMPLEMENTATION
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#include "geometry.h"
#include "material.h"
#include "resource.h"


e8util::res_io_exception::res_io_exception(std::string const& cause):
        m_cause(cause)
{
}

char const*
e8util::res_io_exception::what() const noexcept
{
        return m_cause.c_str();
}

e8util::if_resource::if_resource()
{
}

e8util::if_resource::~if_resource()
{
}

std::vector<std::shared_ptr<e8::if_obj>>
e8util::if_resource::load_roots()
{
        throw std::string("resource doesn't support load_roots");
}

void
e8util::if_resource::save_roots(std::vector<std::shared_ptr<e8::if_obj>> const& /* roots */)
{
        throw std::string("resource doesn't support save_roots");
}



e8util::cornell_scene::cornell_scene()
{
}

std::vector<std::shared_ptr<e8::if_geometry>>
static cornell_scene_load_geometries()
{
        std::vector<std::shared_ptr<e8::if_geometry>> geometries(8);
        geometries[0] = e8util::wavefront_obj("res/cornellbox/left_wall.obj").load_geometry();
        geometries[1] = e8util::wavefront_obj("res/cornellbox/right_wall.obj").load_geometry();
        geometries[2] = e8util::wavefront_obj("res/cornellbox/ceiling.obj").load_geometry();
        geometries[3] = e8util::wavefront_obj("res/cornellbox/floor.obj").load_geometry();
        geometries[4] = e8util::wavefront_obj("res/cornellbox/back_wall.obj").load_geometry();
        geometries[5] = e8util::wavefront_obj("res/cornellbox/left_sphere.obj").load_geometry();
        geometries[6] = e8util::wavefront_obj("res/cornellbox/right_sphere.obj").load_geometry();
        geometries[7] = e8util::wavefront_obj("res/cornellbox/light.obj").load_geometry();
        return geometries;
}

std::vector<std::shared_ptr<e8::if_material>>
static cornell_scene_load_materials()
{
        std::shared_ptr<e8::if_material> white = std::make_shared<e8::oren_nayar>(e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
        std::shared_ptr<e8::if_material> red = std::make_shared<e8::oren_nayar>(e8util::vec3({0.630f, 0.065f, 0.050f}), 0.078f);
        std::shared_ptr<e8::if_material> green = std::make_shared<e8::oren_nayar>(e8util::vec3({0.140f, 0.450f, 0.091f}), 0.078f);
        std::shared_ptr<e8::if_material> glossy = std::make_shared<e8::cook_torr>(e8util::vec3({0.787f, 0.787f, 0.787f}), 0.25f, 2.93f);
        std::shared_ptr<e8::if_material> light = std::make_shared<e8::oren_nayar>(e8util::vec3({0, 0, 0}), 0.078f);
        return std::vector<std::shared_ptr<e8::if_material>>({red, green, white, white, white, glossy, white, light});
}

std::vector<std::shared_ptr<e8::if_light>>
static cornell_scene_load_lights(e8::if_geometry const& light_geo)
{
        return std::vector<std::shared_ptr<e8::if_light>>({nullptr,
                                                           nullptr,
                                                           nullptr,
                                                           nullptr,
                                                           nullptr,
                                                           nullptr,
                                                           nullptr,
                                                         std::make_shared<e8::area_light>(
                                                           light_geo,
                                                           e8util::vec3({0.911f, 0.660f, 0.345f})*15.0f)
                                                          });
}

std::vector<std::shared_ptr<e8::if_light>>
static cornell_scene_load_virtual_lights()
{
        return std::vector<std::shared_ptr<e8::if_light>>();
}

std::shared_ptr<e8::if_camera>
static cornell_scene_load_camera()
{
        /*return new e8::pinhole_camera(e8util::vec3({0.0f, -3.4f, 0.795f}),
                                      e8util::mat44_rotate(M_PI/2.0f, e8util::vec3({1, 0, 0})),
                                      0.032f, 0.035f, 4.0f/3.0f);*/
        e8util::mat44 trans = e8util::mat44_translate({0.0f, 0.795f, 3.4f});
        e8util::mat44 rot = e8util::mat44_rotate(0.0f, {0, 0, 1});
        std::shared_ptr<e8::if_camera> cam =
                        std::make_shared<e8::pinhole_camera>("cornell_cam", 0.032f, 0.035f, 4.0f/3.0f);
        cam->init_blueprint({"rotation", "translation"});
        cam->update_stage(std::make_pair("rotation", rot));
        cam->update_stage(std::make_pair("translation", trans));
        return cam;
}

std::vector<std::shared_ptr<e8::if_obj>>
e8util::cornell_scene::load_roots()
{
        std::vector<std::shared_ptr<e8::if_geometry>> geometries = cornell_scene_load_geometries();
        std::vector<std::shared_ptr<e8::if_material>> mats = cornell_scene_load_materials();
        std::vector<std::shared_ptr<e8::if_light>> obj_lights = cornell_scene_load_lights(*geometries[7]);
        std::shared_ptr<e8::if_camera> cams = cornell_scene_load_camera();
        std::vector<std::shared_ptr<e8::if_obj>> roots;
        roots.push_back(cams);
        for (unsigned i = 0; i < geometries.size(); i ++) {
                geometries[i]->add_child(mats[i]);
                if (obj_lights[i] != nullptr) {
                        geometries[i]->add_child(obj_lights[i]);
                }
                roots.push_back(geometries[i]);
        }
        std::vector<std::shared_ptr<e8::if_light>> virtual_lights = cornell_scene_load_virtual_lights();
        for (unsigned i = 0; i < virtual_lights.size(); i ++) {
                roots.push_back(virtual_lights[i]);
        }
        return roots;
}



e8util::wavefront_obj::wavefront_obj(std::string const& location):
        m_location(location)
{
}

e8util::wavefront_obj::~wavefront_obj()
{
}

std::vector<std::string>
split(std::string const& s, char delim)
{
        std::vector<std::string> parts;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim))
                parts.push_back(item);
        return parts;
}

std::shared_ptr<e8::if_geometry>
e8util::wavefront_obj::load_geometry() const
{
        std::ifstream file(m_location);
        if (!file.is_open()) {
                std::perror(("wavefront_obj::load_geometries to " + m_location).c_str());
                return nullptr;
        }

        // 1. The obj data is assumed to be all triangulated.
        // 2. default id is used.

        // Temps.
        std::vector<e8util::vec3> vertices;
        std::vector<e8util::vec3> normals;
        std::vector<e8util::vec2> texcoords;

        std::vector<unsigned> iverts;
        std::vector<unsigned> inorms;
        std::vector<unsigned> itex;

        std::regex re_vert("v\\s+(.+)\\s+(.+)\\s+(.+)\\s*|\\n");
        std::regex re_norm("vn\\s+(.+)\\s+(.+)\\s+(.+)\\s*|\\n");
        std::regex re_tex("vt\\s+(.+)\\s+(.+)\\s*|\\n");
        std::regex re_face("f\\s+(.+)\\s+(.+)\\s+(.+)\\s*|\\n");

        unsigned i = 0;
        std::string line;
        while (std::getline(file, line)) {
                try {
                        std::smatch result;
                        if (std::regex_match(line, result, re_vert)) {
                                vertices.push_back(e8util::vec3({std::stof(result[1]), std::stof(result[2]), std::stof(result[3])}));
                        } else if (std::regex_match(line, result, re_norm)) {
                                normals.push_back(e8util::vec3({std::stof(result[1]), std::stof(result[2]), std::stof(result[3])}));
                        } else if (std::regex_match(line, result, re_tex)) {
                                texcoords.push_back(e8util::vec2({std::stof(result[1]), std::stof(result[2])}));
                        } else if (std::regex_match(line, result, re_face)) {
                                if (result.size() != 3 + 1) {
                                        // This face is not acceptible.
                                        std::cerr << "load_from_obj_str() - at line " << (i + 1)
                                                  << ". Couldn't accept polygon other than triangle." << std::endl;
                                        continue;
                                }

                                for (unsigned v = 0; v < 3; v ++) {
                                        std::string f = result[v + 1];
                                        std::vector<std::string> s_index = split(f, '/');\
                                        if (s_index.size() != 3)
                                                throw "Malformed data at line " + std::to_string(i + 1)
                                                        + " where attribute " + std::to_string(v + 1)
                                                        + " doesn't have at least 3 vertex attributes";

                                        if (s_index[0].empty())
                                                throw "Malformed data at line " + std::to_string(i + 1)
                                                                + " where vertex index is missing.";
                                        else {
                                                unsigned iattri = std::stoul(s_index[0]) - 1;
                                                if (iattri >= vertices.size())
                                                        throw "At line " + std::to_string(i + 1)
                                                                        + ", attribute " + std::to_string(v + 1)
                                                                        + " referenced vertex " + std::to_string(iattri + 1)
                                                                        + " is illegal.";
                                                iverts.push_back(iattri);
                                        }

                                        if (!s_index[1].empty()) {
                                                unsigned iattri = std::stoul(s_index[1]) - 1;
                                                if (iattri >= texcoords.size())
                                                        throw "At line " + std::to_string(i + 1)
                                                                        + ", attribute " + std::to_string(v + 1)
                                                                        + " referenced texcoord " + std::to_string(iattri + 1)
                                                                        + " is illegal.";
                                                itex.push_back(iattri);
                                        }

                                        if (!s_index[2].empty()) {
                                                unsigned iattri = std::stoul(s_index[2]) - 1;
                                                if (iattri >= normals.size())
                                                        throw "At line " + std::to_string(i + 1)
                                                                        + ", attribute " + std::to_string(v + 1)
                                                                        + " referenced normal " + std::to_string(iattri + 1)
                                                                        + " is illegal.";
                                                inorms.push_back(iattri);
                                        }
                                }
                        }
                } catch (std::string const& e) {
                        throw "Malformed data at line " + std::to_string(i + 1) + ". nested exception: " + e;
                }
                i ++;
        }

        // Assemble the face indices with vertex attributes -- shift vertex data to the proper location.
        if (vertices.empty())
                throw std::string("The mesh doesn't contain vertex data");

        // Fill up the mesh.
        std::shared_ptr<e8::trimesh> mesh = std::make_shared<e8::trimesh>();

        // Vertices are already in the right place.
        mesh->vertices(vertices);

        std::vector<e8util::vec3> packed_norms(vertices.size());
        std::vector<e8util::vec2> packed_tex(vertices.size());
        std::vector<e8::triangle> triangles(iverts.size()/3);

        for (unsigned f = 0; f < iverts.size()/3; f ++) {
                triangles[f] = e8::triangle(&iverts[f*3 + 0]);

                for (unsigned v = 3*f + 0; v < 3*f + 3; v ++) {
                        packed_norms[inorms[v]] = normals[inorms[v]];
                        packed_tex[iverts[v]] = texcoords[itex[v]];
                }
        }

        mesh->normals(packed_norms);
        mesh->texcoords(packed_tex);
        mesh->triangles(triangles);
        mesh->update();
        return mesh;
}

bool
e8util::wavefront_obj::save_geometry(e8::if_geometry const* geo)
{
        std::ofstream file(m_location + "_" + std::to_string(geo->id()) + ".obj");

        if (!file.is_open()) {
                std::perror(("wavefront_obj::save_geometries to " + m_location).c_str());
                return false;
        }

        file << "# e8yescg wavefront_obj" << std::endl;

        // output vertices.
        std::vector<e8util::vec3> const& verts = geo->vertices();
        for (unsigned i = 0; i < verts.size(); i ++) {
                e8util::vec3 const& v = verts[i];
                file << "v " << v(0) << ' ' << v(1) << ' ' << v(2) << std::endl;
        }

        // output normals.
        std::vector<e8util::vec3> const& norms = geo->normals();
        for (unsigned i = 0; i < norms.size(); i ++) {
                e8util::vec3 const& n = norms[i];
                file << "vn " << n(0) << ' ' << n(1) << ' ' << n(2) << std::endl;
        }

        // output texture coordinate.
        std::vector<e8util::vec2> const& texcoords = geo->texcoords();
        for (unsigned i = 0; i < texcoords.size(); i ++) {
                e8util::vec2 const& t = texcoords[i];
                file << "vt " << t(0) << ' ' << t(1) << ' ' << t(2) << std::endl;
        }

        // output faces.
        std::vector<e8::triangle> const& faces = geo->triangles();
        for (unsigned i = 0; i < faces.size(); i ++) {
                e8::triangle const& f = faces[i] + 1;
                file << "f " << f(0) << ' ' << f(1) << ' ' << f(2) << std::endl;
        }

        return true;
}

std::vector<std::shared_ptr<e8::if_obj>>
e8util::wavefront_obj::load_roots()
{
        return std::vector<std::shared_ptr<e8::if_obj>> { load_geometry() };
}

void
e8util::wavefront_obj::save_roots(std::vector<std::shared_ptr<e8::if_obj>> const& roots)
{
        e8::visit_all_filtered(roots.begin(),
                               roots.end(),
                               [this] (e8::if_obj const* obj) {
                                this->save_geometry(static_cast<e8::if_geometry const*>(obj));
                               },
                               std::set<e8::obj_protocol> { e8::obj_protocol::obj_protocol_geometry });
}


class e8util::gltf_scene_internal
{
public:
        enum file_type
        {
                ascii,
                bin
        };

        gltf_scene_internal(std::string const& location);
        ~gltf_scene_internal();

        tinygltf::Model const&  get_model() const;

private:
        tinygltf::Model         m_model;
};

e8util::gltf_scene_internal::file_type
gtlf_file_type(std::string const& loc)
{
        auto dot_pos = loc.find_last_of(".");
        if (dot_pos == std::string::npos) {
                throw e8util::res_io_exception("Unknown file type of " + loc);
        }
        std::string ext = loc.substr(dot_pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == "glb") {
                return e8util::gltf_scene_internal::file_type::bin;
        } else if (ext == "gltf") {
                return e8util::gltf_scene_internal::file_type::ascii;
        } else {
                throw e8util::res_io_exception("Unknown file type of " + loc);
        }
}

e8util::gltf_scene_internal::gltf_scene_internal(std::string const& location)
{
        tinygltf::TinyGLTF loader;

        std::string err;
        std::string warn;
        bool res = false;
        switch (gtlf_file_type(location)) {
        case ascii: {
                res = loader.LoadASCIIFromFile(&m_model, &err, &warn, location.c_str());
                break;
        }
        case bin: {
                res = loader.LoadBinaryFromFile(&m_model, &err, &warn, location.c_str());
                break;
        }
        }

        if (!warn.empty()) {
                std::cerr << typeid(*this).name()
                          << " - Warning while parsing " << location << std::endl
                          << "\t" << warn << std::endl;
        }

        if (!res) {
                throw e8util::res_io_exception("Failed to load " + location + "\n"
                                               "\tError: " + err + "\n"
                                               "\tWarning: " + warn);
        }
}

e8util::gltf_scene_internal::~gltf_scene_internal()
{
}

tinygltf::Model const&
e8util::gltf_scene_internal::get_model() const
{
        return m_model;
}

e8util::gltf_scene::gltf_scene(std::string const& location):
        m_pimpl(std::make_unique<gltf_scene_internal>(location))
{
}

e8util::gltf_scene::~gltf_scene()
{
}


static unsigned
gltf_idx_get(unsigned char const* idx_data, unsigned stride, unsigned i)
{
        switch (stride) {
        case 1:
                return reinterpret_cast<uint8_t const*>(idx_data)[i];
        case 2:
                return reinterpret_cast<uint16_t const*>(idx_data)[i];
        case 4:
                return reinterpret_cast<uint32_t const*>(idx_data)[i];
        default:
                assert(stride == 1 || stride == 2 || stride == 4);
                return 0;
        }
}

static e8util::vec3
gltf_vec3_get(unsigned char const* idx_data, unsigned stride, unsigned i)
{
        switch (stride) {
        case 4*3:
                return e8util::vec3{reinterpret_cast<float const*>(idx_data)[3*i + 0],
                                    reinterpret_cast<float const*>(idx_data)[3*i + 1],
                                    reinterpret_cast<float const*>(idx_data)[3*i + 2]};
        case 8*3:
                return e8util::vec3{static_cast<float>(reinterpret_cast<double const*>(idx_data)[3*i + 0]),
                                    static_cast<float>(reinterpret_cast<double const*>(idx_data)[3*i + 1]),
                                    static_cast<float>(reinterpret_cast<double const*>(idx_data)[3*i + 2])};
        default:
                assert(stride == 4*3 || stride == 8*3);
                return e8util::vec3();
        }
}

static e8util::vec2
gltf_vec2_get(unsigned char const* idx_data, unsigned stride, unsigned i)
{
        switch (stride) {
        case 4*2:
                return e8util::vec2{reinterpret_cast<float const*>(idx_data)[2*i + 0],
                                    reinterpret_cast<float const*>(idx_data)[2*i + 1]};
        case 8*2:
                return e8util::vec2{static_cast<float>(reinterpret_cast<double const*>(idx_data)[2*i + 0]),
                                    static_cast<float>(reinterpret_cast<double const*>(idx_data)[2*i + 1])};
        default:
                assert(stride == 4*2 || stride == 8*2);
                return e8util::vec2();
        }
}


static std::shared_ptr<e8::if_geometry>
load_geometry(tinygltf::Mesh const& mesh, tinygltf::Model const& model)
{
        std::shared_ptr<e8::trimesh> geo = std::make_shared<e8::trimesh>();

        std::vector<e8::triangle> tris;
        std::vector<e8util::vec3> verts;
        std::vector<e8util::vec3> normals;
        std::vector<e8util::vec2> texcoords;

        for (size_t k = 0; k < mesh.primitives.size(); k ++) {
                tinygltf::Primitive const& prim = mesh.primitives[k];
                tinygltf::Accessor const& idx_accessor = model.accessors[static_cast<unsigned>(prim.indices)];
                tinygltf::BufferView const& idx_buf_view = model.bufferViews[static_cast<unsigned>(idx_accessor.bufferView)];
                tinygltf::Buffer const& idx_buf = model.buffers[static_cast<unsigned>(idx_buf_view.buffer)];
                unsigned char const* idx_data = idx_buf.data.data() + idx_buf_view.byteOffset + idx_accessor.byteOffset;
                unsigned stride = static_cast<unsigned>(idx_accessor.ByteStride(idx_buf_view));
                unsigned count = static_cast<unsigned>(idx_accessor.count);

                assert(count % 3 == 0);

                for (unsigned i = 0; i < count; i += 3) {
                        tris.push_back(e8::triangle({gltf_idx_get(idx_data, stride, i),
                                                     gltf_idx_get(idx_data, stride, i + 1),
                                                     gltf_idx_get(idx_data, stride, i + 2)}));
                }

                for (std::pair<std::string, int> const attri: prim.attributes) {
                        tinygltf::Accessor const& attri_acs = model.accessors[static_cast<unsigned>(attri.second)];
                        tinygltf::BufferView const& attri_buf_view = model.bufferViews[static_cast<unsigned>(attri_acs.bufferView)];
                        tinygltf::Buffer const& attri_buf = model.buffers[static_cast<unsigned>(attri_buf_view.buffer)];
                        unsigned char const* attri_data = attri_buf.data.data() +
                                        attri_buf_view.byteOffset +
                                        attri_acs.byteOffset;
                        unsigned stride = static_cast<unsigned>(attri_acs.ByteStride(attri_buf_view));
                        unsigned count = static_cast<unsigned>(attri_acs.count);

                        if (attri.first == "POSITION") {
                                for (unsigned i = 0; i < count; i ++) {
                                        verts.push_back(gltf_vec3_get(attri_data, stride, i));
                                }
                        } else if (attri.first == "NORMAL") {
                                for (unsigned i = 0; i < count; i ++) {
                                        normals.push_back(gltf_vec3_get(attri_data, stride, i));
                                }
                        } else if (attri.first == "TEXCOORD_0") {
                                for (unsigned i = 0; i < count; i ++) {
                                        texcoords.push_back(gltf_vec2_get(attri_data, stride, i));
                                }
                        }
                }
        }

        geo->triangles(tris);
        geo->vertices(verts);
        geo->normals(normals);
        geo->texcoords(texcoords);
        geo->update();

        return geo;
}


std::vector<std::shared_ptr<e8::if_material>>
e8util::gltf_scene::load_materials() const
{
        std::vector<std::shared_ptr<e8::if_material>> mats;

        // TODO: cannot implement material loading with current e8::materials.
        std::shared_ptr<e8::if_material> mat =
                        std::make_shared<e8::oren_nayar>(e8util::vec3{0.8f, 0.8f, 0.8f}, 0.4f);

        tinygltf::Model const& model = m_pimpl->get_model();
        for (size_t i = 0; i < model.materials.size(); i ++) {
                tinygltf::Material const& gltf_mat = model.materials[i];

                for (std::pair<std::string, tinygltf::Parameter> entry: gltf_mat.values) {
                }
        }

        std::vector<std::shared_ptr<e8::if_material>> geo_mats;
        for (size_t i = 0; i < model.meshes.size(); i ++) {
                geo_mats.push_back(mat);
        }
        return geo_mats;
}

std::vector<std::shared_ptr<e8::if_light>>
e8util::gltf_scene::load_lights() const
{
        // Lights are not handled by glTF 2.0 as of now.
        return std::vector<std::shared_ptr<e8::if_light>>();
}

std::vector<std::shared_ptr<e8::if_light>>
e8util::gltf_scene::load_virtual_lights() const
{
        // Lights are not handled by glTF 2.0 as of now.
        // return as least one default light.
        return std::vector<std::shared_ptr<e8::if_light>>{std::make_shared<e8::sky_light>(e8util::vec3{.529f, .808f, .922f})};
}

static std::shared_ptr<e8::if_camera>
load_camera(tinygltf::Camera const& gltfcam)
{
        if (gltfcam.type == "perspective") {
                // gltfcam.perspective.
                std::shared_ptr<e8::pinhole_camera> cam =
                                std::make_shared<e8::pinhole_camera>(gltfcam.name,
                                                                     static_cast<float>(2*gltfcam.perspective.znear*std::tan(gltfcam.perspective.yfov)),
                                                                     static_cast<float>(gltfcam.perspective.znear/std::tan(gltfcam.perspective.yfov)),
                                                                     static_cast<float>(gltfcam.perspective.aspectRatio));
                return cam;
        } else if (gltfcam.type == "orthographic") {
                // TODO: need to support orthographic camera.
                return nullptr;
        } else {
                return nullptr;
        }
}

static std::shared_ptr<e8::if_obj>
load_node(tinygltf::Node const& node, tinygltf::Model const& model)
{
        // Create current node with metadata loaded in.
        std::shared_ptr<e8::if_obj> node_obj = nullptr;
        if (node.mesh >= 0) {
                node_obj = load_geometry(model.meshes[static_cast<unsigned>(node.mesh)], model);
        } else if (node.camera >= 0) {
                node_obj = load_camera(model.cameras[static_cast<unsigned>(node.camera)]);
        }

        if (node_obj == nullptr) {
                node_obj = std::make_shared<e8::null_obj>();
        }

        // Load in transformation.
        e8util::mat44 transform;
        if (!node.matrix.empty()) {
                assert(node.matrix.size() == 16);
                for (unsigned i = 0; i < 4; i ++) {
                        for (unsigned j = 0; j < 4; j ++) {
                                transform(j, i) = static_cast<float>(node.matrix[j + i*2]);
                        }
                }
        } else {
                transform = e8util::mat44_scale(1.0f);
        }
        node_obj->init_blueprint(std::vector<e8::transform_stage_name_t>{"GLTF_DEFAULT"});
        node_obj->update_stage(std::make_pair("GLTF_DEFAULT", transform));

        // Load childrens.
        for (int node_child_i: node.children) {
                node_obj->add_child(load_node(model.nodes[static_cast<unsigned>(node_child_i)], model));
        }
        return node_obj;
}

std::vector<std::shared_ptr<e8::if_obj>>
e8util::gltf_scene::load_roots()
{
        std::vector<std::shared_ptr<e8::if_obj>> roots;
        tinygltf::Model model = m_pimpl->get_model();
        std::vector<tinygltf::Node> nodes = model.nodes;
        if (!model.scenes.empty()) {
                // Can only handle one scene.
                for (int node_i: model.scenes[0].nodes) {
                        roots.push_back(load_node(model.nodes[static_cast<unsigned>(node_i)], model));
                }
        }

        // assign default material if geometry doesn't have material assigned.
        e8::visit_all_filtered(roots.begin(),
                               roots.end(),
                               [] (e8::if_obj* obj) {
                                if (obj->get_children(e8::obj_protocol::obj_protocol_material).empty()) {
                                        obj->add_child(std::make_shared<e8::mat_fail_safe>("gltf_fail_safe"));
                                }
                               },
                               std::set<e8::obj_protocol> { e8::obj_protocol::obj_protocol_geometry });
        return roots;
}
