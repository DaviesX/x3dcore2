#ifndef LIGHTSOURCES_H
#define LIGHTSOURCES_H

#include <map>
#include <memory>
#include "obj.h"
#include "light.h"

namespace e8 {

class if_light_sources: public if_obj_manager
{
public:
        if_light_sources();
        virtual ~if_light_sources() override;

        void                            load(if_obj const* obj, e8util::mat44 const& trans) override;
        void                            unload(if_obj const* obj) override;
        obj_protocol                    support() const override;
        virtual void                    commit() override = 0;
        virtual if_light const*         sample_light(e8util::rng& rng, float& pdf) const = 0;
protected:
        std::map<obj_id_t, std::unique_ptr<if_light>>   m_lights;

};

class basic_light_sources: public if_light_sources
{
public:
        basic_light_sources();
        ~basic_light_sources() override;
        void                    commit() override;
        if_light const*         sample_light(e8util::rng& rng, float& pdf) const override;
private:
        struct light_cdf
        {
                light_cdf(if_light const* light,
                          float cum_power);
                if_light const*                 light;
                float                           cum_power;
        };

        std::vector<light_cdf>  m_light_cdf;
        float                   m_total_power;
};

} // namespace e8

#endif // LIGHTSOURCES_H
