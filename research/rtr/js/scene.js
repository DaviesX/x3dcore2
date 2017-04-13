class scene_cache {
    constructor(backend) {
        this.rend_cache = new Map();
        this.backend = backend;
    }
    has_renderable(id) {
        return this.rend_cache.has(id);
    }
    upload_renderable(id, rend, types) {
        this.rend_cache.set(id, rend);
        for (var i = 0; i < types.length; i++) {
            rend.upload(this.backend, types[i]);
        }
    }
    unload_renderable(id) {
        if (this.has_renderable(id)) {
            var rend = this.rend_cache.get(id);
            rend.unload(this.backend);
            this.rend_cache.delete(id);
        }
    }
    clear() {
        var c = this;
        this.rend_cache.forEach(function (rend, k, m) {
            rend.unload(c.backend);
        });
        this.rend_cache.clear();
    }
}
class scene {
    constructor(backend) {
        this.rend = new Map();
        this.mats = new Map();
        this.mat_in_rend = new Map();
        this.default_id = 139280;
        this.cache = new scene_cache(backend);
    }
    add_renderable(mesh, id) {
        if (this.rend.has(id))
            this.cache.unload_renderable(id);
        this.rend.set(id, mesh);
    }
    add_material(mat, id) {
        this.mats.set(id, mat);
    }
    assign_material_to_renderable(mat_id, mesh_id) {
        if (!this.mats.has(mat_id) || !this.rend.has(mesh_id))
            return false;
        this.mat_in_rend.set(mat_id, mesh_id);
        return true;
    }
    gen_default_id() {
        this.default_id++;
        return this.default_id.toString();
    }
    load_from_obj_str(obj_str, transform, is_static) {
        var mesh = new trimesh();
        var vertices = new Array();
        var normals = new Array();
        var texcoords = new Array();
        var iverts = new Array();
        var inorms = new Array();
        var itex = new Array();
        var lines = obj_str.split('\n');
        var VERTEX_RE = /^v\s/;
        var NORMAL_RE = /^vn\s/;
        var TEXTURE_RE = /^vt\s/;
        var FACE_RE = /^f\s/;
        var WHITESPACE_RE = /\s+/;
        for (var i = 0; i < lines.length; i++) {
            try {
                var line = lines[i].trim();
                var elements = line.split(WHITESPACE_RE);
                elements.shift();
                if (VERTEX_RE.test(line)) {
                    vertices.push(new vec3(parseFloat(elements[0]), parseFloat(elements[1]), parseFloat(elements[2])));
                }
                else if (NORMAL_RE.test(line)) {
                    normals.push(new vec3(parseFloat(elements[0]), parseFloat(elements[1]), parseFloat(elements[2])));
                }
                else if (TEXTURE_RE.test(line)) {
                    texcoords.push(new vec3(parseFloat(elements[0]), parseFloat(elements[1]), parseFloat(elements[2])));
                }
                else if (FACE_RE.test(line)) {
                    if (elements.length != 3) {
                        console.log("load_from_obj_str() - at line " + (i + 1).toString()
                            + ". Couldn't accept polygon other than triangle.");
                        continue;
                    }
                    for (var v = 0; v < 3; v++) {
                        var vert_indices = elements[v].split("/");
                        if (vert_indices.length != 3)
                            throw new Error("Malformed data at line " + (i + 1).toString()
                                + " where attribute " + (v + 1).toString()
                                + " doesn't have at least 3 vertex attributes");
                        if (vert_indices[0].length == 0)
                            throw new Error("Malformed data at line " + (i + 1).toString()
                                + " where vertex index is missing.");
                        else {
                            var iattri = parseInt(vert_indices[0]) - 1;
                            if (iattri < 0 || iattri >= vertices.length)
                                throw new Error("At line " + (i + 1).toString()
                                    + ", attribute " + (v + 1).toString()
                                    + " referenced vertex " + (iattri + 1).toString()
                                    + " is illegal.");
                            iverts.push(iattri);
                        }
                        if (vert_indices[1].length != 0) {
                            var iattri = parseInt(vert_indices[1]) - 1;
                            if (iattri < 0 || iattri >= texcoords.length)
                                throw new Error("At line " + (i + 1).toString()
                                    + ", attribute " + (v + 1).toString()
                                    + " referenced texcoord " + (iattri + 1).toString()
                                    + " is illegal.");
                            itex.push(iattri);
                        }
                        if (vert_indices[2].length != 0) {
                            var iattri = parseInt(vert_indices[2]) - 1;
                            if (iattri < 0 || iattri >= normals.length)
                                throw new Error("At line " + (i + 1).toString()
                                    + ", attribute " + (v + 1).toString()
                                    + " referenced normal " + (iattri + 1).toString()
                                    + " is illegal.");
                            inorms.push(iattri);
                        }
                    }
                }
            }
            catch (e) {
                throw new Error("Malformed data at line " + (i + 1).toString() + ". nested exception: " + e.toString());
            }
        }
        if ((iverts.length != itex.length && itex.length != 0) ||
            (iverts.length != inorms.length && inorms.length != 0)) {
            throw new Error("Vertex attributes mismatch as "
                + "|v|=" + iverts.length + ",|n|=" + inorms.length + ",|t|=" + itex.length);
        }
        if (vertices.length == 0)
            throw new Error("The mesh doesn't contain vertex data");
        mesh.vertices = vertices;
        if (inorms.length != 0)
            mesh.normals.fill(new vec3(0, 0, 0), 0, vertices.length);
        if (itex.length != 0)
            mesh.texcoords.fill(new vec2(0, 0), 0, vertices.length);
        for (var v = 0; v < iverts.length; v++) {
            mesh.indices.push(iverts[v]);
            mesh.normals[iverts[v]] = normals[inorms[v]];
            mesh.texcoords[iverts[v]] = texcoords[itex[v]];
        }
        mesh.is_static = is_static;
        mesh.global_trans = transform == null ? mat4_identity() : transform;
        var id = this.gen_default_id();
        this.add_renderable(mesh, id);
        var m = new Map();
        m.set(id, mesh);
        return m;
    }
    get_all_renderable_ids() {
        var ids = new Array();
        this.rend.forEach(function (rend, id, m) {
            ids.push(id);
        });
        return ids;
    }
    get_all_material_ids() {
        var ids = new Array();
        this.mats.forEach(function (mat, id, m) {
            ids.push(id);
        });
        return ids;
    }
    get_renderable(id) {
        return this.rend.get(id);
    }
    get_renderable_material(rend_id) {
        var mat_id = this.mat_in_rend.get(rend_id);
        return mat_id != null ? this.mats.get(mat_id) : null;
    }
    upload() {
        var this_ = this;
        this.rend.forEach(function (rend, id, m) {
            if (!this_.cache.has_renderable(id) && rend.is_permanent()) {
                var all_attris = rend.available_attributes();
                this_.cache.upload_renderable(id, rend, all_attris);
            }
            else if (!rend.is_permanent()) {
                var mat_id = this_.mat_in_rend.get(id);
                if (mat_id == null)
                    throw new Error("Cannot upload renderable " + id + " for it has no material.");
                var mat = this_.mats.get(mat_id);
                var adaptive_attris = mat.get_required_attributes();
                this_.cache.upload_renderable(id, rend, adaptive_attris);
            }
        });
        return this.cache;
    }
    clear() {
        this.rend.clear();
        this.mats.clear();
        this.mat_in_rend.clear();
        this.cache.clear();
    }
}
//# sourceMappingURL=scene.js.map