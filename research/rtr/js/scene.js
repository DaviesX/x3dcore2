class scene_cache {
    has_mesh(id) {
        return false;
    }
    upload_mesh(id, mesh) {
    }
    unload_mesh(id) {
    }
}
class scene {
    constructor() {
        this.default_id = 139280;
    }
    add_mesh(mesh, id) {
        if (this.meshes.has(id))
            this.cache.unload_mesh(id);
        this.meshes.set(id, mesh);
    }
    add_material(mat, id) {
        this.mats.set(id, mat);
    }
    assign_material_to_mesh(mat_id, mesh_id) {
        if (!this.mats.has(mat_id) || !this.meshes.has(mesh_id))
            return false;
        this.mat_in_mesh.set(mat_id, mesh_id);
        return true;
    }
    gen_default_id() {
        this.default_id++;
        return this.default_id.toString();
    }
    load_from_obj_str(obj_str, is_static) {
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
        mesh.global_trans = mat4_identity();
        var id = this.gen_default_id();
        this.add_mesh(mesh, id);
        var m = new Map();
        m.set(id, mesh);
        return m;
    }
    get_all_mesh_ids() {
        var ids = new Array();
        this.meshes.forEach(function (mesh, id, m) {
            ids.push(id);
        });
        return ids;
    }
    upload() {
        var that = this;
        this.meshes.forEach(function (mesh, id, m) {
            if (!that.cache.has_mesh(id) || !mesh.is_static) {
                that.cache.upload_mesh(id, mesh);
            }
        });
    }
}
//# sourceMappingURL=scene.js.map