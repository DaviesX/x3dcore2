
/// <reference path="tensor.ts" />
/// <reference path="shader.ts" />
/// <reference path="mesh.ts" />

enum material_type
{
        lambert,
        orennayar,
        phong,
        blinnphong,
        cooktorr
}

interface if_material
{
        type(): material_type;
        get_required_attributes(): Array<attri_type>;
        get_radiance_call(irradance: shader_call, incident: shader_call): shader_call;
        upload(backend: if_raster_backend, prog: program_location): void;
        is_mergeable(mat: if_material): boolean;
        merge(mat: if_material): void;
}

class mat_lambert implements if_material
{
        private albedo: vec3 = null;

        constructor()
        {
        }

        public type(): material_type
        {
                return material_type.lambert;
        }

        public set_albedo(a: vec3)
        {
                this.albedo = a;
        }

        public is_mergeable(mat: if_material): boolean
        {
                return mat.type() === material_type.lambert;
        }

        public merge(mat: if_material): void
        {
                if (!this.is_mergeable(mat))
                        throw new Error("material " + this.type() + " is not mergable with " + mat.type());
        }

        public get_required_attributes(): Array<attri_type>
        {
                var attris = new Array<attri_type>();
                attris.push(attri_type.index);
                attris.push(attri_type.normal);
                attris.push(attri_type.position);
                return attris;
        }

        public get_radiance_call(irradiance: shader_call, incident: shader_call): shader_call
        {
                if (this.albedo == null)
                        throw new Error("Parameter albedo constant hasn't been set.");
                var norm_func: shader_function = shader_get_builtin_library().get_function("vec3normalize");
                var norm_call: shader_call = new shader_call(norm_func);
                norm_call.bind_param_to_shader_input(shader_func_param.normal);

                var f_rad: shader_function = shader_get_builtin_library().get_function("brdflambert");
                var c_rad = new shader_call(f_rad);
                c_rad.bind_param_to_call(shader_func_param.incident, incident);
                c_rad.bind_param_to_call(shader_func_param.normal, norm_call);
                c_rad.bind_param_to_call(shader_func_param.irrad, irradiance);
                c_rad.bind_param_to_constant(shader_func_param.albedo);
                return c_rad;
        }

        public upload(backend: if_raster_backend, prog: program_location): void
        {
                backend.program_assign_uniform(prog,
                        shader_constant_var_info(shader_func_param.albedo)[0], this.albedo.toarray(),
                        shader_constant_var_info(shader_func_param.albedo)[1]);
        }
}