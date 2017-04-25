
/// <reference path="tensor.ts" />

class camera
{
        private fovy: number = 60;
        private aspect: number = 4.0 / 3.0;
        private z_near: number = 1.0;
        private z_far: number = 1000.0;

        private p = new vec3(0, 0, 0);
        private target = new vec3(0, 0, 0);
        private up = new vec3(0, 1, 0);

        constructor()
        {
        }

        public set_position(p: vec3): void
        {
                this.p = p;
        }

        public set_target(target: vec3): void
        {
                this.target = target;
        }

        public set_up(up: vec3): void
        {
                this.up = up;
        }

        public get_position(): vec3
        {
                return this.p;
        }

        public get_direction(): vec3
        {
                return this.p.sub(this.target).norm();
        }

        public get_frustum(): frustum
        {
                return frustum_perspective(this.fovy, this.aspect, this.z_near, this.z_far);
        }

        public get_modelview_transform(): mat4
        {
                var t: mat4 = mat4_ttrans(this.p);
                var w: vec3 = this.get_position();
                var u: vec3 = w.outter(this.up);
                var v: vec3 = u.outter(w);
                var r: mat4 = mat4_trotd(u, v, w);
                return t.mul(r);
        }
}