
/// <reference path="typing/webgl2.d.ts" />
/// <reference path="gl.ts" />


function create_shader_from_code(gl: WebGLRenderingContext, code: string, ext: string): WebGLShader
{
        var shader;

        switch (ext) {
                case "glslf":
                        shader = gl.createShader(WebGL2RenderingContext.FRAGMENT_SHADER);
                        break;
                case "glslv":
                        shader = gl.createShader(WebGL2RenderingContext.VERTEX_SHADER);
                        break;
                default:
                        alert("Incorrect extension " + ext);
                        return null;
        }

        gl.shaderSource(shader, code);
        gl.compileShader(shader);

        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
                alert(gl.getShaderInfoLog(shader));
                return null;
        }

        return shader;
}

function create_shader_from_script_tag(gl: WebGLRenderingContext, sid: string): WebGLShader
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

        return create_shader_from_code(gl, code, ext);
}

function last_after(s: string, stor: string): string
{
        var c = s.split(stor);
        return c[c.length - 1];
}

function create_shader_from_ext_script(gl: WebGLRenderingContext, sid: string): WebGLShader
{
        var path = (<HTMLScriptElement>document.getElementById(sid)).src;
        if (!path) {
                alert("Shader script tag doesn't contain the source path (shouldn't import externally?).");
                return null;
        }

        var filename: string = last_after(path, "/");
        var ext: string = last_after(filename, ".");
        var varname:string = filename.replace(".", "");
        var code: string;
        try {
                code = eval(varname);
        } catch (e) {
                alert("Shader source file doesn't contain the string variable: " + varname + " or the shader script hasn't got loaded properly.");
                return null;
        }

        return create_shader_from_code(gl, code, ext);
}


function create_shader_program(vid: string, fid: string, is_ext: boolean = true): WebGLShader
{
        var gl = gl_rendering_context();

        var fs = is_ext ? create_shader_from_ext_script(gl, fid) : create_shader_from_script_tag(gl, fid);
        var vs = is_ext ? create_shader_from_ext_script(gl, vid) : create_shader_from_script_tag(gl, vid);
        var prog = gl.createProgram();

        gl.attachShader(prog, vs);
        gl.attachShader(prog, fs);
        gl.linkProgram(prog);

        if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
                alert("Could not initialise shaders");
        }

        return prog;
}
