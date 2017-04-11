class trimesh {
    constructor() {
        this.vertices = new Array();
        this.texcoords = new Array();
        this.normals = new Array();
        this.indices = Array();
    }
    get_vertex_transform() {
        return this.global_trans;
    }
    get_vertices_f32() {
        var data = new Float32Array(this.vertices.length * 3);
        for (var v = 0; v < this.vertices.length; v++) {
            data[3 * v + 0] = this.vertices[v].x;
            data[3 * v + 1] = this.vertices[v].y;
            data[3 * v + 2] = this.vertices[v].z;
        }
        return data;
    }
    get_normals_f32() {
        var data = new Float32Array(this.normals.length * 3);
        for (var v = 0; v < this.normals.length; v++) {
            data[3 * v + 0] = this.normals[v].x;
            data[3 * v + 1] = this.normals[v].y;
            data[3 * v + 2] = this.normals[v].z;
        }
        return data;
    }
    get_texcoords_f32() {
        var data = new Float32Array(this.texcoords.length * 2);
        for (var v = 0; v < this.texcoords.length; v++) {
            data[3 * v + 0] = this.texcoords[v].x;
            data[3 * v + 1] = this.texcoords[v].y;
        }
        return data;
    }
    get_indices_u32() {
        return new Uint32Array(this.indices);
    }
    get_multi_indices_u16() {
        var m_idx = new Array();
        for (var i = 0; i < this.indices.length;) {
            var eff_len = this.indices.length % 0XFFF;
            var arr = new Uint16Array(eff_len);
            for (var j = 0; j < eff_len; j++)
                arr[j] = this.indices[i + j];
            m_idx.push(arr);
            i += eff_len;
        }
        return m_idx;
    }
    has_normal() {
        return this.normals.length != 0;
    }
    has_tex_coords() {
        return this.texcoords.length != 0;
    }
}
class mesh_vbo {
    constructor(mesh) {
        this.NUM_DATA_BUFS = 3;
        this.LOC_VERT = 0;
        this.LOC_NORM = 1;
        this.LOC_TEX = 2;
        this.LOC_IND_BASE = 3;
        this.buffers = new Array();
        this.num_idx_buffers = 0;
        this.HAS_UINT16_RESTRICTION = true;
        this.mesh = mesh;
        this.gl = gl_rendering_context();
        this.alloc();
    }
    idx_buf_count() {
        return this.HAS_UINT16_RESTRICTION ? Math.ceil(this.mesh.indices.length / 0XFFFF) : 1;
    }
    idx_buf_length(bloc) {
        return this.HAS_UINT16_RESTRICTION ? Math.min(0XFFFF, this.mesh.indices.length - bloc * 0XFFFF) : this.mesh.indices.length;
    }
    alloc() {
        for (var i = 0; i < this.NUM_DATA_BUFS; i++) {
            this.buffers[i] = this.gl.createBuffer();
        }
        this.num_idx_buffers = this.idx_buf_count();
        for (var i = 0; i < this.num_idx_buffers; i++) {
            this.buffers[this.LOC_IND_BASE + i] = this.gl.createBuffer();
        }
    }
    realloc() {
        var new_idx_count = this.idx_buf_count();
        if (new_idx_count > this.num_idx_buffers) {
            for (var i = this.num_idx_buffers; i < new_idx_count; i++) {
                this.buffers[this.LOC_IND_BASE + i] = this.gl.createBuffer();
            }
        }
    }
    upload() {
        this.realloc();
        var data_hint = this.mesh.is_static ? WebGLRenderingContext.STATIC_DRAW : WebGLRenderingContext.DYNAMIC_DRAW;
        this.gl.bindBuffer(WebGLRenderingContext.ARRAY_BUFFER, this.buffers[this.LOC_VERT]);
        this.gl.bufferData(WebGLRenderingContext.ARRAY_BUFFER, this.mesh.get_vertices_f32(), data_hint);
        if (this.mesh.has_normal()) {
            this.gl.bindBuffer(WebGLRenderingContext.ARRAY_BUFFER, this.buffers[this.LOC_NORM]);
            this.gl.bufferData(WebGLRenderingContext.ARRAY_BUFFER, this.mesh.get_normals_f32(), data_hint);
        }
        if (this.mesh.has_tex_coords()) {
            this.gl.bindBuffer(WebGLRenderingContext.ARRAY_BUFFER, this.buffers[this.LOC_TEX]);
            this.gl.bufferData(WebGLRenderingContext.ARRAY_BUFFER, this.mesh.get_texcoords_f32(), data_hint);
        }
        if (this.HAS_UINT16_RESTRICTION) {
            var m_idx = this.mesh.get_multi_indices_u16();
            for (var i = 0; i < this.num_idx_buffers; i++) {
                this.gl.bindBuffer(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, this.buffers[this.LOC_IND_BASE + i]);
                this.gl.bufferData(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, m_idx[i], data_hint);
            }
        }
        else {
            this.gl.bindBuffer(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, this.buffers[this.LOC_IND_BASE + 0]);
            this.gl.bufferData(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, this.mesh.get_indices_u32(), data_hint);
        }
    }
    unload() {
        for (var i = 0; i < this.buffers.length; i++) {
            this.gl.deleteBuffer(this.buffers[i]);
            this.buffers[i] = null;
        }
        this.num_idx_buffers = 0;
        this.alloc();
    }
    bind_attri_buffer(loc) {
        this.gl.bindBuffer(WebGLRenderingContext.ARRAY_BUFFER, this.buffers[loc]);
        return this.buffers[loc];
    }
    bind_idx_buffer(bloc) {
        this.gl.bindBuffer(WebGLRenderingContext.ELEMENT_ARRAY_BUFFER, this.buffers[this.LOC_IND_BASE + bloc]);
        return this.buffers[this.LOC_IND_BASE + bloc];
    }
    destroy() {
        for (var i = 0; i < this.buffers.length; i++) {
            this.gl.deleteBuffer(this.buffers[i]);
            this.buffers[i] = null;
        }
        this.mesh = null;
        this.num_idx_buffers = 0;
    }
}
//# sourceMappingURL=mesh.js.map