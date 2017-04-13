
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
