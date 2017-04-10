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
    get_normal_transform() {
        return this.global_trans;
    }
}
class vbo {
}
//# sourceMappingURL=mesh.js.map