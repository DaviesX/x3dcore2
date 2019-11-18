#ifndef MATERIAL_H
#define MATERIAL_H

#include "obj.h"
#include "tensor.h"
#include <complex>
#include <memory>
#include <string>
#include <vector>

namespace e8 {

/**
 * @brief The texture_map class Generic texture map. It represents a 2D texture containing arbitrary
 * content in each texel. This 2D texture is continously addressable.
 */
template <typename T> class texture_map {
  public:
    /**
     * @brief texture_map
     * @param width Width of the texture in terms of texel.
     * @param height Height of the texture in terms of texel.
     * @param data Data content of the texture.
     */
    texture_map(unsigned width, unsigned height, std::vector<T> const &data);

    /**
     * @brief map Maps a normalized 2D coordinate to content on the texture (map: [0,1)x[0,1)->T).
     * @param uv A 2D coordinate where each component is normalized to [0, 1).
     * @return Mapped content.
     */
    T map(e8util::vec2 const &uv) const;

  private:
    unsigned m_width;
    unsigned m_height;
    std::vector<T> m_data;
};

template <typename T>
texture_map<T>::texture_map(unsigned width, unsigned height, std::vector<T> const &data)
    : m_width(width), m_height(height), m_data(data) {}

template <typename T> T texture_map<T>::map(e8util::vec2 const &uv) const {
    unsigned iu = uv(0) * m_width;
    unsigned iv = uv(1) * m_height;
    return m_data[iu + iv * m_width];
}

/**
 * @brief The if_material class Material interface specialized for path-tracing.
 */
class if_material : public if_copyable_obj<if_material> {
  public:
    /**
     * @brief if_material
     * @param name
     */
    if_material(std::string const &name);
    virtual ~if_material() override;

    std::string name() const;
    obj_protocol protocol() const override;

    virtual std::unique_ptr<if_material> copy() const override = 0;

    /**
     * @brief eval Compute the amount of reflected radiance.
     * @param uv Coordinate to map a normalized 2D coordinate to content on the texture (map:
     * [0,1)x[0,1)->T).
     * @param n Normal vector at the surface.
     * @param o Reflected path.
     * @param i Incident path.
     * @return Reflected radiance.
     */
    virtual e8util::vec3 eval(e8util::vec2 const &uv, e8util::vec3 const &n, e8util::vec3 const &o,
                              e8util::vec3 const &i) const = 0;

    /**
     * @brief sample Compute a incident path sample given the normal and reflected path.
     * @param rng Random number generator.
     * @param cond_density The conditional probability density of the sample.
     * @param uv Coordinate to map a normalized 2D coordinate to content on the texture (map:
     * [0,1)x[0,1)->T).
     * @param n Normal vector at the surface.
     * @param o Reflected path.
     * @return Incident path sample.
     */
    virtual e8util::vec3 sample(e8util::rng *rng, float *cond_density, e8util::vec2 const &uv,
                                e8util::vec3 const &n, e8util::vec3 const &o) const = 0;

  protected:
    if_material(obj_id_t id, std::string const &name);
    std::string m_name;
};

/**
 * @brief The mat_fail_safe class A fail-safe default material. Some renderer requires every object
 * must have material attached. This can allow the renderer to keep running even with faulty scene
 * configurations.
 */
class mat_fail_safe : public if_material {
  public:
    mat_fail_safe(std::string const &name);
    mat_fail_safe(mat_fail_safe const &other);

    std::unique_ptr<if_material> copy() const override;

    e8util::vec3 eval(e8util::vec2 const &uv, e8util::vec3 const &n, e8util::vec3 const &o,
                      e8util::vec3 const &i) const override;
    e8util::vec3 sample(e8util::rng *rng, float *cond_density, e8util::vec2 const &uv,
                        e8util::vec3 const &n, e8util::vec3 const &o) const override;

  private:
    e8util::vec3 m_albedo;
    unsigned m_padding;
};

/**
 * @brief The mat_mixture class Models the weighted mixture of two different materials.
 */
class mat_mixture : public if_material {
  public:
    mat_mixture(std::string const &name, std::unique_ptr<if_material> mat_0,
                std::unique_ptr<if_material> mat_1, float ratio);
    mat_mixture(mat_mixture const &other);

    std::unique_ptr<if_material> copy() const override;

    e8util::vec3 eval(e8util::vec2 const &uv, e8util::vec3 const &n, e8util::vec3 const &o,
                      e8util::vec3 const &i) const override;
    e8util::vec3 sample(e8util::rng *rng, float *cond_density, e8util::vec2 const &uv,
                        e8util::vec3 const &n, e8util::vec3 const &o) const override;

  private:
    std::unique_ptr<if_material> m_mat_0;
    std::unique_ptr<if_material> m_mat_1;
    float m_ratio;
    unsigned reserved0;
};

/**
 * @brief The oren_nayar class A diffuse microfacet reflectance model (See
 * https://en.wikipedia.org/wiki/Oren%E2%80%93Nayar_reflectance_model).
 */
class oren_nayar : public if_material {
  public:
    oren_nayar(std::string const &name, e8util::vec3 const &albedo, float roughness,
               std::shared_ptr<texture_map<e8util::vec3>> const &albedo_map = nullptr,
               std::shared_ptr<texture_map<float>> const &roughness_map = nullptr);
    oren_nayar(oren_nayar const &other);

    std::unique_ptr<if_material> copy() const override;

    e8util::vec3 eval(e8util::vec2 const &uv, e8util::vec3 const &n, e8util::vec3 const &o,
                      e8util::vec3 const &i) const override;
    e8util::vec3 sample(e8util::rng *rng, float *cond_density, e8util::vec2 const &uv,
                        e8util::vec3 const &n, e8util::vec3 const &o) const override;

  private:
    e8util::vec3 albedo(e8util::vec2 const &uv) const;

    std::shared_ptr<texture_map<e8util::vec3>> m_albedo_map;
    std::shared_ptr<texture_map<float>> m_roughness_map;

    e8util::vec3 m_albedo;
    float m_sigma;
    float m_a;
    float m_b;
};

/**
 * @brief The cook_torr class A specular microfacet reflectance model (See
 * https://en.wikipedia.org/wiki/Specular_highlight#Cook%E2%80%93Torrance_model).
 */
class cook_torr : public if_material {
  public:
    cook_torr(std::string const &name, e8util::vec3 const &albedo, float roughness,
              std::complex<float> const &ior,
              std::shared_ptr<texture_map<e8util::vec3>> const &albedo_map = nullptr,
              std::shared_ptr<texture_map<float>> const &roughness_map = nullptr);
    cook_torr(cook_torr const &other);

    std::unique_ptr<if_material> copy() const override;

    e8util::vec3 eval(e8util::vec2 const &uv, e8util::vec3 const &n, e8util::vec3 const &o,
                      e8util::vec3 const &i) const override;
    e8util::vec3 sample(e8util::rng *rng, float *cond_density, e8util::vec2 const &uv,
                        e8util::vec3 const &n, e8util::vec3 const &o) const override;

  private:
    e8util::vec3 albedo(e8util::vec2 const &uv) const;
    float alpha2(e8util::vec2 const &uv) const;

    std::shared_ptr<texture_map<e8util::vec3>> m_albedo_map;
    std::shared_ptr<texture_map<float>> m_roughness_map;

    e8util::vec3 m_albedo;
    std::complex<float> m_ior;
    float m_alpha2;
};

} // namespace e8

#endif // IF_MATERIAL_H
