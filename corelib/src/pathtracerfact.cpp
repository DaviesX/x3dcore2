#include "pathtracerfact.h"
#include "pathtracer.h"
#include <cassert>

e8::pathtracer_factory::pathtracer_factory(pt_type type, options opts)
    : m_type(type), m_opts(opts) {}

e8::pathtracer_factory::~pathtracer_factory() {}

e8::if_pathtracer *e8::pathtracer_factory::create() {
    switch (m_type) {
    case normal:
        return new e8::normal_pathtracer();
    case position:
        return new e8::position_pathtracer();
    case direct:
        return new e8::direct_pathtracer();
    case unidirect:
        return new e8::unidirect_lt1_path_tracer();
    case bidirect_lt2:
        return new e8::bidirect_lt2_pathtracer();
    case bidirect_mis:
        return new e8::bidirect_mis_pathtracer();
    }
    assert(false);
    return nullptr;
}
