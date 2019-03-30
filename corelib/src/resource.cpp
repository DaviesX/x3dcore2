#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../thirdparty/tinygltf/tiny_gltf.h"
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

std::vector<e8::if_geometry*>
e8util::if_resource::load_geometries() const
{
        throw std::string("resource doesn't support load_geometries");
}

std::vector<e8::if_material*>
e8util::if_resource::load_materials() const
{
        throw std::string("resource doesn't support load_materials");
}

std::vector<e8::if_light*>
e8util::if_resource::load_lights() const
{
        throw std::string("resource doesn't support load_lights");
}

std::vector<e8::if_light*>
e8util::if_resource::load_virtual_lights() const
{
        throw std::string("resource doesn't support load_virtual_lights");
}

e8::if_camera*
e8util::if_resource::load_camera() const
{
        throw std::string("resource doesn't support load_camera");
}

bool
e8util::if_resource::save_geometries(std::vector<e8::if_geometry*> const&)
{
        throw std::string("resource doesn't support save_geometries");
}



e8util::cornell_scene::cornell_scene()
{
}

std::vector<e8::if_geometry*>
e8util::cornell_scene::load_geometries() const
{
        std::vector<e8::if_geometry*> geometries(8);
        geometries[0] = wavefront_obj("res/cornellbox/left_wall.obj").load_geometries()[0];
        geometries[1] = wavefront_obj("res/cornellbox/right_wall.obj").load_geometries()[0];
        geometries[2] = wavefront_obj("res/cornellbox/ceiling.obj").load_geometries()[0];
        geometries[3] = wavefront_obj("res/cornellbox/floor.obj").load_geometries()[0];
        geometries[4] = wavefront_obj("res/cornellbox/back_wall.obj").load_geometries()[0];
        geometries[5] = wavefront_obj("res/cornellbox/left_sphere.obj").load_geometries()[0];
        geometries[6] = wavefront_obj("res/cornellbox/right_sphere.obj").load_geometries()[0];
        geometries[7] = wavefront_obj("res/cornellbox/light.obj").load_geometries()[0];
        return geometries;
}

std::vector<e8::if_material*>
e8util::cornell_scene::load_materials() const
{
        e8::if_material* white = new e8::oren_nayar(e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
        e8::if_material* red = new e8::oren_nayar(e8util::vec3({0.630f, 0.065f, 0.050f}), 0.078f);
        e8::if_material* green = new e8::oren_nayar(e8util::vec3({0.140f, 0.450f, 0.091f}), 0.078f);
        e8::if_material* glossy = new e8::cook_torr(e8util::vec3({0.787f, 0.787f, 0.787f}), 0.25f, 2.93f);
        e8::if_material* light = new e8::oren_nayar(e8util::vec3({0, 0, 0}), 0.078f);
        return std::vector<e8::if_material*>({red, green, white, white, white, glossy, white, light});
}

std::vector<e8::if_light*>
e8util::cornell_scene::load_lights() const
{
        return std::vector<e8::if_light*>({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                  new e8::area_light(wavefront_obj("res/cornellbox/light.obj").load_geometries()[0],
                                                              e8util::vec3({0.911f, 0.660f, 0.345f})*15.0f)});
}

std::vector<e8::if_light*>
e8util::cornell_scene::load_virtual_lights() const
{
        return std::vector<e8::if_light*>{new e8::sky_light(e8util::vec3{.529f, .808f, .922f})};
}

e8::if_camera*
e8util::cornell_scene::load_camera() const
{
        /*return new e8::pinhole_camera(e8util::vec3({0.0f, -3.4f, 0.795f}),
                                      e8util::mat44_rotate(M_PI/2.0f, e8util::vec3({1, 0, 0})),
                                      0.032f, 0.035f, 4.0f/3.0f);*/
        return new e8::pinhole_camera(e8util::vec3({0.0f, 0.795f, 3.4f}), 1.0f, 0.032f, 0.035f, 4.0f/3.0f);
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

std::vector<e8::if_geometry*>
e8util::wavefront_obj::load_geometries() const
{
        std::ifstream file(m_location);
        if (!file.is_open()) {
                std::perror(("wavefront_obj::load_geometries to " + m_location).c_str());
                return std::vector<e8::if_geometry*>();
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
        e8::trimesh* mesh = new e8::trimesh();

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
        return std::vector<e8::if_geometry*>({mesh});
}

bool
e8util::wavefront_obj::save_geometries(std::vector<e8::if_geometry*> const& geometries)
{
        if (geometries.size() != 1)
                throw std::string("Can save only 1 geometry at a time.");

        std::ofstream file(m_location);

        if (!file.is_open()) {
                std::perror(("wavefront_obj::save_geometries to " + m_location).c_str());
                return false;
        }

        file << "# e8yescg wavefront_obj" << std::endl;

        e8::if_geometry* geo = geometries[0];

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
        bool res;
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
        m_pimpl(new gltf_scene_internal(location))
{
}

e8util::gltf_scene::~gltf_scene()
{
        delete m_pimpl;
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


std::vector<e8::if_geometry*>
e8util::gltf_scene::load_geometries() const
{
        std::vector<e8::if_geometry*> geos;
        tinygltf::Model const& model = m_pimpl->get_model();

        for (size_t i = 0; i < model.meshes.size(); i ++) {
                tinygltf::Mesh const& mesh = model.meshes[i];
                e8::trimesh* geo = new e8::trimesh();

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

                geos.push_back(geo);
        }
        return geos;
}

std::vector<e8::if_material*>
e8util::gltf_scene::load_materials() const
{
        std::vector<e8::if_material*> mats;

        // TODO: cannot implement material loading with current e8::materials.
        e8::if_material* mat = new e8::oren_nayar(e8util::vec3{0.8f, 0.8f, 0.8f}, 0.4f);

        tinygltf::Model const& model = m_pimpl->get_model();
        for (size_t i = 0; i < model.materials.size(); i ++) {
                tinygltf::Material const& gltf_mat = model.materials[i];

                for (std::pair<std::string, tinygltf::Parameter> entry: gltf_mat.values) {
                }
        }

        std::vector<e8::if_material*> geo_mats;
        for (size_t i = 0; i < model.meshes.size(); i ++) {
                geo_mats.push_back(mat);
        }
        return geo_mats;
}

std::vector<e8::if_light*>
e8util::gltf_scene::load_lights() const
{
        // Lights are not handled by glTF 2.0 as of now.
        return std::vector<e8::if_light*>();
}

std::vector<e8::if_light*>
e8util::gltf_scene::load_virtual_lights() const
{
        // Lights are not handled by glTF 2.0 as of now.
        // return as least one default light.
        return std::vector<e8::if_light*>{new e8::sky_light(e8util::vec3{.529f, .808f, .922f})};
}

e8::if_camera*
e8util::gltf_scene::load_camera() const
{
        // Always capture the first camera, if any.
        tinygltf::Model const& model = m_pimpl->get_model();
        if (!model.cameras.empty()) {
                tinygltf::Camera const& gltfcam = model.cameras[0];
                if (gltfcam.type == "perspective") {
                        // gltfcam.perspective.
                         e8::pinhole_camera* cam = new e8::pinhole_camera(gltfcam.name,
                                                                          e8util::vec3(),
                                                                          e8util::mat44_rotate(0, e8util::vec3({0, 0, 1})),
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
        } else {
                return nullptr;
        }
}
