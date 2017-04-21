
/// <reference path="tensor.ts" />
/// <reference path="gl.ts" />

interface if_light
{
        projective_transforms(): Array<mat4>;
        get_irradiance_call(): shader_call;
        get_incident_call(): shader_call;
        upload(backend: if_raster_backend, prog: program_location): void;
}

class light_spot implements if_light
{
        private umbrella: number = null;
        private near: number = 0.1;
        private far: number = 30;
        private exp: number = 0.0;
        private dir: vec3 = null;
        private p: vec3 = null;

        constructor()
        {
        }

        public set_umbrella(umbrella: number): void
        {
                this.umbrella = Math.cos(umbrella);
        }

        public set_exp(exp: number): void
        {
                this.exp = exp;
        }

        public set_near(near: number): void
        {
                this.near = near;
        }

        public set_far(far: number): void
        {
                this.far = far;
        }

        public set_dir(dir: vec3): void
        {
                this.dir = dir;
        }

        public set_position(p: vec3): void
        {
                this.p = p;
        }

        public projective_transforms(): Array<mat4>
        {
                var trans = new Array<mat4>();
                var proj = mat4_perspective(rad2deg(this.umbrella), 1, this.near, this.far);
                trans.push(proj);
                return trans;
        }

        public get_irradiance_call(): shader_call
        {
                var func = shader_get_builtin_library().get_function("irradspot");
                var irrad_call = new shader_call(func);
                irrad_call.bind_param_to_constant(shader_func_param.lightdir);
                irrad_call.bind_param_to_call(shader_func_param.incident, this.get_incident_call());
                irrad_call.bind_param_to_constant(shader_func_param.exp);
                irrad_call.bind_param_to_constant(shader_func_param.umbrella);
                return irrad_call;
        }

        public get_incident_call(): shader_call
        {
                var func = shader_get_builtin_library().get_function("incident");
                var incident_call = new shader_call(func);
                incident_call.bind_param_to_constant(shader_func_param.lightpos);
                incident_call.bind_param_to_shader_input(shader_func_param.position);
                return incident_call;
        }

        public upload(backend: if_raster_backend, prog: number): void
        {
                backend.program_assign_uniform(prog,
                        shader_constant_var_info(shader_func_param.lightdir)[0],
                        this.dir.toarray(),
                        shader_constant_var_info(shader_func_param.lightdir)[1]);

                backend.program_assign_uniform(prog,
                        shader_constant_var_info(shader_func_param.exp)[0],
                        [this.exp],
                        shader_constant_var_info(shader_func_param.exp)[1]);

                backend.program_assign_uniform(prog,
                        shader_constant_var_info(shader_func_param.umbrella)[0],
                        [this.umbrella],
                        shader_constant_var_info(shader_func_param.umbrella)[1]);

                backend.program_assign_uniform(prog,
                        shader_constant_var_info(shader_func_param.lightpos)[0],
                        this.p.toarray(),
                        shader_constant_var_info(shader_func_param.lightpos)[1]);
        }

}