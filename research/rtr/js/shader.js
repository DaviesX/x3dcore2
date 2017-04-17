function shader_create_from_code(backend, code, ext) {
    var shader;
    switch (ext) {
        case "glslv":
            shader = backend.shader_vertex_create(code);
            break;
        case "glslf":
            shader = backend.shader_fragment_create(code);
            break;
        default:
            throw new Error("Incorrect extension " + ext);
    }
    return shader;
}
function shader_code_from_script_tag(sid) {
    var script = document.getElementById(sid);
    if (!script)
        return null;
    var code = "";
    var k = script.firstChild;
    while (k) {
        if (k.nodeType == 3)
            code += k.textContent;
        k = k.nextSibling;
    }
    return [code, script.type];
}
function shader_create_from_script_tag(backend, sid) {
    var info = shader_code_from_script_tag(sid);
    var ext;
    if (info[1] == "x-shader/x-fragment") {
        ext = "glslf";
    }
    else if (info[1] == "x-shader/x-vertex") {
        ext = "glslv";
    }
    else {
        throw new Error("Unknown shader type " + info[1]);
    }
    return shader_create_from_code(backend, info[0], ext);
}
function shader_code_from_ext_script(sid) {
    var path = document.getElementById(sid).src;
    if (!path) {
        alert("Shader script tag doesn't contain the source path (shouldn't import externally?).");
        return null;
    }
    var u_path = new ut_string(path);
    var filename = u_path.last_after("/");
    var ext = u_path.last_after(".");
    var varname = filename.replace(".", "");
    var code;
    try {
        code = eval(varname);
    }
    catch (e) {
        alert("Shader source file doesn't contain the string variable: " + varname + " or the shader script hasn't got loaded properly.");
        return null;
    }
    return [code, ext, filename];
}
function shader_create_from_ext_script(backend, sid) {
    var info = shader_code_from_ext_script(sid);
    return shader_create_from_code(backend, info[0], info[0]);
}
function create_shader_program(backend, vid, fid, is_ext = true) {
    var fs = is_ext ? shader_create_from_ext_script(backend, fid) : shader_create_from_script_tag(backend, fid);
    var vs = is_ext ? shader_create_from_ext_script(backend, vid) : shader_create_from_script_tag(backend, vid);
    var prog = backend.program_create();
    backend.program_attach_shader(prog, vs);
    backend.program_attach_shader(prog, fs);
    backend.program_link(prog);
    return prog;
}
var shader_func_param;
(function (shader_func_param) {
    shader_func_param[shader_func_param["position"] = 0] = "position";
    shader_func_param[shader_func_param["normal"] = 1] = "normal";
    shader_func_param[shader_func_param["texcoord"] = 2] = "texcoord";
    shader_func_param[shader_func_param["tangent"] = 3] = "tangent";
    shader_func_param[shader_func_param["color_hdr_diff"] = 4] = "color_hdr_diff";
    shader_func_param[shader_func_param["color_hdr_spec"] = 5] = "color_hdr_spec";
    shader_func_param[shader_func_param["color_hdr_ambi"] = 6] = "color_hdr_ambi";
    shader_func_param[shader_func_param["color_hdr"] = 7] = "color_hdr";
    shader_func_param[shader_func_param["color_ldr"] = 8] = "color_ldr";
    shader_func_param[shader_func_param["adapted_lum"] = 9] = "adapted_lum";
    shader_func_param[shader_func_param["depth"] = 10] = "depth";
    shader_func_param[shader_func_param["depth_max"] = 11] = "depth_max";
    shader_func_param[shader_func_param["position_max"] = 12] = "position_max";
    shader_func_param[shader_func_param["t_modelview"] = 13] = "t_modelview";
    shader_func_param[shader_func_param["t_nmodelview"] = 14] = "t_nmodelview";
    shader_func_param[shader_func_param["t_proj"] = 15] = "t_proj";
    shader_func_param[shader_func_param["ambient"] = 16] = "ambient";
    shader_func_param[shader_func_param["albedo"] = 17] = "albedo";
    shader_func_param[shader_func_param["metalness"] = 18] = "metalness";
    shader_func_param[shader_func_param["alpha"] = 19] = "alpha";
    shader_func_param[shader_func_param["beta"] = 20] = "beta";
    shader_func_param[shader_func_param["sigma"] = 21] = "sigma";
    shader_func_param[shader_func_param["ior"] = 22] = "ior";
    shader_func_param[shader_func_param["exp"] = 23] = "exp";
    shader_func_param[shader_func_param["umbrella"] = 24] = "umbrella";
    shader_func_param[shader_func_param["lightinten"] = 25] = "lightinten";
    shader_func_param[shader_func_param["lightpos"] = 26] = "lightpos";
    shader_func_param[shader_func_param["lightdir"] = 27] = "lightdir";
    shader_func_param[shader_func_param["irrad"] = 28] = "irrad";
    shader_func_param[shader_func_param["incident"] = 29] = "incident";
    shader_func_param[shader_func_param["emergent"] = 30] = "emergent";
    shader_func_param[shader_func_param["hdr_sampler"] = 31] = "hdr_sampler";
})(shader_func_param || (shader_func_param = {}));
var shader_func_ret;
(function (shader_func_ret) {
    shader_func_ret[shader_func_ret["position"] = 0] = "position";
    shader_func_ret[shader_func_ret["normal"] = 1] = "normal";
    shader_func_ret[shader_func_ret["texcoord"] = 2] = "texcoord";
    shader_func_ret[shader_func_ret["tangent"] = 3] = "tangent";
    shader_func_ret[shader_func_ret["incident"] = 4] = "incident";
    shader_func_ret[shader_func_ret["emergent"] = 5] = "emergent";
    shader_func_ret[shader_func_ret["albedo"] = 6] = "albedo";
    shader_func_ret[shader_func_ret["irrad"] = 7] = "irrad";
    shader_func_ret[shader_func_ret["color_hdr_diff"] = 8] = "color_hdr_diff";
    shader_func_ret[shader_func_ret["color_hdr_spec"] = 9] = "color_hdr_spec";
    shader_func_ret[shader_func_ret["color_hdr_ambi"] = 10] = "color_hdr_ambi";
    shader_func_ret[shader_func_ret["color_hdr"] = 11] = "color_hdr";
    shader_func_ret[shader_func_ret["color_ldr"] = 12] = "color_ldr";
    shader_func_ret[shader_func_ret["depth"] = 13] = "depth";
})(shader_func_ret || (shader_func_ret = {}));
function shader_func_io_validate(param, ret) {
    if (param.toString() !== ret.toString())
        throw new Error("Return value of the shader function is " + ret
            + " but is fed into the one of the parameters of type " + param + " of another function"
            + ", which is incompatible.");
    return true;
}
function shader_input_var_info(input) {
    var prefix = "in_";
    switch (input) {
        case shader_func_param.position:
            return [prefix + shader_func_param.position.toString(), "vec3"];
        case shader_func_param.normal:
            return [prefix + shader_func_param.normal.toString(), "vec3"];
        case shader_func_param.texcoord:
            return [prefix + shader_func_param.texcoord.toString(), "vec2"];
        case shader_func_param.tangent:
            return [prefix + shader_func_param.tangent.toString(), "vec3"];
        case shader_func_param.depth:
            return [prefix + shader_func_param.depth.toString(), "float"];
        default: {
            throw new Error("Unknown shader input type " + input + ".");
        }
    }
}
function shader_output_var_info(output) {
    var prefix = "out_";
    switch (output) {
        case shader_func_ret.position:
            return [prefix + shader_func_ret.position.toString(), "vec3"];
        case shader_func_ret.normal:
            return [prefix + shader_func_ret.normal.toString(), "vec3"];
        case shader_func_ret.texcoord:
            return [prefix + shader_func_ret.texcoord.toString(), "vec2"];
        case shader_func_ret.tangent:
            return [prefix + shader_func_ret.tangent.toString(), "vec3"];
        case shader_func_ret.color_hdr:
        case shader_func_ret.color_ldr:
            return ["gl_FragColor", "vec4"];
        case shader_func_ret.depth:
            return ["gl_FragDepth", "float"];
        default: {
            throw new Error("Unknown shader output type " + output + ".");
        }
    }
}
function shader_constant_var_info(constant) {
    var prefix = "const_";
    switch (constant) {
        case shader_func_param.t_modelview:
            return [prefix + shader_func_param.t_modelview.toString(), "mat4"];
        case shader_func_param.t_proj:
            return [prefix + shader_func_param.t_proj.toString(), "mat4"];
        case shader_func_param.lightpos:
            return [prefix + shader_func_param.lightpos.toString(), "vec3"];
        case shader_func_param.ambient:
            return [prefix + shader_func_param.ambient.toString(), "vec3"];
        case shader_func_param.albedo:
            return [prefix + shader_func_param.albedo.toString(), "vec3"];
        case shader_func_param.metalness:
            return [prefix + shader_func_param.metalness.toString(), "float"];
        case shader_func_param.alpha:
            return [prefix + shader_func_param.alpha.toString(), "float"];
        case shader_func_param.beta:
            return [prefix + shader_func_param.beta.toString(), "float"];
        case shader_func_param.sigma:
            return [prefix + shader_func_param.sigma.toString(), "float"];
        case shader_func_param.ior:
            return [prefix + shader_func_param.ior.toString(), "float"];
        case shader_func_param.exp:
            return [prefix + shader_func_param.exp.toString(), "float"];
        case shader_func_param.umbrella:
            return [prefix + shader_func_param.umbrella.toString(), "float"];
        case shader_func_param.depth_max:
            return [prefix + shader_func_param.depth_max.toString(), "float"];
        case shader_func_param.position_max:
            return [prefix + shader_func_param.depth_max.toString(), "vec3"];
        case shader_func_param.lightinten:
            return [prefix + shader_func_param.lightinten.toString(), "vec3"];
        case shader_func_param.lightdir:
            return [prefix + shader_func_param.lightdir.toString(), "vec3"];
        case shader_func_param.hdr_sampler:
            return [prefix + shader_func_param.hdr_sampler.toString(), "sample2D"];
        case shader_func_param.adapted_lum:
            return [prefix + shader_func_param.adapted_lum.toString(), "vec3"];
        default:
            throw new Error("Unknown shader constant type " + constant + ".");
    }
}
class shader_function {
    constructor(func_name, code) {
        this.params = new Set();
        this.ordered_params = new Array();
        this.ret = null;
        this.func_required = new Set();
        this.func_name = name;
        this.code = code;
    }
    get_function_name() {
        return this.func_name;
    }
    get_definition() {
        return this.code;
    }
    add_param(param) {
        if (this.params.has(param))
            throw new Error("Parameter type " + param + " has already existed.");
        this.ordered_params.push(param);
        this.params.add(param);
    }
    set_ret(ret) {
        this.ret = ret;
    }
    add_required_function(defn) {
        this.func_required.add(defn);
    }
    get_params() {
        return this.params;
    }
    get_ordered_params() {
        return this.ordered_params;
    }
    get_ret() {
        return this.ret;
    }
    get_functions_required() {
        return this.func_required;
    }
    check_function_required(funcs) {
        var callstack = new Set();
        this.func_required.forEach(function (target, source, m) {
            if (!funcs.has(target))
                throw new Error("Function definition " + target + " required by " + source + " is invalid.");
            else {
                if (callstack.has(target))
                    throw new Error("Circular call from " + source + " to " + target + ".");
                callstack.add(target);
                funcs.get(target).check_function_required(funcs);
                callstack.delete(target);
            }
        });
    }
}
class shader_lib {
    constructor() {
        this.funcs = new Map();
    }
    add_function(func) {
        this.funcs.set(func.get_function_name(), func);
    }
    get_function(func_name) {
        if (!this.funcs.has(func_name))
            throw new Error("Shader function " + func_name + " doesn't exist.");
        return this.funcs.get(func_name);
    }
    has_function(func_name) {
        return this.funcs.has(func_name);
    }
    check_functions() {
        var this_ = this;
        this.funcs.forEach(function (func, name, m) {
            func.check_function_required(this_.funcs);
        });
    }
}
var shader_param_binding;
(function (shader_param_binding) {
    shader_param_binding[shader_param_binding["constant"] = 0] = "constant";
    shader_param_binding[shader_param_binding["input"] = 1] = "input";
    shader_param_binding[shader_param_binding["call"] = 2] = "call";
})(shader_param_binding || (shader_param_binding = {}));
class shader_call {
    constructor() {
        this.param_call_binding = new Map();
        this.param_input_binding = new Set();
        this.param_const_binding = new Set();
    }
    bind_param_to_shader_input(param) {
        this.param_input_binding.add(param);
    }
    bind_param_to_constant(param) {
        this.param_const_binding.add(param);
    }
    bind_param_to_call(param, call) {
        shader_func_io_validate(param, call.func.get_ret());
        this.param_call_binding.set(param, call);
    }
    get_param_binding(param) {
        if (this.param_call_binding.has(param))
            return [shader_param_binding.call, this.param_call_binding.get(param)];
        else if (this.param_input_binding.has(param))
            return [shader_param_binding.input, null];
        else if (this.param_const_binding.has(param))
            return [shader_param_binding.constant, null];
        else
            throw new Error("Input " + param + " doesn't have any binding.");
    }
    get_required_shader_input() {
        return this.param_input_binding;
    }
    get_required_constants() {
        return this.param_const_binding;
    }
    get_all_required_functions(func, lib) {
        var result = new Array();
        var loaded_funcs = new Set();
        func.get_functions_required().forEach(function (func_name, s) {
            result.concat(this.get_all_required_function_definitions(lib.get_function(func_name), lib));
            if (!loaded_funcs.has(func_name)) {
                result.push(lib.get_function(func_name));
                loaded_funcs.add(func_name);
            }
        });
        result.push(func);
        return result;
    }
    get_required_functions(lib) {
        return this.get_all_required_functions(this.func, lib);
    }
    get_ordered_params() {
        return this.func.get_ordered_params();
    }
    get_ret() {
        return this.func.get_ret();
    }
    get_function_name() {
        return this.func.get_function_name();
    }
    check_param_binding() {
        var this_ = this;
        this.func.get_params().forEach(function (input, m) {
            var bindings = new Array();
            if (this_.param_const_binding.has(input))
                bindings.push("constant binding");
            if (this_.param_input_binding.has(input))
                bindings.push("shader input binding");
            if (this_.param_call_binding.has(input) &&
                shader_func_io_validate(input, this_.param_call_binding.get(input).func.get_ret()) &&
                this_.param_call_binding.get(input).check_param_binding())
                bindings.push("shader function call binding");
            if (bindings.length == 0)
                throw new Error("Shader function " + this_.func.get_function_name()
                    + " requires input " + input + " but has no binding to such.");
            else if (bindings.length > 1) {
                var sentence = bindings[0];
                for (var i = 1; i < bindings.length; i++)
                    sentence += " & " + bindings[i];
                throw new Error("Shader function " + this_.func.get_function_name
                    + " has bindings " + sentence + " at the same time, which is ambiguous.");
            }
        });
    }
}
class shader_call_sequence {
    constructor() {
        this.sequence = new Array();
    }
    add_call(call) {
        this.sequence.push(call);
    }
    get_required_constants() {
        var constants = new Set();
        for (var i = 0; i < this.sequence.length; i++) {
            var call = this.sequence[i];
            call.get_required_constants().forEach(function (constant, v, s) {
                constants.add(constant);
            });
        }
        return constants;
    }
    get_required_inputs() {
        var ins = new Set();
        for (var i = 0; i < this.sequence.length; i++) {
            var call = this.sequence[i];
            call.get_required_shader_input().forEach(function (input, v, s) {
                ins.add(input);
            });
        }
        return ins;
    }
    get_required_functions(lib) {
        var funcs = new Array();
        var loaded_funcs = new Set();
        for (var i = 0; i < this.sequence.length; i++) {
            var call = this.sequence[i];
            call.get_required_functions(lib).forEach(function (fs, v, s) {
                if (!loaded_funcs.has(fs.get_function_name())) {
                    funcs.push(fs);
                    loaded_funcs.add(fs.get_function_name());
                }
            });
        }
        return funcs;
    }
    get_outputs() {
        var outs = new Set();
        for (var i = 0; i < this.sequence.length; i++) {
            outs.add(this.sequence[i].get_ret());
        }
        return outs;
    }
    gen_call_string(call) {
        var call_str = call.get_function_name() + "(";
        var params = call.get_ordered_params();
        for (var i = 0; i < params.length; i++) {
            var binding_info = call.get_param_binding(params[i]);
            switch (binding_info[0]) {
                case shader_param_binding.call:
                    call_str += this.gen_call_string(binding_info[1]);
                case shader_param_binding.constant:
                    call_str += shader_constant_var_info(params[i])[0];
                case shader_param_binding.input:
                    call_str += shader_input_var_info(params[1])[0];
            }
            if (i !== params.length - 1)
                call_str += ",";
        }
        call_str += ")";
        return call_str;
    }
    gen_glsl_main(lib) {
        for (var i = 0; i < this.sequence.length; i++) {
            this.sequence[i].check_param_binding();
        }
        var const_section = "";
        var input_section = "";
        var output_section = "";
        var defn_section = "";
        var main_section = "";
        var constants = this.get_required_constants();
        constants.forEach(function (constant, v, s) {
            var info = shader_constant_var_info(constant);
            const_section += "uniform " + info[1] + " " + info[0] + ";" + "\n";
        });
        var inputs = this.get_required_inputs();
        inputs.forEach(function (input, v, s) {
            var info = shader_input_var_info(input);
            const_section += "varying " + info[1] + " " + info[0] + ";" + "\n";
        });
        var outputs = this.get_outputs();
        outputs.forEach(function (output, v, s) {
            var info = shader_output_var_info(output);
            const_section += "varying " + info[1] + " " + info[0] + ";" + "\n";
        });
        var funcs = this.get_required_functions(lib);
        for (var i = 0; i < funcs.length; i++) {
            defn_section += funcs[i].get_definition() + "\n";
        }
        main_section += `
                        void main()
                        {`;
        for (var i = 0; i < this.sequence.length; i++) {
            var info = shader_output_var_info(this.sequence[i].get_ret());
            main_section += info[0] + "=" + this.gen_call_string(this.sequence[i]) + ";" + "\n";
        }
        main_section += `
                        }`;
        return const_section + input_section + output_section + defn_section + main_section;
    }
}
function shade_gen_builtin_library() {
    var lib = new shader_lib();
    var codeinfo;
    var func;
    codeinfo = shader_code_from_ext_script("sl_albedodiff");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.albedo);
    func.add_param(shader_func_param.metalness);
    func.set_ret(shader_func_ret.albedo);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_albedospec");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.albedo);
    func.add_param(shader_func_param.metalness);
    func.set_ret(shader_func_ret.albedo);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfambient");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.albedo);
    func.add_param(shader_func_param.ambient);
    func.set_ret(shader_func_ret.color_hdr_ambi);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfblinnphong");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.incident);
    func.add_param(shader_func_param.normal);
    func.add_param(shader_func_param.emergent);
    func.add_param(shader_func_param.irrad);
    func.add_param(shader_func_param.albedo);
    func.add_param(shader_func_param.alpha);
    func.set_ret(shader_func_ret.color_hdr_spec);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfcolor");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.albedo);
    func.set_ret(shader_func_ret.color_hdr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfcooktorr");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.incident);
    func.add_param(shader_func_param.normal);
    func.add_param(shader_func_param.emergent);
    func.add_param(shader_func_param.irrad);
    func.add_param(shader_func_param.albedo);
    func.add_param(shader_func_param.beta);
    func.add_param(shader_func_param.ior);
    func.set_ret(shader_func_ret.color_hdr_spec);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfdepth");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.position);
    func.add_param(shader_func_param.depth_max);
    func.set_ret(shader_func_ret.color_hdr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfdirection");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.position);
    func.add_param(shader_func_param.depth_max);
    func.set_ret(shader_func_ret.color_hdr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdflambert");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.incident);
    func.add_param(shader_func_param.normal);
    func.add_param(shader_func_param.irrad);
    func.add_param(shader_func_param.albedo);
    func.set_ret(shader_func_ret.color_hdr_diff);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfnormal");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.normal);
    func.set_ret(shader_func_ret.color_hdr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdforennayar");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.incident);
    func.add_param(shader_func_param.normal);
    func.add_param(shader_func_param.emergent);
    func.add_param(shader_func_param.irrad);
    func.add_param(shader_func_param.albedo);
    func.add_param(shader_func_param.sigma);
    func.set_ret(shader_func_ret.color_hdr_diff);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfphong");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.incident);
    func.add_param(shader_func_param.normal);
    func.add_param(shader_func_param.emergent);
    func.add_param(shader_func_param.irrad);
    func.add_param(shader_func_param.albedo);
    func.add_param(shader_func_param.alpha);
    func.set_ret(shader_func_ret.color_hdr_spec);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfposition");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.position);
    func.add_param(shader_func_param.position_max);
    func.set_ret(shader_func_ret.color_hdr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfwhite");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.set_ret(shader_func_ret.color_hdr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_brdfadd3");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.color_hdr_diff);
    func.add_param(shader_func_param.color_hdr_spec);
    func.add_param(shader_func_param.color_hdr_ambi);
    func.set_ret(shader_func_ret.color_hdr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_emergent");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.position);
    func.set_ret(shader_func_ret.emergent);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_incident");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.lightpos);
    func.add_param(shader_func_param.position);
    func.set_ret(shader_func_ret.emergent);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_irradpoint");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.incident);
    func.add_param(shader_func_param.lightinten);
    func.set_ret(shader_func_ret.irrad);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_irradspot");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.lightdir);
    func.add_param(shader_func_param.incident);
    func.add_param(shader_func_param.lightinten);
    func.add_param(shader_func_param.exp);
    func.add_param(shader_func_param.umbrella);
    func.set_ret(shader_func_ret.irrad);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_irradsun");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.lightdir);
    func.add_param(shader_func_param.lightinten);
    func.set_ret(shader_func_ret.irrad);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_loglum");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.color_hdr);
    func.set_ret(shader_func_ret.color_hdr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_texhdrcolor");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.hdr_sampler);
    func.add_param(shader_func_param.texcoord);
    func.set_ret(shader_func_ret.color_hdr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_tonemapaces");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.color_hdr);
    func.add_param(shader_func_param.adapted_lum);
    func.set_ret(shader_func_ret.color_ldr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_tonemapexp");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.color_hdr);
    func.add_param(shader_func_param.adapted_lum);
    func.set_ret(shader_func_ret.color_ldr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_tonemapfilmic");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.color_hdr);
    func.add_param(shader_func_param.adapted_lum);
    func.set_ret(shader_func_ret.color_ldr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_tonemapreinhard");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.color_hdr);
    func.add_param(shader_func_param.adapted_lum);
    func.set_ret(shader_func_ret.color_ldr);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_vec3normalize");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.normal);
    func.set_ret(shader_func_ret.normal);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_vec3modelview");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.t_modelview);
    func.add_param(shader_func_param.position);
    func.set_ret(shader_func_ret.normal);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_vec3nmodelview");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.t_nmodelview);
    func.add_param(shader_func_param.normal);
    func.set_ret(shader_func_ret.normal);
    lib.add_function(func);
    codeinfo = shader_code_from_ext_script("sl_vec3proj");
    func = new shader_function(codeinfo[2], codeinfo[0]);
    func.add_param(shader_func_param.t_proj);
    func.add_param(shader_func_param.position);
    func.set_ret(shader_func_ret.normal);
    lib.add_function(func);
    lib.check_functions();
    return lib;
}
//# sourceMappingURL=shader.js.map