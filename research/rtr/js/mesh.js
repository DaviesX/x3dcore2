var attri_type;
(function (attri_type) {
    attri_type[attri_type["position"] = 0] = "position";
    attri_type[attri_type["normal"] = 1] = "normal";
    attri_type[attri_type["texcoord"] = 2] = "texcoord";
    attri_type[attri_type["index"] = 3] = "index";
})(attri_type || (attri_type = {}));
class buffer_info {
    constructor(loc, len) {
        this.loc = loc;
        this.len = len;
    }
    get_buf() {
        return this.loc;
    }
    get_len() {
        return this.len;
    }
}
class trimesh {
    constructor() {
        this.vertices = new Array();
        this.texcoords = new Array();
        this.normals = new Array();
        this.indices = Array();
        this.NUM_DATA_BUFS = 3;
        this.HAS_UINT16_RESTRICTION = true;
        this.vbos = new Array();
        this.ibo = new Array();
        this.num_idx_buffers = 0;
    }
    affine_transform() {
        return this.global_trans;
    }
    is_permanent() {
        return this.is_static;
    }
    available_attributes() {
        var types = new Array();
        types.push(attri_type.position);
        if (this.has_index())
            types.push(attri_type.index);
        if (this.has_normal())
            types.push(attri_type.normal);
        if (this.has_tex_coords())
            types.push(attri_type.texcoord);
        return types;
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
        var n_idx = this.idx_buf_count();
        var base = 0;
        for (var i = 0; i < n_idx; i++) {
            var arr = new Uint16Array(this.idx_buf_length(i));
            for (var j = 0; j < arr.length; j++)
                arr[j] = this.indices[base + j];
            base += arr.length;
            m_idx.push(arr);
        }
        return m_idx;
    }
    has_normal() {
        return this.normals.length != 0;
    }
    has_tex_coords() {
        return this.texcoords.length != 0;
    }
    has_index() {
        return this.indices.length != 0;
    }
    idx_buf_count() {
        return this.HAS_UINT16_RESTRICTION ? Math.ceil(this.indices.length / 0XFFFF) : 1;
    }
    idx_buf_length(bloc) {
        return this.HAS_UINT16_RESTRICTION ? Math.min(0XFFFF, this.indices.length - bloc * 0XFFFF) : this.indices.length;
    }
    realloc(backend) {
        for (var i = 0; i < this.NUM_DATA_BUFS; i++) {
            if (this.vbos[i] == null)
                this.vbos[i] = backend.attri_buf_create();
        }
        var new_idx_count = this.idx_buf_count();
        if (new_idx_count > this.num_idx_buffers) {
            for (var i = this.num_idx_buffers; i < new_idx_count; i++) {
                this.ibo[i] = backend.index_buf_create();
            }
        }
        this.num_idx_buffers = new_idx_count;
    }
    upload(backend, o) {
        this.realloc(backend);
        switch (o) {
            case attri_type.position:
                backend.attri_buf_writef32(this.vbos[attri_type.position], this.get_vertices_f32(), 3, this.is_permanent());
                return [new buffer_info(this.vbos[attri_type.position], this.vertices.length)];
            case attri_type.normal:
                if (!this.has_normal())
                    throw new Error("This mesh doesn't have the normal attributes.");
                backend.attri_buf_writef32(this.vbos[attri_type.normal], this.get_normals_f32(), 3, this.is_permanent());
                return [new buffer_info(this.vbos[attri_type.normal], this.normals.length)];
            case attri_type.texcoord:
                if (!this.has_tex_coords())
                    throw new Error("This mesh doesn't have the texcoord attributes.");
                backend.attri_buf_writef32(this.vbos[attri_type.texcoord], this.get_texcoords_f32(), 2, this.is_permanent());
                return [new buffer_info(this.vbos[attri_type.texcoord], this.texcoords.length)];
            case attri_type.index:
                if (!this.has_index())
                    throw new Error("This mesh does't have index.");
                if (this.HAS_UINT16_RESTRICTION) {
                    var m_idx = this.get_multi_indices_u16();
                    var infos = new Array();
                    for (var i = 0; i < this.num_idx_buffers; i++) {
                        backend.index_buf_write_u16(this.ibo[i], m_idx[i], this.is_permanent());
                        infos[i] = new buffer_info(this.ibo[i], m_idx[i].length);
                    }
                    return infos;
                }
                else {
                    backend.index_buf_write_u32(this.ibo[0], this.get_indices_u32(), this.is_permanent());
                    return [new buffer_info(this.ibo[0], this.indices.length)];
                }
        }
    }
    unload(backend) {
        for (var i = 0; i < this.vbos.length; i++) {
            backend.attri_buf_delete(this.vbos[i]);
            this.vbos[i] = null;
        }
        for (var i = 0; i < this.ibo.length; i++) {
            backend.index_buf_delete(this.ibo[i]);
            this.ibo[i] = null;
        }
        this.num_idx_buffers = 0;
    }
    get_buffer(o) {
        switch (o) {
            case attri_type.position:
                return [new buffer_info(this.vbos[attri_type.position], this.vertices.length)];
            case attri_type.normal:
                if (!this.has_normal())
                    throw new Error("This mesh doesn't have the normal attributes.");
                return [new buffer_info(this.vbos[attri_type.normal], this.normals.length)];
            case attri_type.texcoord:
                if (!this.has_tex_coords())
                    throw new Error("This mesh doesn't have the texcoord attributes.");
                return [new buffer_info(this.vbos[attri_type.texcoord], this.texcoords.length)];
            case attri_type.index:
                if (!this.has_index())
                    throw new Error("This mesh does't have index.");
                if (this.HAS_UINT16_RESTRICTION) {
                    var infos = new Array();
                    for (var i = 0; i < this.num_idx_buffers; i++) {
                        infos[i] = new buffer_info(this.ibo[i], this.idx_buf_length(i));
                    }
                    return infos;
                }
                else {
                    return [new buffer_info(this.ibo[0], this.indices.length)];
                }
        }
    }
    get_transform_call(o) {
        switch (o) {
            case attri_type.position:
                var t_vert = shader_get_builtin_library().get_function("vec3modelview");
                var t_vert_call = new shader_call(t_vert);
                t_vert_call.bind_param_to_constant(shader_func_param.t_modelview);
                t_vert_call.bind_param_to_shader_input(shader_func_param.position);
                return t_vert_call;
            case attri_type.normal:
                if (!this.has_normal())
                    throw new Error("This mesh doesn't have the normal attributes.");
                var t_norm = shader_get_builtin_library().get_function("vec3nmodelview");
                var t_norm_call = new shader_call(t_norm);
                t_norm_call.bind_param_to_constant(shader_func_param.t_nmodelview);
                t_norm_call.bind_param_to_shader_input(shader_func_param.normal);
                return t_norm_call;
            case attri_type.texcoord:
                if (!this.has_tex_coords())
                    throw new Error("This mesh doesn't have the texcoord attributes.");
                return null;
            default:
                throw new Error("Invalid attribute type " + o.toString() + " for generating transform call.");
        }
    }
    upload_transform_call(backend, prog, o, modelview, proj) {
        var comp_modelview = modelview.mul(this.affine_transform());
        switch (o) {
            case attri_type.position:
                backend.program_assign_uniform(prog, shader_constant_var_info(shader_func_param.t_modelview)[0], comp_modelview.toarray(), shader_constant_var_info(shader_func_param.t_modelview)[1]);
                break;
            case attri_type.normal:
                if (!this.has_normal())
                    throw new Error("This mesh doesn't have the normal attributes.");
                backend.program_assign_uniform(prog, shader_constant_var_info(shader_func_param.t_nmodelview)[0], mat4_normal_affine(comp_modelview).toarray(), shader_constant_var_info(shader_func_param.t_nmodelview)[1]);
                break;
            case attri_type.texcoord:
                if (!this.has_tex_coords())
                    throw new Error("This mesh doesn't have the texcoord attributes.");
                break;
            default:
                throw new Error("Invalid attribute type " + o.toString() + " for transform call upload.");
        }
    }
}
//# sourceMappingURL=mesh.js.map