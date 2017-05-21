var material_type;
(function (material_type) {
    material_type[material_type["lambert"] = 0] = "lambert";
    material_type[material_type["orennayar"] = 1] = "orennayar";
    material_type[material_type["phong"] = 2] = "phong";
    material_type[material_type["blinnphong"] = 3] = "blinnphong";
    material_type[material_type["cooktorr"] = 4] = "cooktorr";
})(material_type || (material_type = {}));
class mat_lambert {
    constructor(albedo) {
        this.albedo = null;
        this.albedo = albedo;
    }
    type() {
        return material_type.lambert;
    }
    set_albedo(a) {
        this.albedo = a;
    }
    is_mergeable(mat) {
        return mat.type() === material_type.lambert;
    }
    get_required_attributes() {
        var attris = new Array();
        attris.push(attri_type.index);
        attris.push(attri_type.normal);
        attris.push(attri_type.position);
        return attris;
    }
    get_radiance_call(irradiance, incident) {
        if (this.albedo == null)
            throw new Error("Parameter albedo constant hasn't been set.");
        var norm_func = shader_get_builtin_library().get_function("vec3normalize");
        var norm_call = new shader_call(norm_func);
        norm_call.bind_param_to_shader_input(shader_func_param.normal);
        var f_rad = shader_get_builtin_library().get_function("brdflambert");
        var c_rad = new shader_call(f_rad);
        c_rad.bind_param_to_call(shader_func_param.incident, incident);
        c_rad.bind_param_to_call(shader_func_param.normal, norm_call);
        c_rad.bind_param_to_call(shader_func_param.irrad, irradiance);
        c_rad.bind_param_to_constant(shader_func_param.albedo);
        return c_rad;
    }
    upload(backend, prog) {
        backend.program_assign_uniform(prog, shader_constant_var_info(shader_func_param.albedo)[0], this.albedo.toarray(), shader_constant_var_info(shader_func_param.albedo)[1]);
    }
}
//# sourceMappingURL=material.js.map