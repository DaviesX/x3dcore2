#ifndef CINEMATICS_H
#define CINEMATICS_H

#include <string>
#include <map>
#include "camera.h"
#include "obj.h"

namespace e8
{

class if_cinematics: public if_obj_manager
{
public:
        if_cinematics(std::string const& name);
        virtual ~if_cinematics() override;

        virtual if_camera*      main_cam() const = 0;

        void                    load(if_obj* obj, e8util::mat44 const& trans) override;
        void                    unload(if_obj* obj) override;
        const std::type_info&   support() const override;
        std::string             name() const;

protected:
        std::string                     m_name;
        std::map<obj_id_t, if_camera*>  m_cams;
};


class stationary_cam_controller: public if_cinematics
{
public:
        stationary_cam_controller(std::string const& name);
        ~stationary_cam_controller() override;

        if_camera*              main_cam() const override;
};


}


#endif // CINEMATICS_H
