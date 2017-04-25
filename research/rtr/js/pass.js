class pass_rend_info {
    constructor(mat, frame_id) {
        this.mat = mat;
        this.frame_id = frame_id;
    }
    appeared(frame_id) {
        this.frame_id = frame_id;
    }
}
class pass_light_info {
}
class pass_cache {
    constructor(backend, need_lights) {
        this.perm_obj = new Set();
        this.mats = new Map();
        this.rend = new Map();
        this.lights = new Map();
        this.cam = null;
        this.backend = backend;
        this.need_lights = need_lights;
    }
    update(scene, cam, frame_id) {
        var this_ = this;
        this.cam = cam;
        var f = cam.get_frustum();
        var rend = scene.get_relevant_renderables(f);
        var lights = this.need_lights ? scene.get_relevant_lights(f) : null;
        rend.forEach(function (mat, r, m) {
            var info = this_.rend.get(r);
            if (info == null) {
                this_.rend.set(r, new pass_rend_info(mat, frame_id));
            }
            else {
                info.appeared(frame_id);
            }
            if (info.mat !== mat) {
                var prog = this_.mats.get(info.mat);
                if (prog !== null) {
                    this_.backend.program_delete(prog);
                    this_.mats.delete(info.mat);
                }
                info.mat = mat;
            }
        });
        if (this.need_lights) {
            lights.forEach(function (light, i, a) {
                var info = this_.lights.get(light);
                if (info === null)
                    this_.lights.set(light, new pass_light_info());
            });
        }
    }
    upload_material(backend, prog, mat) {
        var old_prog = this.mats.get(mat);
        if (old_prog !== null && old_prog !== prog)
            backend.program_delete(prog);
        this.mats.set(mat, prog);
        mat.upload(backend, prog);
    }
    download_material(backend, mat) {
        return this.mats.get(mat);
    }
    upload_renderable(backend, prog, rend, modelview, proj, mat) {
        var req_attri = mat.get_required_attributes();
        if (rend.is_permanent()) {
            if (!this.perm_obj.has(rend)) {
                this.perm_obj.add(rend);
                var all_attri = rend.available_attributes();
                for (var i = 0; i < all_attri.length; i++)
                    rend.upload(backend, all_attri[i]);
            }
        }
        else {
            for (var i = 0; i < req_attri.length; i++)
                rend.upload(backend, req_attri[i]);
        }
        for (var i = 0; i < req_attri.length; i++)
            rend.upload_transform_call(backend, prog, req_attri[i], modelview, proj);
    }
    upload_light(backend, prog, light) {
        light.upload(backend, prog, 0);
    }
    get_renderables() {
        return this.rend;
    }
    get_lights() {
        return this.lights;
    }
    get_camera() {
        return this.cam;
    }
    clear(backend) {
        this.perm_obj.forEach(function (i, rend, m) {
            rend.unload(backend);
        });
        this.perm_obj.clear();
        this.rend = null;
        this.lights = null;
        this.cam = null;
    }
}
function pass_create_program_from_material(backend, rend, mat, light) {
    var prog = backend.program_create();
    var attris = mat.get_required_attributes();
    var calls = new shader_call_sequence();
    for (var i = 0; i < attris.length; i++)
        calls.add_call(rend.get_transform_call(attris[i]));
    var code = calls.gen_glsl_main(shader_get_builtin_library());
    var vert_shader = backend.shader_vertex_create(code);
    var rad_call = mat.get_radiance_call(light.get_irradiance_call(), light.get_incident_call());
    var calls = new shader_call_sequence();
    calls.add_call(rad_call);
    var code = calls.gen_glsl_main(shader_get_builtin_library());
    var frag_shader = backend.shader_fragment_create(code);
    backend.program_attach_shader(prog, vert_shader);
    backend.program_attach_shader(prog, frag_shader);
    backend.program_link(prog);
    return prog;
}
var pass_buffer_type;
(function (pass_buffer_type) {
    pass_buffer_type[pass_buffer_type["radiance"] = 0] = "radiance";
})(pass_buffer_type || (pass_buffer_type = {}));
class pass_radiance {
    constructor() {
        this.cache = null;
    }
    gen_cache(backend, scene, camera, frame_id) {
        if (this.cache === null) {
            this.cache = new pass_cache(backend, true);
        }
        this.cache.update(scene, camera, frame_id);
        return this.cache;
    }
    run(backend, bufs, cache) {
        var this_ = this;
        var lights = cache.get_lights();
        var rends = cache.get_renderables();
        if (lights.size !== 0) {
            var first_light;
            lights.forEach(function (info, light, m) {
                first_light = light;
                return;
            });
            var cam = cache.get_camera();
            rends.forEach(function (info, rend, m) {
                var light = first_light;
                var prog = cache.download_material(backend, info.mat);
                if (prog === null)
                    prog = pass_create_program_from_material(backend, rend, info.mat, first_light);
                cache.upload_light(backend, prog, first_light);
                cache.upload_material(backend, prog, info.mat);
                cache.upload_renderable(backend, prog, rend, cam.get_modelview_transform(), cam.get_frustum().projective_transform(), info.mat);
                var buf = rend.get_buffer(attri_type.index);
                for (var i = 0; i < buf.length; i++)
                    backend.draw_indexed_triangles(backend.frame_buf_get_default(), prog, buf[i].get_buf(), 0, buf[i].get_len());
            });
        }
        return [pass_buffer_type.radiance, backend.frame_buf_get_default()];
    }
    release(backend) {
        this.cache.clear(backend);
    }
}
//# sourceMappingURL=pass.js.map