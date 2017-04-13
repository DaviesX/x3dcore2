
type buffer_location = number;
type shader_location = number;
type uniform_location = number;
type input_location = number;
type program_location = number;

interface if_raster_backend
{
        set_viewport_dimension(x: number, y: number, w: number, h: number): void;

        frame_buf_get_default(): buffer_location;
        frame_buf_set_background(loc: buffer_location, r: number, g: number, b: number, a: number): void;
        frame_buf_set_depth(loc: buffer_location, depth: number): void;
        frame_buf_fill(loc: buffer_location, color: boolean, depth: boolean): void;

        attri_buf_create(): buffer_location;
        attri_buf_delete(loc: buffer_location): void;
        attri_buf_writef32(loc: buffer_location, data: Float32Array, grp_size: number, onchip: boolean): void;

        attri_array_create(): buffer_location;
        attri_array_delete(loc: buffer_location): void;
        attri_array_add(loc: buffer_location, attri_buf: buffer_location): void;

        index_buf_create(): buffer_location;
        index_buf_delete(loc: buffer_location): void;
        index_buf_write_u16(loc: buffer_location, data: Uint16Array, onchip: boolean): void;
        index_buf_write_u32(loc: buffer_location, data: Uint32Array, onchip: boolean): void;

        shader_vertex_create(code: string): shader_location;
        shader_fragment_create(code: string): shader_location;
        shader_delete(loc: shader_location): void;

        program_create(): program_location;
        program_delete(loc: program_location): void;
        program_attach_shader(prog: program_location, loc: shader_location): void;
        program_link(loc: program_location): void;
        program_assign_uniform(loc: program_location, var_name: string, data: Array<number>, type: string): void;
        program_assign_input(loc: program_location, input: string, buf: buffer_location): void;

        draw_indexed_triangles(frame: buffer_location, prog: program_location, index_buf: buffer_location, offset: number, length: number): void
        draw_triangles(frame: buffer_location, prog: program_location, offset: number, length: number): void
        draw_points(frame: buffer_location, prog: program_location, offset: number, length: number): void
}


// GL backend implementation.
class gl_program_info
{
        public prog: WebGLProgram;
        public uni = new Map<string, WebGLUniformLocation>();
        public input = new Map<string, input_location>();

        constructor(prog: WebGLProgram)
        {
                this.prog = prog;
        }

        public get_uniform_location(gl: WebGLRenderingContext, s: string): WebGLUniformLocation
        {
                var loc = this.uni.get(s);
                if (loc == null) {
                        loc = gl.getUniformLocation(this.prog, s);
                        if (loc == null)
                                throw new Error("Uniform " + s + " doesn't exist in program " + this.prog);
                        else {
                                this.uni.set(s, loc);
                                return loc;
                        }
                } else
                        return loc;
        }

        public get_input_location(gl: WebGLRenderingContext, s: string): input_location
        {
                var loc = this.input.get(s);
                if (loc == null) {
                        loc = gl.getAttribLocation(this.prog, s);
                        if (loc == null)
                                throw new Error("Attribute " + s + " doesn't exist in program " + this.prog);
                        else {
                                this.uni.set(s, loc);
                                return loc;
                        }
                } else
                        return loc;
        }
}

class ibo_info
{
        public ibo: WebGLBuffer;
        public bitsize: number = 0;
        public length: number = 0;

        constructor(ibo: WebGLBuffer)
        {
                this.ibo = ibo;
        }
}

enum gl_elm_type
{
        int,
        short,
        byte,
        float
}

class gl_vbo_info
{
        public vbo: WebGLBuffer;
        public grpsize: number = 0;
        public elmtype: gl_elm_type;
        public length: number = 0;

        constructor(vbo: WebGLBuffer)
        {
                this.vbo = vbo;
        }
}

class gl_fbo_info
{
        public fbo: WebGLFramebuffer;

        constructor(fbo: WebGLFramebuffer)
        {
                this.fbo = fbo;
        }
}

class gl_raster_backend implements if_raster_backend
{
        private gl: WebGLRenderingContext = null;

        private fbo = new Map<number, gl_fbo_info>();
        private vbo = new Map<number, gl_vbo_info>();
        private ibo = new Map<number, ibo_info>();

        private program = new Map<number, gl_program_info>();
        private vs = new Map<number, WebGLShader>();
        private fs = new Map<number, WebGLShader>();

        private sampler = new Map<number, WebGLSampler>();

        private curr_buf_location: number = 0;
        private curr_program: number = 0;

        private uuid: number = 1;

        constructor(canvas: HTMLCanvasElement)
        {
                typeof (canvas);
                try {
                        this.gl = canvas.getContext("experimental-webgl");
                } catch (e) {
                        alert("Could not initialise WebGL, sorry :-(");
                        throw e;
                }
                if (this.gl == null) {
                        throw new Error("Could not initialise WebGL, sorry :-(");
                }


                this.gl.enable(WebGLRenderingContext.DEPTH_TEST);
        }

        public set_viewport_dimension(x: number, y: number, w: number, h: number): void
        {
                this.gl.viewport(0, 0, w, h);
        }

        public frame_buf_get_default(): buffer_location
        {
                return 0;
        }

        public frame_buf_set_background(loc: buffer_location, r: number, g: number, b: number, a: number): void
        {
                this.gl.clearColor(r, g, b, a);
        }

        public frame_buf_set_depth(loc: buffer_location, depth: number): void
        {
                this.gl.clearDepth(depth);
        }

        public frame_buf_fill(loc: buffer_location, color: boolean, depth: boolean): void
        {
                var flag: number = 0;
                if (color)
                        flag |= WebGLRenderingContext.COLOR_BUFFER_BIT;
                if (depth)
                        flag |= WebGLRenderingContext.DEPTH_BUFFER_BIT;
                this.gl.clear(flag);
        }

        public attri_buf_create(): buffer_location
        {
                this.vbo.set(this.uuid, new gl_vbo_info(this.gl.createBuffer()));
                return this.uuid++;
        }

        public attri_buf_delete(loc: buffer_location): void
        {
                var info: gl_vbo_info = this.vbo.get(loc);
                if (info != null) {
                        this.gl.deleteBuffer(info.vbo);
                        this.vbo.delete(loc);
                }
        }

        private bind_buffer(type: number, location: number, buf: WebGLBuffer): void
        {
                if (location != this.curr_buf_location) {
                        this.curr_buf_location = location;
                        this.gl.bindBuffer(type, buf);
                }
        }

        public attri_buf_writef32(loc: buffer_location, data: Float32Array, grp_size: number, onchip: boolean): void
        {
                var info: gl_vbo_info = this.vbo.get(loc);
                if (info == null)
                        throw new Error("Buffer location " + loc + " doesn't exist, as an attribute buffer");
                info.grpsize = grp_size;
                info.elmtype = gl_elm_type.float;
                info.length = data.length;
                this.bind_buffer(WebGLRenderingContext.ARRAY_BUFFER, loc, info.vbo);
                this.gl.bufferData(WebGLRenderingContext.ARRAY_BUFFER, data,
                        onchip ? WebGLRenderingContext.STATIC_DRAW : WebGLRenderingContext.DYNAMIC_DRAW);
        }

        public attri_array_create(): buffer_location
        {
                throw new Error("Current GL version doesn't have VAO support.");
        }

        public attri_array_delete(loc: number): buffer_location
        {
                throw new Error("Current GL version doesn't have VAO support.");
        }

        public attri_array_add(loc: buffer_location, attri_buf: buffer_location): void
        {
                throw new Error("Current GL version doesn't have VAO support.");
        }

        public index_buf_create(): buffer_location
        {
                this.ibo.set(this.uuid, new ibo_info(this.gl.createBuffer()));
                return this.uuid++;
        }

        public index_buf_delete(loc: buffer_location): void
        {
                var info: ibo_info = this.ibo.get(loc);
                if (info != null) {
                        this.gl.deleteBuffer(info.ibo);
                        this.ibo.delete(loc);
                }
        }

        public index_buf_write_u16(loc: buffer_location, data: Uint16Array, onchip: boolean): void
        {
                var info: ibo_info = this.ibo.get(loc);
                if (info == null)
                        throw new Error("Buffer location " + loc + " doesn't exist, as an index buffer");
                info.bitsize = 16;
                info.length = data.length;
                this.bind_buffer(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, loc, info.ibo);
                this.gl.bufferData(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, data,
                        onchip ? WebGLRenderingContext.STATIC_DRAW : WebGLRenderingContext.DYNAMIC_DRAW);
        }

        public index_buf_write_u32(loc: buffer_location, data: Uint32Array, onchip: boolean): void
        {
                throw new Error("Current GL version doesn't support Uint32 indexing.");
        }

        private gen_shader(code: string, type: number): WebGLShader
        {
                var shader = this.gl.createShader(type);
                this.gl.shaderSource(shader, code);
                this.gl.compileShader(shader);
                if (!this.gl.getShaderParameter(shader, WebGLRenderingContext.COMPILE_STATUS)) {
                        throw new Error("Failed to compile shader with code: " + code
                                + ". Cause: " + this.gl.getShaderInfoLog(shader).toString());
                }
                return shader;
        }

        public shader_vertex_create(code: string): shader_location
        {
                var shader = this.gen_shader(code, WebGLRenderingContext.VERTEX_SHADER);
                this.vs.set(this.uuid, shader);
                return this.uuid++;
        }

        public shader_fragment_create(code: string): shader_location
        {
                var shader = this.gen_shader(code, WebGLRenderingContext.FRAGMENT_SHADER);
                this.fs.set(this.uuid, shader);
                return this.uuid++;
        }

        public shader_delete(loc: shader_location): void
        {
                var shader = this.vs.get(loc);
                if (shader == null)
                        shader = this.fs.get(loc);
                if (shader != null) {
                        this.gl.deleteShader(shader);
                        this.vs.delete(loc);
                        this.fs.delete(loc);
                }
        }

        public program_create(): program_location
        {
                this.program.set(this.uuid, new gl_program_info(this.gl.createProgram()));
                return this.uuid++;
        }

        public program_delete(loc: program_location): void
        {
                var info = this.program.get(loc);
                if (info != null) {
                        this.gl.deleteProgram(info.prog);
                        this.program.delete(loc);
                }
        }

        public program_attach_shader(prog: program_location, shader: shader_location): void
        {
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

        public program_link(loc: program_location): void
        {
                var info = this.program.get(loc);
                if (info == null)
                        throw new Error("Program " + loc + " doesn't exist.");
                this.gl.linkProgram(info.prog);

                if (!this.gl.getProgramParameter(info.prog, WebGLRenderingContext.LINK_STATUS)) {
                        throw new Error("Failed to link shader " + loc + ". Cause: " + this.gl.getProgramInfoLog(info.prog));
                }
        }

        private program_use(loc: program_location): void
        {
                var info = this.program.get(loc);
                if (info == null)
                        throw new Error("Program " + loc + " doesn't exist.");

                if (loc != this.curr_program) {
                        this.curr_program = loc;
                        this.gl.useProgram(info.prog);
                }
        }

        public program_assign_uniform(loc: program_location, var_name: string, data: Array<number>, type: string): void
        {
                var prog: gl_program_info = this.program.get(loc);
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

        public program_assign_input(loc: program_location, var_name: string, buf: buffer_location): void
        {
                var prog: gl_program_info = this.program.get(loc);
                if (prog == null)
                        throw new Error("Program " + loc + " doesn't exist.");
                var input_loc = prog.get_input_location(this.gl, var_name);

                var info: gl_vbo_info = this.vbo.get(buf);
                if (info == null)
                        throw new Error("Attribute buffer " + buf + " doesn't exist.");
                this.bind_buffer(WebGLRenderingContext.ARRAY_BUFFER, buf, info.vbo);

                var type: number;
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

        public draw_indexed_triangles(frame: buffer_location, prog: program_location, index_buf: buffer_location, offset: number, length: number): void
        {
                this.program_use(prog);

                var info: ibo_info = this.ibo.get(index_buf);
                if (info == null)
                        throw new Error("Index buffer location " + index_buf + " doesn't exist.");
                this.bind_buffer(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, index_buf, info.ibo);
                var type = info.bitsize == 16 ? WebGLRenderingContext.UNSIGNED_SHORT : WebGLRenderingContext.UNSIGNED_INT;

                this.gl.drawElements(WebGLRenderingContext.TRIANGLES, length, type, offset);
        }

        public draw_triangles(frame: buffer_location, prog: program_location, offset: number, length: number): void
        {
                this.program_use(prog);
                this.gl.drawArrays(WebGLRenderingContext.TRIANGLES, offset, length);
        }

        public draw_points(frame: buffer_location, prog: program_location, offset: number, length: number): void
        {
                this.program_use(prog);
                this.gl.drawArrays(WebGLRenderingContext.POINTS, offset, length);
        }
}

function gl_create_backend_from_canvas(canvas: HTMLCanvasElement): if_raster_backend
{
        return new gl_raster_backend(canvas);
}