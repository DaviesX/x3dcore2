#ifndef PATHTRACERFACT_H
#define PATHTRACERFACT_H

#include "pathtracer.h"

namespace e8
{

class pathtracer_factory
{
public:
        enum pt_type {
                normal,
                position,
                direct,
                unidirect,
                bidirect_lt2,
                bidirect_mis
        };

        struct options {
                int max_pathlen;
        };

        pathtracer_factory(pt_type type, options opts);
        ~pathtracer_factory();

        if_pathtracer*  create();
private:
        pt_type         m_type;
        options         m_opts;
};

}

#endif // PATHTRACERFACT_H
