#include "pathtracerfact.h"
#include "pathtracer.h"
#include <cassert>

e8::pathtracer_factory::pathtracer_factory(pt_type type, options opts)
    : m_type(type), m_opts(opts) {}

e8::pathtracer_factory::~pathtracer_factory() {}

e8::if_path_tracer *e8::pathtracer_factory::create() {
    switch (m_type) {
    case normal:
        return new e8::normal_tracer();
    case position:
        return new e8::position_tracer();
    case direct:
        return new e8::direct_path_tracer();
    case unidirect_lt1:
        return new e8::unidirect_lt1_path_tracer();
    case bidirect_lt2:
        return new e8::bidirect_lt2_path_tracer();
    case bidirect_mis:
        return new e8::bidirect_mis_path_tracer();
    }
    assert(false);
    return nullptr;
}
