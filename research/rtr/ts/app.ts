
/// <reference path="gl.ts" />
/// <reference path="tensor.ts" />
/// <reference path="material.ts" />
/// <reference path="scene.ts" />
/// <reference path="light.ts" />
/// <reference path="camera.ts" />
/// <reference path="pipeline.ts" />


class app_scenes
{
        private scenes = new Map<string, scene>();
        private mats = new Map<string, if_material>();
        private light: light_spot;
        private curr_scene: scene = null;

        private load_default_material(): void
        {
                var lambert: if_material = new mat_lambert(new vec3(1.0, 1.0, 1.0));
                this.mats.set("lambertian", lambert);
        }

        private load_default_light(): void
        {
                this.light = new light_spot();
                this.light.set_position(new vec3());
                this.light.set_dir(new vec3(0, -1, 0));
        }

        private load_default_scenes(): void
        {
                var teapot_scene = new scene();
                var bunny_scene = new scene();
                var mountain_scene = new scene();
                var plate_scene = new scene();
                var cornell_scene = new scene();

                var tteapot = mat4_trota(-1.5708, new vec3(1, 0, 0)).mul(mat4_tscale(new vec3(0.15, 0.15, 0.15)));
                teapot_scene.load_from_obj_str("teapot", eval("teapot_mesh_str"), tteapot, true);
                teapot_scene.add_material(this.mats.get("lambertian"), "lambertian");
                teapot_scene.assign_material_to_renderable("lambertian", "teapot");

                var tbunny = mat4_ttrans(new vec3(0.5, 0, 0));
                bunny_scene.load_from_obj_str("bunny", eval("bunny_mesh_str"), tbunny, true);
                bunny_scene.add_material(this.mats.get("lambertian"), "lambertian");
                bunny_scene.assign_material_to_renderable("lambertian", "bunny");

                var tmountain = mat4_ttrans(new vec3(0.5, -3, -10));
                mountain_scene.load_from_obj_str("mountain", eval("mountain_mesh_str"), tmountain, true);
                mountain_scene.add_material(this.mats.get("lambertian"), "lambertian");
                mountain_scene.assign_material_to_renderable("lambertian", "mountain");

                var tplate = mat4_ttrans(new vec3(0.5, 0, -5));
                plate_scene.load_from_obj_str("cornell", eval("cornell_mesh_str"), tplate, true);
                plate_scene.add_material(this.mats.get("lambertian"), "lambertian");
                plate_scene.assign_material_to_renderable("lambertian", "cornell");

                this.add_scene("bunny_scene", bunny_scene);
                this.add_scene("mountain_scene", mountain_scene);
                this.add_scene("plate_scene", plate_scene);
                this.add_scene("cornell_scene", cornell_scene);
                this.add_scene("teapot_scene", teapot_scene);
        }

        constructor()
        {
                this.load_default_material();
                this.load_default_light();
                this.load_default_scenes();
        }

        public add_scene(id: string, s: scene)
        {
                this.curr_scene = s;
                this.scenes.set(id, s);
        }

        public select_scene(id: string)
        {
                var s = this.scenes.get(id);
        }

        public get_current_scene(): scene
        {
                return this.curr_scene;
        }

        public get_all_scene_ids(): Set<string>
        {
                var sids = new Set<string>();
                this.scenes.forEach(function (s: scene, id: string, m)
                {
                        sids.add(id);
                });
                return sids;
        }
}

class app
{
        /*
        * Screen dimensions.
        */
        private width: number;
        private height: number;

        // Animation related variables
        private rotY: number = 0.0;
        private rotY_light: number = 0.0;
        private is_draw_light = false;

        private last_time = 0;
        private rot_speed = 60;
        private rot_speed_light = 60;

        private is_animated = false;
        private is_animated_light = false;

        /*
        * Default scenes.
        */
        private scenes = new app_scenes();
        private cam = new camera();
        private pipeline: if_pipeline;
        private backend: if_raster_backend;

        private draw_scene(): void
        {
                this.cam.set_position(new vec3(0, 5, -5));
                this.cam.set_target(new vec3(0, 0, 0));
                this.cam.set_up(new vec3(0, 1, 0));

                this.pipeline.render(this.scenes.get_current_scene(), this.cam, new vec4(0.1, 0.1, 0.1, 1.0))
        }

        private tick(this_: app): void
        {
                //requestAnimationFrame(tick);
                setTimeout(this_.tick.bind(null, this_), 1000);

                var timeNow = new Date().getTime();
                if (this_.last_time != 0) {
                        var elapsed = timeNow - this_.last_time;
                        if (this_.is_animated)
                                this_.rotY += this_.rot_speed * 0.0175 * elapsed / 1000.0;
                        if (this_.is_animated_light)
                                this_.rotY_light += this_.rot_speed_light * 0.0175 * elapsed / 1000.0;
                }
                this_.last_time = timeNow;

                this_.draw_scene();
        }

        public get_scenes(): app_scenes
        {
                return this.scenes;
        }

        public start(canvas: HTMLCanvasElement): void
        {
                this.width = canvas.width;
                this.height = canvas.height;

                this.backend = gl_create_backend_from_canvas(canvas);
                this.pipeline = new pipe_forward(this.backend);

                this.tick(this);
        }
}