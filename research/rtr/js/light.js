class light_spot {
    constructor() {
        this.umbrella = null;
        this.near = 0.1;
        this.far = 30;
        this.exp = 0.0;
        this.dir = null;
        this.p = null;
    }
    set_umbrella(umbrella) {
        this.umbrella = Math.cos(umbrella);
    }
    set_exp(exp) {
        this.exp = exp;
    }
    set_near(near) {
        this.near = near;
    }
    set_far(far) {
        this.far = far;
    }
    set_dir(dir) {
        this.dir = dir;
    }
    set_position(p) {
        this.p = p;
    }
    projective_transforms() {
        return [frustum_perspective(rad2deg(this.umbrella), 1, this.near, this.far)];
    }
    get_irradiance_call() {
        var func = shader_get_builtin_library().get_function("irradspot");
        var irrad_call = new shader_call(func);
        irrad_call.bind_param_to_constant(shader_func_param.lightdir);
        irrad_call.bind_param_to_call(shader_func_param.incident, this.get_incident_call());
        irrad_call.bind_param_to_constant(shader_func_param.exp);
        irrad_call.bind_param_to_constant(shader_func_param.umbrella);
        return irrad_call;
    }
    get_incident_call() {
        var func = shader_get_builtin_library().get_function("incident");
        var incident_call = new shader_call(func);
        incident_call.bind_param_to_constant(shader_func_param.lightpos);
        incident_call.bind_param_to_shader_input(shader_func_param.position);
        return incident_call;
    }
    upload(backend, prog, i) {
        backend.program_assign_uniform(prog, shader_constant_var_info(shader_func_param.lightdir)[0], this.dir.toarray(), shader_constant_var_info(shader_func_param.lightdir)[1]);
        backend.program_assign_uniform(prog, shader_constant_var_info(shader_func_param.exp)[0], [this.exp], shader_constant_var_info(shader_func_param.exp)[1]);
        backend.program_assign_uniform(prog, shader_constant_var_info(shader_func_param.umbrella)[0], [this.umbrella], shader_constant_var_info(shader_func_param.umbrella)[1]);
        backend.program_assign_uniform(prog, shader_constant_var_info(shader_func_param.lightpos)[0], this.p.toarray(), shader_constant_var_info(shader_func_param.lightpos)[1]);
    }
}
//# sourceMappingURL=light.js.map