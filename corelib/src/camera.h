#ifndef CAMERA_H
#define CAMERA_H


#include <vector>
#include <string>
#include "tensor.h"


namespace e8
{

class if_camera
{
public:
        if_camera(std::string const& name);
        virtual ~if_camera();

        virtual e8util::ray             sample(e8util::rng& rng,
                                               unsigned x,
                                               unsigned y,
                                               unsigned w,
                                               unsigned h,
                                               float& pdf) const = 0;
        virtual e8util::mat44           projection() const = 0;

        std::string     name() const;
private:
        std::string     m_name;
};

class pinhole_camera: public if_camera
{
public:
        pinhole_camera(std::string const& name,
                       e8util::vec3 const& t,
                       e8util::mat44 const& r,
                       float sensor_size,
                       float f,
                       float aspect);
        pinhole_camera(e8util::vec3 const& t,
                       e8util::mat44 const& r,
                       float sensor_size,
                       float f,
                       float aspect);
        ~pinhole_camera() override;

        e8util::ray                     sample(e8util::rng& rng,
                                               unsigned x,
                                               unsigned y,
                                               unsigned w,
                                               unsigned h,
                                               float& pdf) const override;
        e8util::mat44                   projection() const override;
private:
        float           m_znear;
        float           m_sensor_size;
        float           m_focal_len;
        float           m_aspect;
        e8util::vec3    m_t;
        e8util::mat44   m_r;
        e8util::mat44   m_proj;
        e8util::mat44   m_forward;
};

}

#endif // CAMERA_H
