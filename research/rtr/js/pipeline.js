class pipe_forward {
    constructor(backend) {
        this.rad_pass = new pass_radiance();
        this.frame_id = 0;
        this.backend = backend;
    }
    render(s, c, bgcolor) {
        this.backend.frame_buf_set_background(this.backend.frame_buf_get_default(), bgcolor.x, bgcolor.y, bgcolor.z, bgcolor.w);
        this.backend.frame_buf_fill(this.backend.frame_buf_get_default(), true, true);
        var cache = this.rad_pass.gen_cache(this.backend, s, c, this.frame_id);
        this.rad_pass.run(this.backend, new Map(), cache);
        this.frame_id++;
    }
    release() {
        this.rad_pass.release(this.backend);
    }
}
//# sourceMappingURL=pipeline.js.map