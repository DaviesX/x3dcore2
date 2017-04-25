

/// <reference path="pass.ts" />
/// <reference path="camera.ts" />

interface if_pipeline
{
        render(s: scene, c: camera, bgcolor: vec4): void;
        release(): void;
}

class pipe_forward implements if_pipeline
{
        private backend: if_raster_backend;
        private rad_pass = new pass_radiance();

        private frame_id: number = 0;

        constructor(backend: if_raster_backend)
        {
                this.backend = backend;
        }

        public render(s: scene, c: camera, bgcolor: vec4): void
        {
                this.backend.frame_buf_set_background(this.backend.frame_buf_get_default(), bgcolor.x, bgcolor.y, bgcolor.z, bgcolor.w);
                this.backend.frame_buf_fill(this.backend.frame_buf_get_default(), true, true);

                var cache = this.rad_pass.gen_cache(this.backend, s, c, this.frame_id);
                this.rad_pass.run(this.backend, new Map<pass_buffer_type, buffer_location>(), cache);

                this.frame_id++;
        }

        public release(): void
        {
                this.rad_pass.release(this.backend);
        }
}