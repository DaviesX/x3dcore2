class gl_program_info {
    constructor(prog) {
        this.uni = new Map();
        this.input = new Map();
        this.prog = prog;
    }
    get_uniform_location(gl, s) {
        var loc = this.uni.get(s);
        if (loc == null) {
            loc = gl.getUniformLocation(this.prog, s);
            if (loc == null)
                throw new Error("Uniform " + s + " doesn't exist in program " + this.prog);
            else {
                this.uni.set(s, loc);
                return loc;
            }
        }
        else
            return loc;
    }
    get_input_location(gl, s) {
        var loc = this.input.get(s);
        if (loc == null) {
            loc = gl.getAttribLocation(this.prog, s);
            if (loc == null)
                throw new Error("Attribute " + s + " doesn't exist in program " + this.prog);
            else {
                this.uni.set(s, loc);
                return loc;
            }
        }
        else
            return loc;
    }
}
class ibo_info {
    constructor(ibo) {
        this.bitsize = 0;
        this.length = 0;
        this.ibo = ibo;
    }
}
var gl_elm_type;
(function (gl_elm_type) {
    gl_elm_type[gl_elm_type["int"] = 0] = "int";
    gl_elm_type[gl_elm_type["short"] = 1] = "short";
    gl_elm_type[gl_elm_type["byte"] = 2] = "byte";
    gl_elm_type[gl_elm_type["float"] = 3] = "float";
})(gl_elm_type || (gl_elm_type = {}));
class gl_vbo_info {
    constructor(vbo) {
        this.grpsize = 0;
        this.length = 0;
        this.vbo = vbo;
    }
}
class gl_fbo_info {
    constructor(fbo) {
        this.fbo = fbo;
    }
}
class gl_raster_backend {
    constructor(canvas) {
        this.gl = null;
        this.fbo = new Map();
        this.vbo = new Map();
        this.ibo = new Map();
        this.program = new Map();
        this.vs = new Map();
        this.fs = new Map();
        this.sampler = new Map();
        this.curr_buf_location = 0;
        this.curr_program = 0;
        this.uuid = 1;
        typeof (canvas);
        try {
            this.gl = canvas.getContext("experimental-webgl");
        }
        catch (e) {
            alert("Could not initialise WebGL, sorry :-(");
            throw e;
        }
        if (this.gl == null) {
            throw new Error("Could not initialise WebGL, sorry :-(");
        }
        this.gl.enable(WebGLRenderingContext.DEPTH_TEST);
    }
    set_viewport_dimension(x, y, w, h) {
        this.gl.viewport(0, 0, w, h);
    }
    frame_buf_get_default() {
        return 0;
    }
    frame_buf_set_background(loc, r, g, b, a) {
        this.gl.clearColor(r, g, b, a);
    }
    frame_buf_set_depth(loc, depth) {
        this.gl.clearDepth(depth);
    }
    frame_buf_fill(loc, color, depth) {
        var flag = 0;
        if (color)
            flag |= WebGLRenderingContext.COLOR_BUFFER_BIT;
        if (depth)
            flag |= WebGLRenderingContext.DEPTH_BUFFER_BIT;
        this.gl.clear(flag);
    }
    attri_buf_create() {
        this.vbo.set(this.uuid, new gl_vbo_info(this.gl.createBuffer()));
        return this.uuid++;
    }
    attri_buf_delete(loc) {
        var info = this.vbo.get(loc);
        if (info != null) {
            this.gl.deleteBuffer(info.vbo);
            this.vbo.delete(loc);
        }
    }
    bind_buffer(type, location, buf) {
        if (location != this.curr_buf_location) {
            this.curr_buf_location = location;
            this.gl.bindBuffer(type, buf);
        }
    }
    attri_buf_writef32(loc, data, grp_size, onchip) {
        var info = this.vbo.get(loc);
        if (info == null)
            throw new Error("Buffer location " + loc + " doesn't exist, as an attribute buffer");
        info.grpsize = grp_size;
        info.elmtype = gl_elm_type.float;
        info.length = data.length;
        this.bind_buffer(WebGLRenderingContext.ARRAY_BUFFER, loc, info.vbo);
        this.gl.bufferData(WebGLRenderingContext.ARRAY_BUFFER, data, onchip ? WebGLRenderingContext.STATIC_DRAW : WebGLRenderingContext.DYNAMIC_DRAW);
    }
    attri_array_create() {
        throw new Error("Current GL version doesn't have VAO support.");
    }
    attri_array_delete(loc) {
        throw new Error("Current GL version doesn't have VAO support.");
    }
    attri_array_add(loc, attri_buf) {
        throw new Error("Current GL version doesn't have VAO support.");
    }
    index_buf_create() {
        this.ibo.set(this.uuid, new ibo_info(this.gl.createBuffer()));
        return this.uuid++;
    }
    index_buf_delete(loc) {
        var info = this.ibo.get(loc);
        if (info != null) {
            this.gl.deleteBuffer(info.ibo);
            this.ibo.delete(loc);
        }
    }
    index_buf_write_u16(loc, data, onchip) {
        var info = this.ibo.get(loc);
        if (info == null)
            throw new Error("Buffer location " + loc + " doesn't exist, as an index buffer");
        info.bitsize = 16;
        info.length = data.length;
        this.bind_buffer(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, loc, info.ibo);
        this.gl.bufferData(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, data, onchip ? WebGLRenderingContext.STATIC_DRAW : WebGLRenderingContext.DYNAMIC_DRAW);
    }
    index_buf_write_u32(loc, data, onchip) {
        throw new Error("Current GL version doesn't support Uint32 indexing.");
    }
    gen_shader(code, type) {
        var shader = this.gl.createShader(type);
        this.gl.shaderSource(shader, code);
        this.gl.compileShader(shader);
        if (!this.gl.getShaderParameter(shader, WebGLRenderingContext.COMPILE_STATUS)) {
            throw new Error("Failed to compile shader with code: " + code
                + ". Cause: " + this.gl.getShaderInfoLog(shader).toString());
        }
        return shader;
    }
    shader_vertex_create(code) {
        var shader = this.gen_shader(code, WebGLRenderingContext.VERTEX_SHADER);
        this.vs.set(this.uuid, shader);
        return this.uuid++;
    }
    shader_fragment_create(code) {
        var shader = this.gen_shader(code, WebGLRenderingContext.FRAGMENT_SHADER);
        this.fs.set(this.uuid, shader);
        return this.uuid++;
    }
    shader_delete(loc) {
        var shader = this.vs.get(loc);
        if (shader == null)
            shader = this.fs.get(loc);
        if (shader != null) {
            this.gl.deleteShader(shader);
            this.vs.delete(loc);
            this.fs.delete(loc);
        }
    }
    program_create() {
        this.program.set(this.uuid, new gl_program_info(this.gl.createProgram()));
        return this.uuid++;
    }
    program_delete(loc) {
        var info = this.program.get(loc);
        if (info != null) {
            this.gl.deleteProgram(info.prog);
            this.program.delete(loc);
        }
    }
    program_attach_shader(prog, shader) {
        var info = this.program.get(prog);
        if (info == null)
            throw new Error("Program " + prog + " doesn't exist.");
        var s = this.vs.get(shader);
        if (s == null) {
            s = this.fs.get(shader);
            if (s == null)
                throw new Error("Shader " + shader + " doesn't exist.");
        }
        this.gl.attachShader(info.prog, s);
    }
    program_link(loc) {
        var info = this.program.get(loc);
        if (info == null)
            throw new Error("Program " + loc + " doesn't exist.");
        this.gl.linkProgram(info.prog);
        if (!this.gl.getProgramParameter(info.prog, WebGLRenderingContext.LINK_STATUS)) {
            throw new Error("Failed to link shader " + loc + ". Cause: " + this.gl.getProgramInfoLog(info.prog));
        }
    }
    program_use(loc) {
        var info = this.program.get(loc);
        if (info == null)
            throw new Error("Program " + loc + " doesn't exist.");
        if (loc != this.curr_program) {
            this.curr_program = loc;
            this.gl.useProgram(info.prog);
        }
    }
    program_assign_uniform(loc, var_name, data, type) {
        var prog = this.program.get(loc);
        if (prog == null)
            throw new Error("Program " + loc + " doesn't exist.");
        var uniloc = prog.get_uniform_location(this.gl, var_name);
        this.program_use(loc);
        switch (type) {
            case "float":
                this.gl.uniform1f(uniloc, data[0]);
                break;
            case "vec2":
                this.gl.uniform2fv(uniloc, data);
                break;
            case "vec3":
                this.gl.uniform3fv(uniloc, data);
                break;
            case "vec4":
                this.gl.uniform4fv(uniloc, data);
                break;
            case "mat2":
                this.gl.uniformMatrix2fv(uniloc, false, data);
                break;
            case "mat3":
                this.gl.uniformMatrix3fv(uniloc, false, data);
                break;
            case "mat4":
                this.gl.uniformMatrix4fv(uniloc, false, data);
                break;
            default:
                throw new Error("Unknown type " + type + ".");
        }
    }
    program_assign_input(loc, var_name, buf) {
        var prog = this.program.get(loc);
        if (prog == null)
            throw new Error("Program " + loc + " doesn't exist.");
        var input_loc = prog.get_input_location(this.gl, var_name);
        var info = this.vbo.get(buf);
        if (info == null)
            throw new Error("Attribute buffer " + buf + " doesn't exist.");
        this.bind_buffer(WebGLRenderingContext.ARRAY_BUFFER, buf, info.vbo);
        var type;
        switch (info.elmtype) {
            case gl_elm_type.float:
                type = WebGLRenderingContext.FLOAT;
                break;
            case gl_elm_type.byte:
                type = WebGLRenderingContext.BYTE;
                break;
            case gl_elm_type.short:
                type = WebGLRenderingContext.SHORT;
                break;
            case gl_elm_type.int:
                type = WebGLRenderingContext.INT;
                break;
            default:
                throw new Error("Unkown element type " + info.elmtype + ".");
        }
        this.gl.enableVertexAttribArray(input_loc);
        this.gl.vertexAttribPointer(input_loc, info.grpsize, type, false, 0, 0);
    }
    draw_indexed_triangles(frame, prog, index_buf, offset, length) {
        this.program_use(prog);
        var info = this.ibo.get(index_buf);
        if (info == null)
            throw new Error("Index buffer location " + index_buf + " doesn't exist.");
        this.bind_buffer(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, index_buf, info.ibo);
        var type = info.bitsize == 16 ? WebGLRenderingContext.UNSIGNED_SHORT : WebGLRenderingContext.UNSIGNED_INT;
        this.gl.drawElements(WebGLRenderingContext.TRIANGLES, length, type, offset);
    }
    draw_triangles(frame, prog, offset, length) {
        this.program_use(prog);
        this.gl.drawArrays(WebGLRenderingContext.TRIANGLES, offset, length);
    }
    draw_points(frame, prog, offset, length) {
        this.program_use(prog);
        this.gl.drawArrays(WebGLRenderingContext.POINTS, offset, length);
    }
}
function gl_create_backend_from_canvas(canvas) {
    return new gl_raster_backend(canvas);
}
//# sourceMappingURL=gl.js.map