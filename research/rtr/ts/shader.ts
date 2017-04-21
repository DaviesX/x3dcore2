
/// <reference path="typing/webgl2.d.ts" />
/// <reference path="utils.ts" />
/// <reference path="gl.ts" />


function shader_create_from_code(backend: if_raster_backend, code: string, ext: string): shader_location
{
        var shader: shader_location;

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

function shader_code_from_script_tag(sid: string): [string, string]
{
        var script = <HTMLScriptElement>document.getElementById(sid);
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

function shader_create_from_script_tag(backend: if_raster_backend, sid: string): shader_location
{
        var info = shader_code_from_script_tag(sid);
        var ext;
        if (info[1] == "x-shader/x-fragment") {
                ext = "glslf";
        } else if (info[1] == "x-shader/x-vertex") {
                ext = "glslv";
        } else {
                throw new Error("Unknown shader type " + info[1]);
        }

        return shader_create_from_code(backend, info[0], ext);
}



function shader_code_from_ext_script(sid: string): [string, string, string]
{
        var path = (<HTMLScriptElement>document.getElementById(sid)).src;
        if (!path) {
                alert("Shader script tag doesn't contain the source path (shouldn't import externally?).");
                return null;
        }

        var u_path = new ut_string(path);

        var filename: string = u_path.last_after("/");
        var ext: string = u_path.last_after(".");
        var varname: string = filename.replace(".", "");
        var code: string;
        try {
                code = eval(varname);
        } catch (e) {
                alert("Shader source file doesn't contain the string variable: " + varname + " or the shader script hasn't got loaded properly.");
                return null;
        }

        return [code, ext, filename];
}

function shader_create_from_ext_script(backend: if_raster_backend, sid: string): shader_location
{
        var info = shader_code_from_ext_script(sid);
        return shader_create_from_code(backend, info[0], info[0]);
}


function create_shader_program(backend: if_raster_backend, vid: string, fid: string, is_ext: boolean = true): program_location
{
        var fs: shader_location = is_ext ? shader_create_from_ext_script(backend, fid) : shader_create_from_script_tag(backend, fid);
        var vs: shader_location = is_ext ? shader_create_from_ext_script(backend, vid) : shader_create_from_script_tag(backend, vid);
        var prog: program_location = backend.program_create();

        backend.program_attach_shader(prog, vs);
        backend.program_attach_shader(prog, fs);
        backend.program_link(prog);
        return prog;
}


enum shader_func_param
{
        position,
        normal,
        texcoord,
        tangent,

        color_hdr_diff,
        color_hdr_spec,
        color_hdr_ambi,
        color_hdr,
        color_ldr,
        adapted_lum,
        depth,

        depth_max,
        position_max,

        t_modelview,
        t_nmodelview,
        t_proj,

        ambient,
        albedo,
        metalness,
        alpha,
        beta,
        sigma,
        ior,
        exp,
        umbrella,

        lightinten,
        lightpos,
        lightdir,

        irrad,

        incident,
        emergent,

        hdr_sampler,
}

enum shader_func_ret
{
        position,
        normal,
        texcoord,
        tangent,

        incident,
        emergent,

        albedo,
        irrad,

        color_hdr_diff,
        color_hdr_spec,
        color_hdr_ambi,

        color_hdr,
        color_ldr,
        depth
}


function shader_func_io_validate(param: shader_func_param, ret: shader_func_ret): boolean
{
        if (param.toString() !== ret.toString())
                throw new Error("Return value of the shader function is " + ret
                        + " but is fed into the one of the parameters of type " + param + " of another function"
                        + ", which is incompatible.");
        return true;
}

function shader_input_var_info(input: shader_func_param): [string, string]
{
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

function shader_output_var_info(output: shader_func_ret): [string, string]
{
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

function shader_constant_var_info(constant: shader_func_param): [string, string]
{
        var prefix: string = "const_";
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



class shader_function
{
        private func_name: string;
        private code: string;

        private params = new Set<shader_func_param>();
        private ordered_params = new Array<shader_func_param>();
        private ret: shader_func_ret = null;

        private func_required = new Set<string>();

        constructor(func_name: string, code: string)
        {
                this.func_name = name;
                this.code = code;
        }

        public get_function_name(): string
        {
                return this.func_name;
        }

        public get_definition(): string
        {
                return this.code;
        }

        public add_param(param: shader_func_param): void
        {
                if (this.params.has(param))
                        throw new Error("Parameter type " + param + " has already existed.");
                this.ordered_params.push(param);
                this.params.add(param);
        }

        public set_ret(ret: shader_func_ret): void
        {
                this.ret = ret;
        }

        public add_required_function(defn: string): void
        {
                this.func_required.add(defn);
        }

        public get_params(): Set<shader_func_param>
        {
                return this.params;
        }

        public get_ordered_params(): Array<shader_func_param>
        {
                return this.ordered_params;
        }

        public get_ret(): shader_func_ret
        {
                return this.ret;
        }

        public get_functions_required(): Set<string>
        {
                return this.func_required;
        }

        public check_function_required(funcs: Map<string, shader_function>): void
        {
                var callstack = new Set<string>();

                this.func_required.forEach(function (target: string, source: string, m)
                {
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

class shader_lib
{
        private funcs = new Map<string, shader_function>();

        constructor()
        {
        }

        public add_function(func: shader_function): void
        {
                this.funcs.set(func.get_function_name(), func);
        }

        public get_function(func_name: string): shader_function
        {
                if (!this.funcs.has(func_name))
                        throw new Error("Shader function " + func_name + " doesn't exist.");
                return this.funcs.get(func_name);
        }

        public has_function(func_name: string): boolean
        {
                return this.funcs.has(func_name);
        }

        public check_functions(): void
        {
                var this_: shader_lib = this;
                this.funcs.forEach(function (func: shader_function, name, m)
                {
                        func.check_function_required(this_.funcs);
                });
        }
}

enum shader_param_binding
{
        constant,
        input,
        call
}

class shader_call
{
        private func: shader_function;

        private param_call_binding = new Map<shader_func_param, shader_call>();
        private param_input_binding = new Set<shader_func_param>();
        private param_const_binding = new Set<shader_func_param>();

        constructor(func: shader_function)
        {
                this.func = func;
        }

        public bind_param_to_shader_input(param: shader_func_param)
        {
                this.param_input_binding.add(param);
        }

        public bind_param_to_constant(param: shader_func_param): void
        {
                this.param_const_binding.add(param);
        }

        public bind_param_to_call(param: shader_func_param, call: shader_call): void
        {
                shader_func_io_validate(param, call.func.get_ret());
                this.param_call_binding.set(param, call);
        }

        public get_param_binding(param: shader_func_param): [shader_param_binding, shader_call]
        {
                if (this.param_call_binding.has(param))
                        return [shader_param_binding.call, this.param_call_binding.get(param)];
                else if (this.param_input_binding.has(param))
                        return [shader_param_binding.input, null];
                else if (this.param_const_binding.has(param))
                        return [shader_param_binding.constant, null];
                else
                        throw new Error("Input " + param + " doesn't have any binding.");
        }

        public get_required_shader_input(): Set<shader_func_param>
        {
                return this.param_input_binding;
        }

        public get_required_constants(): Set<shader_func_param>
        {
                return this.param_const_binding;
        }

        private get_all_required_functions(func: shader_function, lib: shader_lib): Array<shader_function>
        {
                var result = new Array<shader_function>();
                var loaded_funcs = new Set<string>();

                // Trace the dependency tree.
                func.get_functions_required().forEach(function (func_name: string, s)
                {
                        result.concat(this.get_all_required_function_definitions(lib.get_function(func_name), lib));
                        if (!loaded_funcs.has(func_name)) {
                                result.push(lib.get_function(func_name));
                                loaded_funcs.add(func_name);
                        }
                });
                // Add the root function itself.
                result.push(func);
                return result;
        }

        public get_required_functions(lib: shader_lib): Array<shader_function>
        {
                return this.get_all_required_functions(this.func, lib);
        }

        public get_ordered_params(): Array<shader_func_param>
        {
                return this.func.get_ordered_params();
        }

        public get_ret(): shader_func_ret
        {
                return this.func.get_ret();
        }

        public get_function_name(): string
        {
                return this.func.get_function_name();
        }

        public check_param_binding(): void
        {
                var this_: shader_call = this;

                this.func.get_params().forEach(function (input: shader_func_param, m)
                {
                        var bindings = new Array<string>();
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
                                var sentence: string = bindings[0];
                                for (var i = 1; i < bindings.length; i++)
                                        sentence += " & " + bindings[i];
                                throw new Error("Shader function " + this_.func.get_function_name
                                        + " has bindings " + sentence + " at the same time, which is ambiguous.");
                        }
                });
        }
}

class shader_call_sequence
{
        private sequence = new Array<shader_call>();

        constructor()
        {
        }

        public add_call(call: shader_call): void
        {
                this.sequence.push(call);
        }

        public get_required_constants(): Set<shader_func_param>
        {
                var constants = new Set<shader_func_param>();
                for (var i = 0; i < this.sequence.length; i++) {
                        var call = this.sequence[i];
                        call.get_required_constants().forEach(function (constant: shader_func_param, v, s)
                        {
                                constants.add(constant);
                        })
                }
                return constants;
        }

        public get_required_inputs(): Set<shader_func_param>
        {
                var ins = new Set<shader_func_param>();
                for (var i = 0; i < this.sequence.length; i++) {
                        var call = this.sequence[i];
                        call.get_required_shader_input().forEach(function (input: shader_func_param, v, s)
                        {
                                ins.add(input);
                        })
                }
                return ins;
        }

        public get_required_functions(lib: shader_lib): Array<shader_function>
        {
                var funcs = new Array<shader_function>();
                var loaded_funcs = new Set<string>();

                for (var i = 0; i < this.sequence.length; i++) {
                        var call = this.sequence[i];
                        call.get_required_functions(lib).forEach(function (fs: shader_function, v, s)
                        {
                                if (!loaded_funcs.has(fs.get_function_name())) {
                                        funcs.push(fs);
                                        loaded_funcs.add(fs.get_function_name());
                                }
                        })
                }
                return funcs;
        }

        public get_outputs(): Set<shader_func_ret>
        {
                var outs = new Set<shader_func_ret>();
                for (var i = 0; i < this.sequence.length; i++) {
                        outs.add(this.sequence[i].get_ret());
                }
                return outs;
        }

        private gen_call_string(call: shader_call): string
        {
                var call_str: string = call.get_function_name() + "(";
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

        public gen_glsl_main(lib: shader_lib): string
        {
                // Semantic checkings.
                for (var i = 0; i < this.sequence.length; i++) {
                        this.sequence[i].check_param_binding();
                }

                var const_section: string = "";
                var input_section: string = "";
                var output_section: string = "";
                var defn_section: string = "";
                var main_section: string = "";

                // Pull in constants.
                var constants: Set<shader_func_param> = this.get_required_constants();
                constants.forEach(function (constant: shader_func_param, v, s)
                {
                        var info = shader_constant_var_info(constant);
                        const_section += "uniform " + info[1] + " " + info[0] + ";" + "\n";
                });

                // Pull in inputs.
                var inputs: Set<shader_func_param> = this.get_required_inputs();
                inputs.forEach(function (input: shader_func_param, v, s)
                {
                        var info = shader_input_var_info(input);
                        const_section += "varying " + info[1] + " " + info[0] + ";" + "\n";
                });

                // Pull in outputs.
                var outputs: Set<shader_func_ret> = this.get_outputs();
                outputs.forEach(function (output: shader_func_ret, v, s)
                {
                        var info = shader_output_var_info(output);
                        const_section += "varying " + info[1] + " " + info[0] + ";" + "\n";
                });

                // Pull in function definitions.
                var funcs: Array<shader_function> = this.get_required_functions(lib);
                for (var i = 0; i < funcs.length; i++) {
                        defn_section += funcs[i].get_definition() + "\n";
                }

                // Construct the main section.
                main_section += `
                        void main()
                        {`
                for (var i = 0; i < this.sequence.length; i++) {
                        var info = shader_output_var_info(this.sequence[i].get_ret());
                        main_section += info[0] + "=" + this.gen_call_string(this.sequence[i]) + ";" + "\n";
                }
                main_section += `
                        }`;

                return const_section + input_section + output_section + defn_section + main_section;
        }
}


function shade_gen_builtin_library(): shader_lib
{
        var lib = new shader_lib();

        var codeinfo: [string, string, string];
        var func: shader_function;

        // vec4 albedodiff(vec3 albedo, float metalness)        
        codeinfo = shader_code_from_ext_script("sl_albedodiff");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.albedo);
        func.add_param(shader_func_param.metalness);
        func.set_ret(shader_func_ret.albedo);
        lib.add_function(func);

        // vec4 albedodiff(vec3 albedo, float metalness)   
        codeinfo = shader_code_from_ext_script("sl_albedospec");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.albedo);
        func.add_param(shader_func_param.metalness);
        func.set_ret(shader_func_ret.albedo);
        lib.add_function(func);

        // vec4 brdfambientglslt(vec3 albedo, vec3 ambient)        
        codeinfo = shader_code_from_ext_script("sl_brdfambient");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.albedo);
        func.add_param(shader_func_param.ambient);
        func.set_ret(shader_func_ret.color_hdr_ambi);
        lib.add_function(func);

        // vec4 brdfblinnphong(vec3 i, vec3 n, vec3 o, vec3 irrad, vec3 albedo, float alpha)
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

        // vec4 brdfcolor(vec3 albedo)
        codeinfo = shader_code_from_ext_script("sl_brdfcolor");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.albedo);
        func.set_ret(shader_func_ret.color_hdr);
        lib.add_function(func);

        // vec4 brdfcooktorr(vec3 i, vec3 n, vec3 o, vec3 irrad, vec3 albedo, float beta, float ior)
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

        // vec4 brdfdepth(vec3 p, float maxdepth)
        codeinfo = shader_code_from_ext_script("sl_brdfdepth");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.position);
        func.add_param(shader_func_param.depth_max);
        func.set_ret(shader_func_ret.color_hdr);
        lib.add_function(func);

        // vec4 brdfdirection(vec3 unit_dir)
        codeinfo = shader_code_from_ext_script("sl_brdfdirection");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.position);
        func.add_param(shader_func_param.depth_max);
        func.set_ret(shader_func_ret.color_hdr);
        lib.add_function(func);

        // vec4 brdflambert(vec3 i, vec3 n, vec3 irrad, vec3 albedo)
        codeinfo = shader_code_from_ext_script("sl_brdflambert");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.incident);
        func.add_param(shader_func_param.normal);
        func.add_param(shader_func_param.irrad);
        func.add_param(shader_func_param.albedo);
        func.set_ret(shader_func_ret.color_hdr_diff);
        lib.add_function(func);

        // vec4 brdfnormal(vec3 unit_norm)
        codeinfo = shader_code_from_ext_script("sl_brdfnormal");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.normal);
        func.set_ret(shader_func_ret.color_hdr);
        lib.add_function(func);

        // vec4 brdforennayar(vec3 i, vec3 n, vec3 o, vec3 irrad, vec3 albedo, float sig_gradi)
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

        // vec4 brdfphong(vec3 i, vec3 n, vec3 o, vec3 irrad, vec3 albedo, float alpha)
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

        // vec4 brdfposition(vec3 p, vec3 max)
        codeinfo = shader_code_from_ext_script("sl_brdfposition");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.position);
        func.add_param(shader_func_param.position_max);
        func.set_ret(shader_func_ret.color_hdr);
        lib.add_function(func);

        // vec4 brdfwhite()
        codeinfo = shader_code_from_ext_script("sl_brdfwhite");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.set_ret(shader_func_ret.color_hdr);
        lib.add_function(func);

        // vec4 brdfadd3(vec4 diff, vec4 spec, vec4 ambient)
        codeinfo = shader_code_from_ext_script("sl_brdfadd3");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.color_hdr_diff);
        func.add_param(shader_func_param.color_hdr_spec);
        func.add_param(shader_func_param.color_hdr_ambi);
        func.set_ret(shader_func_ret.color_hdr);
        lib.add_function(func);

        // vec3 emergent(vec3 hit_point)
        codeinfo = shader_code_from_ext_script("sl_emergent");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.position);
        func.set_ret(shader_func_ret.emergent);
        lib.add_function(func);

        // vec3 incident(vec3 light_pos, vec3 hit_point)
        codeinfo = shader_code_from_ext_script("sl_incident");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.lightpos);
        func.add_param(shader_func_param.position);
        func.set_ret(shader_func_ret.emergent);
        lib.add_function(func);

        // vec3 irradpoint(vec3 ray, vec3 inten)
        codeinfo = shader_code_from_ext_script("sl_irradpoint");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.incident);
        func.add_param(shader_func_param.lightinten);
        func.set_ret(shader_func_ret.irrad);
        lib.add_function(func);

        // vec3 irradspot(vec3 d, vec3 ray, vec3 inten, float exp, float umbrella)
        codeinfo = shader_code_from_ext_script("sl_irradspot");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.lightdir);
        func.add_param(shader_func_param.incident);
        func.add_param(shader_func_param.lightinten);
        func.add_param(shader_func_param.exp);
        func.add_param(shader_func_param.umbrella);
        func.set_ret(shader_func_ret.irrad);
        lib.add_function(func);

        // vec3 irradsun(vec3 d, vec3 inten)
        codeinfo = shader_code_from_ext_script("sl_irradsun");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.lightdir);
        func.add_param(shader_func_param.lightinten);
        func.set_ret(shader_func_ret.irrad);
        lib.add_function(func);

        // float loglum(vec3 color)
        codeinfo = shader_code_from_ext_script("sl_loglum");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.color_hdr);
        func.set_ret(shader_func_ret.color_hdr);
        lib.add_function(func);

        // vec3 texhdrcolor(sampler2D tex, vec2 tex_coord)
        codeinfo = shader_code_from_ext_script("sl_texhdrcolor");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.hdr_sampler);
        func.add_param(shader_func_param.texcoord);
        func.set_ret(shader_func_ret.color_hdr);
        lib.add_function(func);

        // vec3 tonemapacesglslt(vec3 color, float adapted_lum)
        codeinfo = shader_code_from_ext_script("sl_tonemapaces");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.color_hdr);
        func.add_param(shader_func_param.adapted_lum);
        func.set_ret(shader_func_ret.color_ldr);
        lib.add_function(func);

        // vec3 tonemapexp(vec3 color, float adapted_lum) 
        codeinfo = shader_code_from_ext_script("sl_tonemapexp");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.color_hdr);
        func.add_param(shader_func_param.adapted_lum);
        func.set_ret(shader_func_ret.color_ldr);
        lib.add_function(func);

        // vec3 tonemapfilmic(vec3 color, float adapted_lum)
        codeinfo = shader_code_from_ext_script("sl_tonemapfilmic");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.color_hdr);
        func.add_param(shader_func_param.adapted_lum);
        func.set_ret(shader_func_ret.color_ldr);
        lib.add_function(func);

        // vec3 tonemapreinhard(vec3 color, float adapted_lum)
        codeinfo = shader_code_from_ext_script("sl_tonemapreinhard");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.color_hdr);
        func.add_param(shader_func_param.adapted_lum);
        func.set_ret(shader_func_ret.color_ldr);
        lib.add_function(func);

        // vec3 vec3normalize(vec3 n)
        codeinfo = shader_code_from_ext_script("sl_vec3normalize");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.normal);
        func.set_ret(shader_func_ret.normal);
        lib.add_function(func);

        // vec3 vec3modelview(mat4 t, vec3 v)
        codeinfo = shader_code_from_ext_script("sl_vec3modelview");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.t_modelview);
        func.add_param(shader_func_param.position);
        func.set_ret(shader_func_ret.normal);
        lib.add_function(func);

        // vec3 vec3nmodelview(mat4 t, vec3 v)
        codeinfo = shader_code_from_ext_script("sl_vec3nmodelview");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.t_nmodelview);
        func.add_param(shader_func_param.normal);
        func.set_ret(shader_func_ret.normal);
        lib.add_function(func);

        // vec3 vec3proj(mat4 t, vec3 p)
        codeinfo = shader_code_from_ext_script("sl_vec3proj");
        func = new shader_function(codeinfo[2], codeinfo[0]);
        func.add_param(shader_func_param.t_proj);
        func.add_param(shader_func_param.position);
        func.set_ret(shader_func_ret.normal);
        lib.add_function(func);

        lib.check_functions();
        return lib;
}

var g_shader_lib = shade_gen_builtin_library();

function shader_get_builtin_library(): shader_lib
{
        return g_shader_lib;
}