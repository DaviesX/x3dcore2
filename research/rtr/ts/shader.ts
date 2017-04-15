
/// <reference path="typing/webgl2.d.ts" />
/// <reference path="gl.ts" />


function create_shader_from_code(backend: if_raster_backend, code: string, ext: string): shader_location
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

function create_shader_from_script_tag(backend: if_raster_backend, sid: string): shader_location
{
        var shaderScript = <HTMLScriptElement>document.getElementById(sid);
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
        } else if (shaderScript.type == "x-shader/x-vertex") {
                ext = "glslv";
        } else {
                return null;
        }

        return create_shader_from_code(backend, code, ext);
}

function last_after(s: string, stor: string): string
{
        var c = s.split(stor);
        return c[c.length - 1];
}

function create_shader_from_ext_script(backend: if_raster_backend, sid: string): shader_location
{
        var path = (<HTMLScriptElement>document.getElementById(sid)).src;
        if (!path) {
                alert("Shader script tag doesn't contain the source path (shouldn't import externally?).");
                return null;
        }

        var filename: string = last_after(path, "/");
        var ext: string = last_after(filename, ".");
        var varname: string = filename.replace(".", "");
        var code: string;
        try {
                code = eval(varname);
        } catch (e) {
                alert("Shader source file doesn't contain the string variable: " + varname + " or the shader script hasn't got loaded properly.");
                return null;
        }

        return create_shader_from_code(backend, code, ext);
}


function create_shader_program(backend: if_raster_backend, vid: string, fid: string, is_ext: boolean = true): program_location
{
        var fs: shader_location = is_ext ? create_shader_from_ext_script(backend, fid) : create_shader_from_script_tag(backend, fid);
        var vs: shader_location = is_ext ? create_shader_from_ext_script(backend, vid) : create_shader_from_script_tag(backend, vid);
        var prog: program_location = backend.program_create();

        backend.program_attach_shader(prog, vs);
        backend.program_attach_shader(prog, fs);
        backend.program_link(prog);
        return prog;
}

enum shader_input
{
        position,
        normal,
        texcoord,
        tangent,
        index,

        hdr_color,
        ldr_color,
        depth
}

type shader_output = shader_input;

enum shader_constant
{
        mv_transform,
        proj_transform
}


function shader_io_validate(input: shader_input, output: shader_output): boolean
{
        if (input != output)
                throw new Error("Shader output is " + output + " but input requires " + input + ".");
        return true;
}

function shader_gen_var_info(input: shader_input, prefix: string): [string, string]
{
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

function shader_input_var_info(input: shader_input): [string, string]
{
        return shader_gen_var_info(input, "in_");
}

function shader_output_var_info(output: shader_output): [string, string]
{
        return shader_gen_var_info(output, "out_");
}

function shader_constant_var_info(constant: shader_constant): [string, string]
{
        var prefix: string = "const_";
        switch (constant) {
                case shader_constant.mv_transform:
                        return [prefix + "mv_trans", "mat4"];
                case shader_constant.proj_transform:
                        return [prefix + "proj_trans", "mat4"];
                default:
                        throw new Error("Unknown shader constant type " + constant + ".");
        }
}



class shader_function
{
        private func_name: string;
        private code: string;

        private input = new Map<shader_input, string>();
        private ordered_input = new Array<shader_input>();
        private output: shader_output = null;

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

        public add_input(param_name: string, type: shader_input): void
        {
                if (this.input.has(type))
                        throw new Error("Input type " + type + " has already existed.");
                this.ordered_input.push(type);
                this.input.set(type, param_name);
        }

        public set_output(type: shader_output): void
        {
                this.output = type;
        }

        public add_required_function(defn: string): void
        {
                this.func_required.add(defn);
        }

        public get_inputs(): Set<shader_input>
        {
                var ins = new Set<shader_input>();
                this.input.forEach(function (v: string, input: shader_input, m)
                {
                        ins.add(input);
                });
                return ins;
        }

        public get_ordered_inputs(): Array<shader_input>
        {
                return this.ordered_input;
        }

        public get_output(): shader_output
        {
                return this.output;
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

function gen_builtin_library(): shader_lib
{
        var lib = new shader_lib();
        lib.check_functions();
        return lib;
}

enum shader_input_binding
{
        constant,
        input,
        call
}

class shader_call
{
        private func: shader_function;

        private in_call_binding = new Map<shader_input, shader_call>();
        private in_input_binding = new Map<shader_input, shader_input>();
        private in_const_binding = new Map<shader_input, shader_constant>();

        public bind_input_from_shader_input(input: shader_input, shader_in: shader_input)
        {
                this.in_input_binding.set(input, shader_in);
        }

        public bind_input_from_constant(input: shader_input, constant: shader_constant): void
        {
                this.in_const_binding.set(input, constant);
        }

        public bind_input_from_call(input: shader_input, func: shader_call): void
        {
                shader_io_validate(input, func.func.get_output());
                this.in_call_binding.set(input, func);
        }

        public get_input_binding(input: shader_input): [shader_input_binding, any]
        {
                if (this.in_call_binding.has(input))
                        return [shader_input_binding.call, this.in_call_binding.get(input)];
                else if (this.in_input_binding.has(input))
                        return [shader_input_binding.input, this.in_input_binding.get(input)];
                else if (this.in_const_binding.has(input))
                        return [shader_input_binding.constant, this.in_const_binding.get(input)];
                else
                        throw new Error("Input " + input + " doesn't have any binding.");
        }

        public get_required_shader_input(): Set<shader_input>
        {
                var ins = new Set<shader_input>();
                this.in_input_binding.forEach(function (input: shader_input, type: shader_input, m)
                {
                        ins.add(input);
                });
                return ins;
        }

        public get_required_constants(): Set<shader_constant>
        {
                var constants = new Set<shader_constant>();
                this.in_const_binding.forEach(function (constanst: shader_constant, type: shader_input, m)
                {
                        constants.add(constanst);
                });
                return constants;
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

        public get_ordered_inputs(): Array<shader_input>
        {
                return this.func.get_ordered_inputs();
        }

        public get_output(): shader_output
        {
                return this.func.get_output();
        }

        public get_function_name(): string
        {
                return this.func.get_function_name();
        }

        public check_input_binding(): void
        {
                var this_: shader_call = this;

                this.func.get_inputs().forEach(function (input: shader_input, m)
                {
                        var bindings = new Array<string>();
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

        public get_required_constants(): Set<shader_constant>
        {
                var constants = new Set<shader_constant>();
                for (var i = 0; i < this.sequence.length; i++) {
                        var call = this.sequence[i];
                        call.get_required_constants().forEach(function (constant: shader_constant, v, s)
                        {
                                constants.add(constant);
                        })
                }
                return constants;
        }

        public get_required_inputs(): Set<shader_input>
        {
                var ins = new Set<shader_input>();
                for (var i = 0; i < this.sequence.length; i++) {
                        var call = this.sequence[i];
                        call.get_required_shader_input().forEach(function (input: shader_input, v, s)
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

        public get_outputs(): Set<shader_output>
        {
                var outs = new Set<shader_output>();
                for (var i = 0; i < this.sequence.length; i++) {
                        outs.add(this.sequence[i].get_output());
                }
                return outs;
        }

        private gen_call_string(func: shader_call): string
        {
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

        public gen_glsl_main(lib: shader_lib): string
        {
                // Semantic checkings.
                for (var i = 0; i < this.sequence.length; i++) {
                        this.sequence[i].check_input_binding();
                }

                var const_section: string = "";
                var input_section: string = "";
                var output_section: string = "";
                var defn_section: string = "";
                var main_section: string = "";

                // Pull in constants.
                var constants: Set<shader_constant> = this.get_required_constants();
                constants.forEach(function (constant: shader_constant, v, s)
                {
                        var info = shader_constant_var_info(constant);
                        const_section += "uniform " + info[1] + " " + info[0] + ";" + "\n";
                });

                // Pull in inputs.
                var inputs: Set<shader_input> = this.get_required_inputs();
                inputs.forEach(function (input: shader_input, v, s)
                {
                        var info = shader_input_var_info(input);
                        const_section += "varying " + info[1] + " " + info[0] + ";" + "\n";
                });

                // Pull in outputs.
                var outputs: Set<shader_output> = this.get_outputs();
                outputs.forEach(function (output: shader_output, v, s)
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
                        var info = shader_output_var_info(this.sequence[i].get_output());
                        main_section += info[0] + "=" + this.gen_call_string(this.sequence[i]) + ";" + "\n";
                }
                main_section += `
                        }`;

                return const_section + input_section + output_section + defn_section + main_section;
        }
}
