function create_shader_from_code(backend, code, ext) {
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
function create_shader_from_script_tag(backend, sid) {
    var shaderScript = document.getElementById(sid);
    if (!shaderScript)
        return null;
    var code = "";
    var k = shaderScript.firstChild;
    while (k) {
        if (k.nodeType == 3)
            code += k.textContent;
        k = k.nextSibling;
    }
    var ext;
    if (shaderScript.type == "x-shader/x-fragment") {
        ext = "glslf";
    }
    else if (shaderScript.type == "x-shader/x-vertex") {
        ext = "glslv";
    }
    else {
        return null;
    }
    return create_shader_from_code(backend, code, ext);
}
function last_after(s, stor) {
    var c = s.split(stor);
    return c[c.length - 1];
}
function create_shader_from_ext_script(backend, sid) {
    var path = document.getElementById(sid).src;
    if (!path) {
        alert("Shader script tag doesn't contain the source path (shouldn't import externally?).");
        return null;
    }
    var filename = last_after(path, "/");
    var ext = last_after(filename, ".");
    var varname = filename.replace(".", "");
    var code;
    try {
        code = eval(varname);
    }
    catch (e) {
        alert("Shader source file doesn't contain the string variable: " + varname + " or the shader script hasn't got loaded properly.");
        return null;
    }
    return create_shader_from_code(backend, code, ext);
}
function create_shader_program(backend, vid, fid, is_ext = true) {
    var fs = is_ext ? create_shader_from_ext_script(backend, fid) : create_shader_from_script_tag(backend, fid);
    var vs = is_ext ? create_shader_from_ext_script(backend, vid) : create_shader_from_script_tag(backend, vid);
    var prog = backend.program_create();
    backend.program_attach_shader(prog, vs);
    backend.program_attach_shader(prog, fs);
    backend.program_link(prog);
    return prog;
}
var shader_input;
(function (shader_input) {
    shader_input[shader_input["position"] = 0] = "position";
    shader_input[shader_input["normal"] = 1] = "normal";
    shader_input[shader_input["texcoord"] = 2] = "texcoord";
    shader_input[shader_input["tangent"] = 3] = "tangent";
    shader_input[shader_input["index"] = 4] = "index";
    shader_input[shader_input["hdr_color"] = 5] = "hdr_color";
    shader_input[shader_input["ldr_color"] = 6] = "ldr_color";
    shader_input[shader_input["depth"] = 7] = "depth";
})(shader_input || (shader_input = {}));
var shader_constant;
(function (shader_constant) {
    shader_constant[shader_constant["mv_transform"] = 0] = "mv_transform";
    shader_constant[shader_constant["proj_transform"] = 1] = "proj_transform";
})(shader_constant || (shader_constant = {}));
function shader_io_validate(input, output) {
    if (input != output)
        throw new Error("Shader output is " + output + " but input requires " + input + ".");
    return true;
}
function shader_gen_var_info(input, prefix) {
    switch (input) {
        case shader_input.position:
            return [prefix + "position", "vec3"];
        case shader_input.normal:
            return [prefix + "normal", "vec3"];
        case shader_input.texcoord:
            return [prefix + "texcoord", "vec2"];
        case shader_input.tangent:
            return [prefix + "tangent", "vec3"];
        case shader_input.index:
            return [prefix + "index", "int"];
        case shader_input.hdr_color:
            return [prefix + "hdr_color", "vec4"];
        case shader_input.ldr_color:
            return [prefix + "ldr_color", "vec4"];
        case shader_input.depth:
            return [prefix + "depth", "float"];
        default: {
            if (prefix === "in_")
                throw new Error("Unknown shader input type " + input + ".");
            else
                throw new Error("Unknown shader output type " + input + ".");
        }
    }
}
function shader_input_var_info(input) {
    return shader_gen_var_info(input, "in_");
}
function shader_output_var_info(output) {
    return shader_gen_var_info(output, "out_");
}
function shader_constant_var_info(constant) {
    var prefix = "const_";
    switch (constant) {
        case shader_constant.mv_transform:
            return [prefix + "mv_trans", "mat4"];
        case shader_constant.proj_transform:
            return [prefix + "proj_trans", "mat4"];
        default:
            throw new Error("Unknown shader constant type " + constant + ".");
    }
}
class shader_function {
    constructor(func_name, code) {
        this.input = new Map();
        this.ordered_input = new Array();
        this.output = null;
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
    add_input(param_name, type) {
        if (this.input.has(type))
            throw new Error("Input type " + type + " has already existed.");
        this.ordered_input.push(type);
        this.input.set(type, param_name);
    }
    set_output(type) {
        this.output = type;
    }
    add_required_function(defn) {
        this.func_required.add(defn);
    }
    get_inputs() {
        var ins = new Set();
        this.input.forEach(function (v, input, m) {
            ins.add(input);
        });
        return ins;
    }
    get_ordered_inputs() {
        return this.ordered_input;
    }
    get_output() {
        return this.output;
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
function gen_builtin_library() {
    var lib = new shader_lib();
    lib.check_functions();
    return lib;
}
var shader_input_binding;
(function (shader_input_binding) {
    shader_input_binding[shader_input_binding["constant"] = 0] = "constant";
    shader_input_binding[shader_input_binding["input"] = 1] = "input";
    shader_input_binding[shader_input_binding["call"] = 2] = "call";
})(shader_input_binding || (shader_input_binding = {}));
class shader_call {
    constructor() {
        this.in_call_binding = new Map();
        this.in_input_binding = new Map();
        this.in_const_binding = new Map();
    }
    bind_input_from_shader_input(input, shader_in) {
        this.in_input_binding.set(input, shader_in);
    }
    bind_input_from_constant(input, constant) {
        this.in_const_binding.set(input, constant);
    }
    bind_input_from_call(input, func) {
        shader_io_validate(input, func.func.get_output());
        this.in_call_binding.set(input, func);
    }
    get_input_binding(input) {
        if (this.in_call_binding.has(input))
            return [shader_input_binding.call, this.in_call_binding.get(input)];
        else if (this.in_input_binding.has(input))
            return [shader_input_binding.input, this.in_input_binding.get(input)];
        else if (this.in_const_binding.has(input))
            return [shader_input_binding.constant, this.in_const_binding.get(input)];
        else
            throw new Error("Input " + input + " doesn't have any binding.");
    }
    get_required_shader_input() {
        var ins = new Set();
        this.in_input_binding.forEach(function (input, type, m) {
            ins.add(input);
        });
        return ins;
    }
    get_required_constants() {
        var constants = new Set();
        this.in_const_binding.forEach(function (constanst, type, m) {
            constants.add(constanst);
        });
        return constants;
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
    get_ordered_inputs() {
        return this.func.get_ordered_inputs();
    }
    get_output() {
        return this.func.get_output();
    }
    get_function_name() {
        return this.func.get_function_name();
    }
    check_input_binding() {
        var this_ = this;
        this.func.get_inputs().forEach(function (input, m) {
            var bindings = new Array();
            if (this_.in_const_binding.has(input))
                bindings.push("constant binding");
            if (this_.in_input_binding.has(input))
                bindings.push("shader input binding");
            if (this_.in_call_binding.has(input) &&
                shader_io_validate(input, this_.in_call_binding.get(input).func.get_output()) &&
                this_.in_call_binding.get(input).check_input_binding())
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
            outs.add(this.sequence[i].get_output());
        }
        return outs;
    }
    gen_call_string(func) {
        var call = func.get_function_name() + "(";
        var inputs = func.get_ordered_inputs();
        for (var i = 0; i < inputs.length; i++) {
            var binding_info = func.get_input_binding(inputs[i]);
            switch (binding_info[0]) {
                case shader_input_binding.call:
                    call += this.gen_call_string(binding_info[1]);
                case shader_input_binding.constant:
                    call += shader_constant_var_info(binding_info[1])[0];
                case shader_input_binding.input:
                    call += shader_input_var_info(binding_info[1])[0];
            }
            if (i !== inputs.length - 1)
                call += ",";
        }
        call += ")";
        return call;
    }
    gen_glsl_main(lib) {
        for (var i = 0; i < this.sequence.length; i++) {
            this.sequence[i].check_input_binding();
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
            var info = shader_output_var_info(this.sequence[i].get_output());
            main_section += info[0] + "=" + this.gen_call_string(this.sequence[i]) + ";" + "\n";
        }
        main_section += `
                        }`;
        return const_section + input_section + output_section + defn_section + main_section;
    }
}
//# sourceMappingURL=shader.js.map