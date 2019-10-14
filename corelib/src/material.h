#ifndef MATERIAL_H
#define MATERIAL_H

#include "obj.h"
#include "tensor.h"

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
 * @brief The if_material class
 */
class if_material : public if_copyable_obj<if_material> {
  public:
    if_material(std::string const &name);
    virtual ~if_material() override;

    std::string name() const;
    obj_protocol protocol() const override;

    virtual std::unique_ptr<if_material> copy() const override = 0;
    virtual e8util::vec3 eval(e8util::vec3 const &n, e8util::vec3 const &o,
                              e8util::vec3 const &i) const = 0;
    virtual e8util::vec3 sample(e8util::rng &rng, e8util::vec3 const &n, e8util::vec3 const &o,
                                float &pdf) const = 0;

  protected:
    if_material(obj_id_t id, std::string const &name);
    std::string m_name;
};

class mat_fail_safe : public if_material {
  public:
    mat_fail_safe(std::string const &name);
    mat_fail_safe(mat_fail_safe const &other);

    std::unique_ptr<if_material> copy() const override;
    e8util::vec3 eval(e8util::vec3 const &n, e8util::vec3 const &o,
                      e8util::vec3 const &i) const override;
    e8util::vec3 sample(e8util::rng &rng, e8util::vec3 const &n, e8util::vec3 const &o,
                        float &pdf) const override;

  private:
    e8util::vec3 m_albedo;
    unsigned m_padding;
};

class oren_nayar : public if_material {
  public:
    oren_nayar(e8util::vec3 const &albedo, float roughness);
    oren_nayar(std::string const &name, e8util::vec3 const &albedo, float roughness);
    oren_nayar(oren_nayar const &other);

    std::unique_ptr<if_material> copy() const override;
    e8util::vec3 eval(e8util::vec3 const &n, e8util::vec3 const &o,
                      e8util::vec3 const &i) const override;
    e8util::vec3 sample(e8util::rng &rng, e8util::vec3 const &n, e8util::vec3 const &o,
                        float &pdf) const override;

  private:
    e8util::vec3 m_albedo;
    float m_sigma;
    float A;
    float B;
};

class cook_torr : public if_material {
  public:
    cook_torr(std::string const &name, e8util::vec3 const &albedo, float beta, float ior);
    cook_torr(e8util::vec3 const &albedo, float beta, float ior);
    cook_torr(cook_torr const &other);

    std::unique_ptr<if_material> copy() const override;
    e8util::vec3 eval(e8util::vec3 const &n, e8util::vec3 const &o,
                      e8util::vec3 const &i) const override;
    e8util::vec3 sample(e8util::rng &rng, e8util::vec3 const &n, e8util::vec3 const &o,
                        float &pdf) const override;

  private:
    float fresnel(e8util::vec3 const &i, e8util::vec3 const &h) const;
    float ggx_distri(e8util::vec3 const &n, e8util::vec3 const &h) const;
    float ggx_shadow1(e8util::vec3 const &v, e8util::vec3 const &h) const;
    float ggx_shadow(e8util::vec3 const &i, e8util::vec3 const &o, e8util::vec3 const &h) const;

    e8util::vec3 m_albedo;
    float m_beta2;
    float m_ior2;
    uint32_t m_padding;
};

} // namespace e8

#endif // IF_MATERIAL_H
