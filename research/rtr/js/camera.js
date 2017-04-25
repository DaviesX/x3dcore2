class camera {
    constructor() {
        this.fovy = 60;
        this.aspect = 4.0 / 3.0;
        this.z_near = 1.0;
        this.z_far = 1000.0;
        this.p = new vec3(0, 0, 0);
        this.target = new vec3(0, 0, 0);
        this.up = new vec3(0, 1, 0);
    }
    set_position(p) {
        this.p = p;
    }
    set_target(target) {
        this.target = target;
    }
    set_up(up) {
        this.up = up;
    }
    get_position() {
        return this.p;
    }
    get_direction() {
        return this.p.sub(this.target).norm();
    }
    get_frustum() {
        return frustum_perspective(this.fovy, this.aspect, this.z_near, this.z_far);
    }
    get_modelview_transform() {
        var t = mat4_ttrans(this.p);
        var w = this.get_position();
        var u = w.outter(this.up);
        var v = u.outter(w);
        var r = mat4_trotd(u, v, w);
        return t.mul(r);
    }
}
//# sourceMappingURL=camera.js.map