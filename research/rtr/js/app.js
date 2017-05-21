class app_scenes {
    constructor() {
        this.scenes = new Map();
        this.mats = new Map();
        this.curr_scene = null;
        this.load_default_material();
        this.load_default_scenes();
    }
    load_default_material() {
        var lambert = new mat_lambert(new vec3(1.0, 1.0, 1.0));
        this.mats.set("lambertian", lambert);
    }
    load_default_scenes() {
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
    add_scene(id, s) {
        this.curr_scene = s;
        this.scenes.set(id, s);
    }
    select_scene(id) {
        var s = this.scenes.get(id);
    }
    get_current_scene() {
        return this.curr_scene;
    }
    get_all_scene_ids() {
        var sids = new Set();
        this.scenes.forEach(function (s, id, m) {
            sids.add(id);
        });
        return sids;
    }
}
class app {
    constructor() {
        this.rotY = 0.0;
        this.rotY_light = 0.0;
        this.is_draw_light = false;
        this.last_time = 0;
        this.rot_speed = 60;
        this.rot_speed_light = 60;
        this.is_animated = false;
        this.is_animated_light = false;
        this.scenes = new app_scenes();
        this.cam = new camera();
    }
    draw_scene() {
        this.cam.set_position(new vec3(0, 5, -5));
        this.cam.set_target(new vec3(0, 0, 0));
        this.cam.set_up(new vec3(0, 1, 0));
        this.pipeline.render(this.scenes.get_current_scene(), this.cam, new vec4(0.1, 0.1, 0.1, 1.0));
    }
    tick(this_) {
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
    get_scenes() {
        return this.scenes;
    }
    start(canvas) {
        this.width = canvas.width;
        this.height = canvas.height;
        this.backend = gl_create_backend_from_canvas(canvas);
        this.pipeline = new pipe_forward(this.backend);
        this.tick(this);
    }
}
//# sourceMappingURL=app.js.map