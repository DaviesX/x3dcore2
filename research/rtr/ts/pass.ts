
/// <reference path="gl.ts" />
/// <reference path="tensor.ts" />
/// <reference path="light.ts" />
/// <reference path="mesh.ts" />
/// <reference path="material.ts" />
/// <reference path="camera.ts" />
/// <reference path="scene.ts" />

class pass_rend_info
{
        public mat: if_material;
        public frame_id: number;

        constructor(mat: if_material, frame_id: number)
        {
                this.mat = mat;
                this.frame_id = frame_id;
        }

        public appeared(frame_id: number): void
        {
                this.frame_id = frame_id;
        }
}

class pass_light_info
{
        public static_sm: buffer_location;
}

class pass_cache
{
        private need_lights: boolean;

        private backend: if_raster_backend;

        private perm_obj = new Set<if_renderable>();
        private mats = new Map<if_material, program_location>();

        private rends = new Map<if_renderable, pass_rend_info>();
        private batched_rends = new Map<if_material, Array<if_renderable>>();
        private lights = new Map<if_light, pass_light_info>();
        private cam: camera = null;

        constructor(backend: if_raster_backend, need_lights: boolean)
        {
                this.backend = backend;
                this.need_lights = need_lights;
        }

        public update(scene: scene, cam: camera, frame_id: number): void
        {
                var this_: pass_cache = this;

                this.cam = cam;
                var f: frustum = cam.get_frustum();

                var rend: Map<if_renderable, if_material> = scene.get_relevant_renderables(f);
                var lights: Array<if_light> = this.need_lights ? scene.get_relevant_lights(f) : null;

                // Update renderables.
                this.batched_rends.clear();
                rend.forEach(function (mat: if_material, r: if_renderable, m)
                {
                        var info: pass_rend_info = this_.rends.get(r);
                        // Update renderable info.
                        if (info == null)
                                this_.rends.set(r, new pass_rend_info(mat, frame_id));
                        else
                                info.appeared(frame_id);

                        // Update material.
                        if (info.mat !== mat) {
                                // Unload material.
                                var prog: program_location = this_.mats.get(info.mat);
                                if (prog !== null) {
                                        this_.backend.program_delete(prog);
                                        this_.mats.delete(info.mat);
                                }
                                // Update with new material.
                                info.mat = mat;
                        }

                        // update the actual batches of renderables that need to be rendered.
                        var batched_rends: Array<if_renderable> = this_.batched_rends.get(mat);
                        if (batched_rends === null)
                                this_.batched_rends.set(mat, [r]);
                        else
                                batched_rends.push(r);
                });

                // Update lights.
                if (this.need_lights) {
                        lights.forEach(function (light: if_light, i, a)
                        {
                                var info = this_.lights.get(light);
                                if (info === null)
                                        this_.lights.set(light, new pass_light_info());
                        });
                }
        }

        public upload_material(backend: if_raster_backend, prog: program_location, mat: if_material): void
        {
                var old_prog: program_location = this.mats.get(mat);
                if (old_prog !== null && old_prog !== prog)
                        backend.program_delete(prog);

                this.mats.set(mat, prog);
                mat.upload(backend, prog);
        }

        public download_material(backend: if_raster_backend, mat: if_material): program_location
        {
                return this.mats.get(mat);
        }

        public upload_renderable(backend: if_raster_backend, prog: program_location,
                rend: if_renderable, modelview: mat4, mat: if_material): void
        {
                var req_attri: Array<attri_type> = mat.get_required_attributes();

                if (rend.is_permanent()) {
                        if (!this.perm_obj.has(rend)) {
                                this.perm_obj.add(rend);

                                // Upload whatever that's is possible if the object is permanent, as we have not done so.
                                var all_attri: Array<attri_type> = rend.available_attributes();
                                for (var i = 0; i < all_attri.length; i++)
                                        rend.upload(backend, all_attri[i]);
                        }
                } else {
                        // Upload just the required parts.        
                        for (var i = 0; i < req_attri.length; i++)
                                rend.upload(backend, req_attri[i]);
                }

                // Upload transformation.
                for (var i = 0; i < req_attri.length; i++)
                        rend.upload_transform(req_attri[i], modelview);
        }

        public upload_light(backend: if_raster_backend, prog: program_location, light: if_light): void
        {
                // @todo: support multi light sources.
                light.upload(backend, prog, 0);
        }

        public get_batched_renderables(): Map<if_material, Array<if_renderable>>
        {
                return this.batched_rends;
        }

        public get_lights(): Map<if_light, pass_light_info>
        {
                return this.lights;
        }

        public get_camera(): camera
        {
                return this.cam;
        }

        public clear(backend: if_raster_backend): void
        {
                this.perm_obj.forEach(function (i, rend: if_renderable, m)
                {
                        rend.unload(backend);
                });

                this.perm_obj.clear();
                this.rends = null;
                this.lights = null;
                this.cam = null;
        }
}

enum pass_buffer_type
{
        radiance
}

interface if_pass
{
        gen_cache(backend: if_raster_backend, scene: scene, camera: camera, frame_id: number): pass_cache;
        run(backend: if_raster_backend, bufs: Map<pass_buffer_type, buffer_location>, cache: pass_cache): [pass_buffer_type, buffer_location];
        release(backend: if_raster_backend): void;
}

class pass_radiance implements if_pass
{
        // private fbo: buffer_location = null;
        private cache: pass_cache = null;

        constructor()
        {
        }

        public gen_cache(backend: if_raster_backend, scene: scene, camera: camera, frame_id: number): pass_cache
        {
                if (this.cache === null) {
                        this.cache = new pass_cache(backend, true);
                }
                this.cache.update(scene, camera, frame_id);
                return this.cache;
        }

        private create_program(backend: if_raster_backend, batched_rends: Array<if_renderable>, mat: if_material, light: if_light): program_location
        {
                if (batched_rends.length === 0)
                        throw new Error("Cannot create program since the batch of renderables is empty");

                var prog: program_location = backend.program_create();

                // construct vertex shader.
                var attris: Array<attri_type> = mat.get_required_attributes();
                var calls = new shader_call_sequence();
                for (var i = 0; i < attris.length; i++) {
                        var call: shader_call = batched_rends[0].get_transform_call(attris[i]);
                        if (call !== null)
                                calls.add_call(call);
                }

                var proj_call = new shader_call(shader_get_builtin_library().get_function("vec3proj"));
                proj_call.bind_param_to_constant(shader_func_param.t_proj);
                proj_call.bind_param_to_shader_input(shader_func_param.position);
                calls.add_call(proj_call);

                var code = calls.gen_glsl_main(shader_get_builtin_library());
                var vert_shader: shader_location = backend.shader_vertex_create(code);

                // construct fragment shader.
                var rad_call: shader_call = mat.get_radiance_call(light.get_irradiance_call(), light.get_incident_call());
                var calls = new shader_call_sequence();
                calls.add_call(rad_call);

                var code: string = calls.gen_glsl_main(shader_get_builtin_library());
                var frag_shader: shader_location = backend.shader_fragment_create(code);

                backend.program_attach_shader(prog, vert_shader);
                backend.program_attach_shader(prog, frag_shader);
                backend.program_link(prog);

                return prog;
        }

        private upload_vertex_projection(backend: if_raster_backend, prog: program_location, proj: mat4): void
        {
                backend.program_assign_uniform(prog,
                        shader_constant_var_info(shader_func_param.t_proj)[0],
                        proj.toarray(),
                        shader_constant_var_info(shader_func_param.t_proj)[1]);
        }

        public run(backend: if_raster_backend, bufs: Map<pass_buffer_type, buffer_location>, cache: pass_cache): [pass_buffer_type, buffer_location]
        {
                var this_: pass_radiance = this;

                var lights: Map<if_light, pass_light_info> = cache.get_lights();
                var rends: Map<if_material, Array<if_renderable>> = cache.get_batched_renderables();

                if (lights.size !== 0) {
                        var first_light: if_light;
                        lights.forEach(function (info: pass_light_info, light: if_light, m)
                        {
                                first_light = light;
                                return;
                        });

                        // camera 
                        var cam: camera = cache.get_camera();
                        var modelview: mat4 = cam.get_modelview_transform();
                        var proj: mat4 = cam.get_frustum().projective_transform().mul(modelview);

                        rends.forEach(function (batched_rends: Array<if_renderable>, mat: if_material, m)
                        {
                                if (batched_rends.length === 0)
                                        return;

                                // @todo: support multiple light sources.
                                var light: if_light = first_light;

                                var prog: program_location = cache.download_material(backend, mat);
                                if (prog === null) {
                                        // @fixme: should batch renderables instead of using the first to construct the program.
                                        prog = this_.create_program(backend, batched_rends, mat, first_light);
                                }

                                // Update program.
                                cache.upload_light(backend, prog, first_light);
                                cache.upload_material(backend, prog, mat);

                                for (var i = 0; i < batched_rends.length; i++) {
                                        cache.upload_renderable(backend, prog, batched_rends[i], modelview, mat);
                                        this_.upload_vertex_projection(backend, prog, proj);

                                        // Draw.
                                        var buf: Array<buffer_info> = batched_rends[i].get_buffer(attri_type.index);
                                        for (var i = 0; i < buf.length; i++) {
                                                backend.draw_indexed_triangles(backend.frame_buf_get_default(), prog,
                                                        buf[i].get_buf(), 0, buf[i].get_len());
                                        }
                                }
                        });
                }
                return [pass_buffer_type.radiance, backend.frame_buf_get_default()];
        }

        public release(backend: if_raster_backend): void
        {
                this.cache.clear(backend);
        }
}