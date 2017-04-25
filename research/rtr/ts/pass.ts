
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

        private rend = new Map<if_renderable, pass_rend_info>();
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
                rend.forEach(function (mat: if_material, r: if_renderable, m)
                {
                        var info: pass_rend_info = this_.rend.get(r);
                        // Update renderable info.
                        if (info == null) {
                                this_.rend.set(r, new pass_rend_info(mat, frame_id));
                        } else {
                                info.appeared(frame_id);
                        }
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
                rend: if_renderable, modelview: mat4, proj: mat4, mat: if_material): void
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

                // Upload transformation calls.
                for (var i = 0; i < req_attri.length; i++)
                        rend.upload_transform_call(backend, prog, req_attri[i], modelview, proj);
        }

        public upload_light(backend: if_raster_backend, prog: program_location, light: if_light): void
        {
                // @todo: support multi light sources.
                light.upload(backend, prog, 0);
        }

        public get_renderables(): Map<if_renderable, pass_rend_info>
        {
                return this.rend;
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
                this.rend = null;
                this.lights = null;
                this.cam = null;
        }
}

function pass_create_program_from_material(backend: if_raster_backend, rend: if_renderable, mat: if_material, light: if_light): program_location
{
        var prog: program_location = backend.program_create();

        // construct vertex shader.
        var attris: Array<attri_type> = mat.get_required_attributes();
        var calls = new shader_call_sequence();
        for (var i = 0; i < attris.length; i++)
                calls.add_call(rend.get_transform_call(attris[i]));

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

        public run(backend: if_raster_backend, bufs: Map<pass_buffer_type, buffer_location>, cache: pass_cache): [pass_buffer_type, buffer_location]
        {
                var this_: pass_radiance = this;

                var lights: Map<if_light, pass_light_info> = cache.get_lights();
                var rends: Map<if_renderable, pass_rend_info> = cache.get_renderables();

                if (lights.size !== 0) {
                        var first_light: if_light;
                        lights.forEach(function (info: pass_light_info, light: if_light, m)
                        {
                                first_light = light;
                                return;
                        });

                        // camera 
                        var cam: camera = cache.get_camera();

                        rends.forEach(function (info: pass_rend_info, rend: if_renderable, m)
                        {
                                // @todo: support multiple light sources.
                                var light: if_light = first_light;

                                var prog: program_location = cache.download_material(backend, info.mat);
                                if (prog === null)
                                        prog = pass_create_program_from_material(backend, rend, info.mat, first_light);

                                // Update program.
                                cache.upload_light(backend, prog, first_light);
                                cache.upload_material(backend, prog, info.mat);
                                cache.upload_renderable(backend, prog, rend,
                                        cam.get_modelview_transform(),
                                        cam.get_frustum().projective_transform(), info.mat);

                                // Draw.
                                var buf: Array<buffer_info> = rend.get_buffer(attri_type.index);
                                for (var i = 0; i < buf.length; i++)
                                        backend.draw_indexed_triangles(backend.frame_buf_get_default(), prog, buf[i].get_buf(), 0, buf[i].get_len());
                        });
                }
                return [pass_buffer_type.radiance, backend.frame_buf_get_default()];
        }

        public release(backend: if_raster_backend): void
        {
                this.cache.clear(backend);
        }
}