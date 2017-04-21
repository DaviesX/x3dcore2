
class vec2
{
        public x: number;
        public y: number;

        constructor(x: number, y: number)
        {
                this.x = x;
                this.y = y;
        }

        public add(rhs: vec2): vec2
        {
                return new vec2(this.x + rhs.x, this.y + rhs.y);
        }

        public sub(rhs: vec2): vec2
        {
                return new vec2(this.x - rhs.x, this.y - rhs.y);
        }

        public scale(s: number): vec2
        {
                return new vec2(s * this.x, s * this.y);
        }

        public inner(rhs: vec2): number
        {
                return this.x * rhs.x + this.y * rhs.y;
        }

        public len(): number
        {
                return Math.sqrt(this.inner(this));
        }

        public norm(): vec2
        {
                var inv_len = 1.0 / this.len();
                return new vec2(this.x * inv_len, this.y * inv_len);
        }
}

class vec3
{
        public x: number;
        public y: number;
        public z: number;

        constructor(x: number, y: number, z: number)
        {
                this.x = x;
                this.y = y;
                this.z = z;
        }

        public add(rhs: vec3): vec3
        {
                return new vec3(this.x + rhs.x, this.y + rhs.y, this.z + rhs.z);
        }

        public sub(rhs: vec3): vec3
        {
                return new vec3(this.x - rhs.x, this.y - rhs.y, this.z - rhs.z);
        }

        public scale(s: number): vec3
        {
                return new vec3(s * this.x, s * this.y, s * this.z);
        }

        public inner(rhs: vec3): number
        {
                return this.x * rhs.x + this.y * rhs.y + this.z * this.z;
        }

        public outter(rhs: vec3): vec3
        {
                return new vec3(this.y * rhs.z - rhs.y * this.z, -(this.x * rhs.z - rhs.x * this.z), this.x * rhs.y - rhs.x * this.y);
        }

        public len(): number
        {
                return Math.sqrt(this.inner(this));
        }

        public norm(): vec3
        {
                var inv_len = 1.0 / this.len();
                return new vec3(this.x * inv_len, this.y * inv_len, this.z * inv_len);
        }

        public toarray(): Array<number>
        {
                return [this.x, this.y, this.z];
        }
}

class vec4
{
        public x: number;
        public y: number;
        public z: number;
        public w: number;

        constructor(x: number, y: number, z: number, w: number)
        {
                this.x = x;
                this.y = y;
                this.z = z;
                this.w = w;
        }

        public scale(s: number): vec4
        {
                return new vec4(s * this.x, s * this.y, s * this.z, s * this.w);
        }


        public tovec3w(): vec3
        {
                return new vec3(this.x / this.w, this.y / this.w, this.z / this.w);
        }

        public tovec3(): vec3
        {
                return new vec3(this.x, this.y, this.z);
        }

        public toarray(): Array<number>
        {
                return [this.x, this.y, this.z, this.w];
        }
}

class mat4
{
        public col0: vec4;
        public col1: vec4;
        public col2: vec4;
        public col3: vec4;

        constructor(col0: vec4, col1: vec4, col2: vec4, col3: vec4)
        {
                this.col0 = col0;
                this.col1 = col1;
                this.col2 = col2;
                this.col3 = col3;
        }

        public scale(s: number): mat4
        {
                return new mat4(
                        this.col0.scale(s),
                        this.col1.scale(s),
                        this.col2.scale(s),
                        this.col3.scale(s)
                );
        }

        public mul(rhs: mat4): mat4
        {
                return new mat4(
                        new vec4(this.col0.x * rhs.col0.x + this.col1.x * rhs.col0.y + this.col2.x * rhs.col0.z + this.col3.x * rhs.col0.w,
                                this.col0.y * rhs.col0.x + this.col1.y * rhs.col0.y + this.col2.y * rhs.col0.z + this.col3.y * rhs.col0.w,
                                this.col0.z * rhs.col0.x + this.col1.z * rhs.col0.y + this.col2.z * rhs.col0.z + this.col3.z * rhs.col0.w,
                                this.col0.w * rhs.col0.x + this.col1.w * rhs.col0.y + this.col2.w * rhs.col0.z + this.col3.w * rhs.col0.w),

                        new vec4(this.col0.x * rhs.col1.x + this.col1.x * rhs.col1.y + this.col2.x * rhs.col1.z + this.col3.x * rhs.col1.w,
                                this.col0.y * rhs.col1.x + this.col1.y * rhs.col1.y + this.col2.y * rhs.col1.z + this.col3.y * rhs.col1.w,
                                this.col0.z * rhs.col1.x + this.col1.z * rhs.col1.y + this.col2.z * rhs.col1.z + this.col3.z * rhs.col1.w,
                                this.col0.w * rhs.col1.x + this.col1.w * rhs.col1.y + this.col2.w * rhs.col1.z + this.col3.w * rhs.col1.w),

                        new vec4(this.col0.x * rhs.col2.x + this.col1.x * rhs.col2.y + this.col2.x * rhs.col2.z + this.col3.x * rhs.col2.w,
                                this.col0.y * rhs.col2.x + this.col1.y * rhs.col2.y + this.col2.y * rhs.col2.z + this.col3.y * rhs.col2.w,
                                this.col0.z * rhs.col2.x + this.col1.z * rhs.col2.y + this.col2.z * rhs.col2.z + this.col3.z * rhs.col2.w,
                                this.col0.w * rhs.col2.x + this.col1.w * rhs.col2.y + this.col2.w * rhs.col2.z + this.col3.w * rhs.col2.w),

                        new vec4(this.col0.x * rhs.col3.x + this.col1.x * rhs.col3.y + this.col2.x * rhs.col3.z + this.col3.x * rhs.col3.w,
                                this.col0.y * rhs.col3.x + this.col1.y * rhs.col3.y + this.col2.y * rhs.col3.z + this.col3.y * rhs.col3.w,
                                this.col0.z * rhs.col3.x + this.col1.z * rhs.col3.y + this.col2.z * rhs.col3.z + this.col3.z * rhs.col3.w,
                                this.col0.w * rhs.col3.x + this.col1.w * rhs.col3.y + this.col2.w * rhs.col3.z + this.col3.w * rhs.col3.w)
                );
        }

        public apply(rhs: vec4): vec4
        {
                return new vec4(
                        this.col0.x * rhs.x + this.col1.x * rhs.y + this.col2.x * rhs.z + this.col3.x * rhs.w,
                        this.col0.y * rhs.x + this.col1.y * rhs.y + this.col2.y * rhs.z + this.col3.y * rhs.w,
                        this.col0.z * rhs.x + this.col1.z * rhs.y + this.col2.z * rhs.z + this.col3.z * rhs.w,
                        this.col0.w * rhs.x + this.col1.w * rhs.y + this.col2.w * rhs.z + this.col3.w * rhs.w
                );
        }

        public transpose(): mat4
        {
                return new mat4(
                        new vec4(this.col0.x, this.col1.x, this.col2.x, this.col3.x),
                        new vec4(this.col0.y, this.col1.y, this.col2.y, this.col3.y),
                        new vec4(this.col0.z, this.col1.z, this.col2.z, this.col3.z),
                        new vec4(this.col0.w, this.col1.w, this.col2.w, this.col3.w)
                );
        }

        public inv(): mat4
        {
                var c = this.col0.x, d = this.col0.y, e = this.col0.z, g = this.col0.w;
                var f = this.col1.x, h = this.col1.y, i = this.col1.z, j = this.col1.w;
                var k = this.col2.x, l = this.col2.y, o = this.col2.z, m = this.col2.w;
                var n = this.col3.x, p = this.col3.y, r = this.col3.z, s = this.col3.w;

                var A = c * h - d * f;
                var B = c * i - e * f;
                var t = c * j - g * f;
                var u = d * i - e * h;
                var v = d * j - g * h;
                var w = e * j - g * i;
                var x = k * p - l * n;
                var y = k * r - o * n;
                var z = k * s - m * n;
                var C = l * r - o * p;
                var D = l * s - m * p;
                var E = o * s - m * r;
                var q = 1 / (A * E - B * D + t * C + u * z - v * y + w * x);

                return new mat4(
                        new vec4((h * E - i * D + j * C) * q, (-d * E + e * D - g * C) * q, (p * w - r * v + s * u) * q, (-l * w + o * v - m * u) * q),
                        new vec4((-f * E + i * z - j * y) * q, (c * E - e * z + g * y) * q, (-n * w + r * t - s * B) * q, (k * w - o * t + m * B) * q),
                        new vec4((f * D - h * z + j * x) * q, (-c * D + d * z - g * x) * q, (n * v - p * t + s * A) * q, (-k * v + l * t - m * A) * q),
                        new vec4((-f * C + h * y - i * x) * q, (c * C - d * y + e * x) * q, (-n * u + p * B - r * A) * q, (k * u - l * B + o * A) * q)
                );
        }

        public toarray(): Array<number>
        {
                return this.col0.toarray().concat(this.col1.toarray()).concat(this.col2.toarray()).concat(this.col3.toarray());
        }
}

function mat4_identity(): mat4
{
        return new mat4(
                new vec4(1, 0, 0, 0),
                new vec4(0, 1, 0, 0),
                new vec4(0, 0, 1, 0),
                new vec4(0, 0, 0, 1)
        );
}

function mat4_tscale(s: vec3): mat4
{
        return new mat4(
                new vec4(s.x, 0, 0, 0),
                new vec4(0, s.y, 0, 0),
                new vec4(0, 0, s.z, 0),
                new vec4(0, 0, 0, 1)
        );
}

function mat4_ttrans(v: vec3): mat4
{
        return new mat4(
                new vec4(1, 0, 0, 0),
                new vec4(0, 1, 0, 0),
                new vec4(0, 0, 1, 0),
                new vec4(v.x, v.y, v.z, 1)
        );
}

function mat4_trota(a: number, axis: vec3): mat4
{
        axis = axis.norm();

        var ref: vec3 = Math.abs(axis.y - 1) < 1e-7 ? new vec3(1, 0, 0) : new vec3(0, 1, 0);

        var u: vec3 = axis.outter(ref).norm();
        var v: vec3 = axis.outter(u).norm();

        var inv_r: mat4 = new mat4(
                new vec4(u.x, u.y, u.z, 0),
                new vec4(v.x, v.y, v.z, 0),
                new vec4(axis.x, axis.y, axis.z, 0),
                new vec4(0, 0, 0, 1)
        );

        var r: mat4 = inv_r.transpose();

        var cos: number = Math.cos(a);
        var sin: number = Math.sin(a);

        var rotw = new mat4(
                new vec4(cos, sin, 0, 0),
                new vec4(-sin, cos, 0, 0),
                new vec4(0, 0, 1, 0),
                new vec4(0, 0, 0, 1)
        )

        return inv_r.mul(rotw).mul(r);
}

function mat4_tlookat(eye: vec3, center: vec3, up: vec3): mat4
{
        var w: vec3 = center.sub(eye).norm();
        up = up.norm();
        var v: vec3 = w.outter(up).norm();
        var u: vec3 = v.outter(w);

        var l: mat4 = new mat4(
                new vec4(v.x, u.x, -w.x, 0),
                new vec4(v.y, u.y, -w.y, 0),
                new vec4(v.z, u.z, -w.z, 0),
                new vec4(0, 0, 0, 1)
        );

        var t: mat4 = mat4_ttrans(eye.scale(-1));
        return t.mul(l);
}

function mat4_frustum(left: number, right: number, bottom: number, top: number, z_near: number, z_far: number): mat4
{
        var a: number = 2 * z_near;
        var width: number = right - left;
        var height: number = top - bottom;
        var d: number = z_far - z_near;

        var delta_y = top + bottom;
        var delta_x = left + right;

        return new mat4(
                new vec4(a / width, 0, 0, 0),
                new vec4(0, a / height, 0, 0),
                new vec4(delta_x / width, delta_y / height, (-z_far - z_near) / d, -1),
                new vec4(0, 0, -a * z_far / d, 0)
        );
}

function mat4_perspective(fovy: number, aspect: number, z_near: number, z_far: number): mat4
{
        var tan: number = Math.tan(fovy * Math.PI / 360);
        var top: number = z_near * tan;
        var right: number = top * aspect;
        return mat4_frustum(-right, right, -top, top, z_near, z_far);
}

function mat4_viewport(x: number, y: number, height: number, width: number)
{
        return new mat4(
                new vec4(width / 2, 0, 0, 0),
                new vec4(0, height / 2, 0, 0),
                new vec4(0, 0, 1, 0),
                new vec4(width / 2 + x, height / 2 + y, 0, 1)
        );
}

function mat4_normal_affine(affine: mat4): mat4
{
        return affine.inv().transpose();
}

//function mat4_proj

function rad2deg(rad: number): number
{
        return rad / Math.PI * 180;
}

function deg2rad(deg: number): number
{
        return deg / 180 * Math.PI;
}