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
//# sourceMappingURL=shader.js.map