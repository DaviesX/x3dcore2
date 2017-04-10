
/// <reference path="tensor.ts" />

class trimesh
{
        public vertices         = new Array<vec3>();
        public texcoords        = new Array<vec2>();
        public normals          = new Array<vec3>();
        public indices          = Array<number>();

        public global_trans: mat4;
        public is_static: boolean;


        constructor()
        {
        }

        public get_vertex_transform(): mat4
        {
                return this.global_trans;
        }

        public get_normal_transform(): mat4
        {
                return this.global_trans;
        }
}

class vbo
{
}