
/// <reference path="mesh.ts" />
/// <reference path="material.ts" />

class scene_cache
{
        private mesh_cache = new Map<string, mesh_vbo>();

        public has_mesh(id: string): boolean
        {
                return this.mesh_cache.has(id);
        }

        public upload_mesh(id: string, mesh: trimesh): void
        {
                var vbo;
                if (!this.has_mesh(id)) {
                        vbo = new mesh_vbo(mesh);
                        this.mesh_cache.set(id, vbo);
                } 
                vbo.unload();
        }

        public unload_mesh(id: string): void
        {
                if (this.has_mesh(id)) {
                        var vbo: mesh_vbo = this.mesh_cache.get(id);
                        vbo.destroy();
                        this.mesh_cache.delete(id);
                }
        }

        public get_mesh_buffer(id: string): mesh_vbo
        {
                return this.mesh_cache.get(id);
        }

        public clear(): void
        {
                this.mesh_cache.forEach(function (vbo: mesh_vbo, k, m) {
                        vbo.destroy();
                });
                this.mesh_cache.clear();
        }
}

class scene
{
        private meshes =        new Map<string, trimesh>();
        private mats =          new Map<string, material>();
        private mat_in_mesh =   new Map<string, string>();
        private default_id =    139280;
        private cache =         new scene_cache();

        constructor()
        {
        }

        public add_mesh(mesh: trimesh, id: string): void
        {
                if (this.meshes.has(id))
                        this.cache.unload_mesh(id);
                this.meshes.set(id, mesh);
        }

        public add_material(mat: material, id: string): void
        {
                this.mats.set(id, mat);
        }

        public assign_material_to_mesh(mat_id: string, mesh_id: string): boolean
        {
                if (!this.mats.has(mat_id) || !this.meshes.has(mesh_id))
                        return false;
                this.mat_in_mesh.set(mat_id, mesh_id);
                return true;
        }

        public gen_default_id(): string
        {
                this.default_id ++;
                return this.default_id.toString();
        }
 
        public load_from_obj_str(obj_str: string, transform: mat4, is_static: boolean): Map<string, trimesh>
        {
                // 1. The obj data is assumed to be all triangulated.
                // 2. default id is used.
                var mesh: trimesh = new trimesh();

                // Temps.
                var vertices = new Array<vec3>();
                var normals = new Array<vec3>();
                var texcoords = new Array<vec2>();

                var iverts = new Array<number>();
                var inorms = new Array<number>();
                var itex = new Array<number>();

                // array of lines separated by the newline
                var lines: Array<string> = obj_str.split('\n');

                var VERTEX_RE = /^v\s/;
                var NORMAL_RE = /^vn\s/;
                var TEXTURE_RE = /^vt\s/;
                var FACE_RE = /^f\s/;
                var WHITESPACE_RE = /\s+/;


                for (var i = 0; i < lines.length; i++) {
                        try {
                                var line: string = lines[i].trim();
                                var elements: Array<string> = line.split(WHITESPACE_RE);
                                elements.shift();

                                if (VERTEX_RE.test(line)) {
                                        vertices.push(new vec3(parseFloat(elements[0]),
                                                parseFloat(elements[1]),
                                                parseFloat(elements[2])));
                                } else if (NORMAL_RE.test(line)) {
                                        normals.push(new vec3(parseFloat(elements[0]),
                                                parseFloat(elements[1]),
                                                parseFloat(elements[2])));
                                } else if (TEXTURE_RE.test(line)) {
                                        texcoords.push(new vec3(parseFloat(elements[0]),
                                                parseFloat(elements[1]),
                                                parseFloat(elements[2])));
                                } else if (FACE_RE.test(line)) {
                                        if (elements.length != 3) {
                                                // This face is not acceptible.
                                                console.log("load_from_obj_str() - at line " + (i + 1).toString()
                                                        + ". Couldn't accept polygon other than triangle.");
                                                continue;
                                        }

                                        for (var v = 0; v < 3; v ++) {
                                                var vert_indices: Array<string> = elements[v].split("/");
                                                if (vert_indices.length != 3)
                                                        throw new Error("Malformed data at line " + (i+1).toString()
                                                                        + " where attribute " + (v+1).toString() 
                                                                        + " doesn't have at least 3 vertex attributes");

                                                if (vert_indices[0].length == 0)
                                                        throw new Error("Malformed data at line " + (i + 1).toString()
                                                                + " where vertex index is missing.");
                                                else {
                                                        var iattri = parseInt(vert_indices[0]) - 1;
                                                        if (iattri < 0 || iattri >= vertices.length)
                                                                throw new Error("At line " + (i+1).toString()
                                                                                + ", attribute " + (v+1).toString()
                                                                                + " referenced vertex " + (iattri+1).toString()
                                                                                + " is illegal.");
                                                        iverts.push(iattri);
                                                }

                                                if (vert_indices[1].length != 0) {
                                                        var iattri = parseInt(vert_indices[1]) - 1;
                                                        if (iattri < 0 || iattri >= texcoords.length)
                                                                throw new Error("At line " + (i+1).toString()
                                                                                + ", attribute " + (v+1).toString()
                                                                                + " referenced texcoord " + (iattri+1).toString()
                                                                                + " is illegal.");
                                                        itex.push(iattri);
                                                }

                                                if (vert_indices[2].length != 0) {
                                                        var iattri = parseInt(vert_indices[2]) - 1;
                                                        if (iattri < 0 || iattri >= normals.length)
                                                                throw new Error("At line " + (i+1).toString()
                                                                                + ", attribute " + (v+1).toString()
                                                                                + " referenced normal " + (iattri+1).toString()
                                                                                + " is illegal.");
                                                        inorms.push(iattri);
                                                }
                                        }
                                }
                        } catch (e) {
                                throw new Error("Malformed data at line " + (i + 1).toString() + ". nested exception: " + e.toString());
                        }
                }

                // Assemble the face indices with vertex attributes -- shift vertex data to the proper location.
                if ((iverts.length != itex.length && itex.length != 0) ||
                    (iverts.length != inorms.length && inorms.length != 0)) {
                            throw new Error("Vertex attributes mismatch as "
                                        + "|v|=" + iverts.length + ",|n|=" + inorms.length + ",|t|=" + itex.length);
                }
                
                if (vertices.length == 0)
                        throw new Error("The mesh doesn't contain vertex data");
                
                // Vertices are already in the right place.
                mesh.vertices = vertices;
                if (inorms.length != 0)
                        mesh.normals.fill(new vec3(0,0,0), 0, vertices.length);
                if (itex.length != 0)
                        mesh.texcoords.fill(new vec2(0,0), 0, vertices.length);
                for (var v = 0; v < iverts.length; v ++) {
                        mesh.indices.push(iverts[v]);
                        mesh.normals[iverts[v]] = normals[inorms[v]];
                        mesh.texcoords[iverts[v]] = texcoords[itex[v]];
                }

                mesh.is_static = is_static;
                mesh.global_trans = transform == null ? mat4_identity() : transform;

                // Adde object to scene.
                var id = this.gen_default_id();
                this.add_mesh(mesh, id);

                // Return info.
                var m = new Map<string, trimesh>();
                m.set(id,mesh);
                return m;
        }

        public get_all_mesh_ids(): Array<string>
        {
                var ids = new Array<string>();
                this.meshes.forEach(function (mesh: trimesh, id: string, m) {
                        ids.push(id);
                });
                return ids;
        }

        public get_mesh(id: string): trimesh
        {
                return this.meshes.get(id);
        }

        public get_mesh_material(mesh_id: string): material
        {
                var mat_id = this.mat_in_mesh.get(mesh_id);
                return mat_id != null ? this.mats.get(mat_id) : null;
        }

        public upload(): scene_cache
        {
                var that: scene = this;
                this.meshes.forEach(function (mesh: trimesh, id: string, m) {
                        if (!that.cache.has_mesh(id) || !mesh.is_static) {
                                that.cache.upload_mesh(id, mesh);
                        }
                });
                return this.cache;
        }

        public clear(): void
        {
                this.meshes.clear();
                this.mats.clear();
                this.mat_in_mesh.clear();
                this.cache.clear();
        }
}