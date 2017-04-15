
/// <reference path="gl.ts" />
/// <reference path="tensor.ts" />
/// <reference path="scene.ts" />


enum DefaultScenes
{
        Teapot,
        StandfordBunny,
        Mountain,
        Plate,
        CornellBox
}

class app
{
        /*
        * Screen dimensions.
        */
        private width: number;
        private height: number;

        /*
        * Default scenes.
        */
        private teapot_scene: scene;
        private bunny_scene: scene;
        private mountain_scene: scene;
        private plate_scene: scene;
        private cornell_scene: scene;

        private teapot_id: string;
        private bunny_id: string;
        private mountain_id: string;
        private plate_id: string;

        private backend: if_raster_backend;

        private load_default_scenes(): void
        {
                this.teapot_scene = new scene(this.backend);
                this.bunny_scene = new scene(this.backend);
                this.mountain_scene = new scene(this.backend);
                this.plate_scene = new scene(this.backend);
                this.cornell_scene = new scene(this.backend);

                var tteapot = mat4_trota(-1.5708, new vec3(1, 0, 0)).
                        mul(mat4_tscale(new vec3(0.15, 0.15, 0.15)));
                this.teapot_scene.load_from_obj_str(eval("teapot_mesh_str"), tteapot, true);

                var tbunny = mat4_ttrans(new vec3(0.5, 0, 0));
                this.bunny_scene.load_from_obj_str(eval("bunny_mesh_str"), tbunny, true);

                var tmountain = mat4_ttrans(new vec3(0.5, -3, -10));
                this.mountain_scene.load_from_obj_str(eval("mountain_mesh_str"), tmountain, true);

                var tplate = mat4_ttrans(new vec3(0.5, 0, -5));
                this.plate_scene.load_from_obj_str(eval("cornell_mesh_str"), tplate, true);
        }


        /*
         * Load default materials.
         */
        private load_default_materials(): void
        {
        }

        shaderPrograms: Array<program_location>;
        currentProgram: program_location;
        lightProgram: program_location;

        private createShader(vs_id: string, fs_id: string, is_ext = true): program_location
        {
                return create_shader_program(this.backend, vs_id, fs_id, is_ext);
        }

        public initShaders(): void
        {
                this.shaderPrograms = [
                        this.createShader("shader-vs", "shader-fs0"),
                        this.createShader("shader-vs", "shader-fs1-1"),
                        this.createShader("shader-vs", "shader-fs1-2"),
                        this.createShader("shader-vs", "shader-fs1-3"),
                        this.createShader("shader-vs", "shader-fs2"),
                        this.createShader("shader-vs", "shader-fs3-1"),
                        this.createShader("shader-vs", "shader-fs3-2"),
                        this.createShader("shader-vs", "shader-fs4"),
                ];
                this.currentProgram = this.shaderPrograms[7];

                //
                // Declaring shading model specific uniform variables
                //

                // Phong shading.
                this.backend.program_assign_uniform(this.shaderPrograms[5], "uExponent", [50.0], "float");

                // Blinn-Phong shading.
                this.backend.program_assign_uniform(this.shaderPrograms[6], "uExponent", [50.0], "float");

                // Microfacet shading.
                this.backend.program_assign_uniform(this.shaderPrograms[7], "uIOR", [5.0], "float");
                this.backend.program_assign_uniform(this.shaderPrograms[7], "uBeta", [0.2], "float");

                // Initializing light source drawing shader.
                this.lightProgram = create_shader_program(this.backend, "shader-vs-light", "shader-fs-light");
        }


        /*
         * Initializing buffers
         */
        private lightPositionBuffer: buffer_location;

        private initBuffers(): void
        {
                this.lightPositionBuffer = this.backend.attri_buf_create();
        }


        /*
         * Main rendering code 
         */
        // Basic rendering parameters
        private mvMatrix: mat4 = mat4_identity();
        private nMatrix: mat4 = mat4_identity();
        private pMatrix: mat4 = mat4_identity();

        // Lighting control
        private lightMatrix: mat4 = mat4_identity();
        private lightPos: vec4 = new vec4(0, 0, 0, 1);
        private lightPower = 5.0;

        // Common parameters for shading models
        private diffuseColor: vec3 = new vec3(0.2392, 0.5216, 0.7765);
        private specularColor: vec3 = new vec3(1, 1, 1);
        private ambientIntensity: number = 0.1;

        // Animation related variables
        private rotY: number = 0.0;
        private rotY_light: number = 0.0;

        private setUniforms(prog: program_location): void
        {
                try {
                        this.backend.program_assign_uniform(prog, "uPMatrix", this.pMatrix.toarray(), "mat4");
                        this.backend.program_assign_uniform(prog, "uMVMatrix", this.mvMatrix.toarray(), "mat4");
                        this.backend.program_assign_uniform(prog, "uNMatrix", this.nMatrix.toarray(), "mat4");
                        this.backend.program_assign_uniform(prog, "uLightPos", this.lightPos.tovec3().toarray(), "vec3");
                        this.backend.program_assign_uniform(prog, "uLightPower", [this.lightPower], "float");
                        this.backend.program_assign_uniform(prog, "uDiffuseColor", this.diffuseColor.toarray(), "vec3");
                        this.backend.program_assign_uniform(prog, "uSpecularColor", this.specularColor.toarray(), "vec3");
                        this.backend.program_assign_uniform(prog, "uAmbient", [this.ambientIntensity], "float");
                } catch (e) {
                        console.log(e.toString());
                }
        }

        private draw_light = false;
        private drawScene(): void
        {
                var current_scene: scene = this.mountain_scene;

                this.backend.frame_buf_set_background(this.backend.frame_buf_get_default(), 0.19, 0.19, 0.19, 1.0);
                this.backend.frame_buf_fill(this.backend.frame_buf_get_default(), true, true);

                //pMatrix = mat4_viewport(0, 0, gl_viewport_width(), gl_viewport_height());
                this.pMatrix = mat4_identity();

                this.pMatrix = this.pMatrix.mul(mat4_perspective(35, this.width / this.height, 0.1, 1000.0));

                this.lightMatrix = mat4_ttrans(new vec3(0.0, -1.0, -7.0)).
                        mul(mat4_trota(0.3, new vec3(1, 0, 0))).
                        mul(mat4_trota(this.rotY_light, new vec3(0, 1, 0)));

                this.lightPos = this.lightMatrix.apply(new vec4(0.0, 2.5, 3.0, 1));

                var glb_mvt: mat4 = mat4_ttrans(new vec3(0.0, -1.0, -7.0)).
                        mul(mat4_trota(0.3, new vec3(1, 0, 0))).
                        mul(mat4_trota(this.rotY, new vec3(0, 1, 0)));

                current_scene.upload();
                var mesh_ids: Array<string> = current_scene.get_all_renderable_ids();

                for (var i = 0; i < mesh_ids.length; i++) {
                        var renderable: if_renderable = current_scene.get_renderable(mesh_ids[i]);
                        this.mvMatrix = glb_mvt.mul(renderable.affine_transform());
                        this.nMatrix = mat4_normal_affine(this.mvMatrix);

                        this.setUniforms(this.currentProgram);

                        var vert_buf = renderable.get_buffer(shader_input.position);
                        this.backend.program_assign_input(this.currentProgram, "aVertexPosition", vert_buf[0].get_buf());

                        var norm_buf = renderable.get_buffer(shader_input.normal);
                        this.backend.program_assign_input(this.currentProgram, "aVertexNormal", norm_buf[0].get_buf());

                        var idx_bufs = renderable.get_buffer(shader_input.index);
                        for (var j = 0; j < idx_bufs.length; j++) {
                                this.backend.draw_indexed_triangles(this.backend.frame_buf_get_default(),
                                        this.currentProgram, idx_bufs[j].get_buf(), 0, idx_bufs[j].get_len());
                        }
                }

                if (this.draw_light) {
                        this.backend.program_assign_uniform(this.lightProgram, "uPMatrix", this.pMatrix.toarray(), "mat4");
                        this.backend.program_assign_input(this.lightProgram, "aVertexPosition", this.lightPositionBuffer);
                        this.backend.attri_buf_writef32(this.lightPositionBuffer, Float32Array.from(this.lightPos.tovec3().toarray()), 3, false);
                        this.backend.draw_points(this.backend.frame_buf_get_default(), this.lightProgram, 0, 1);
                }
        }

        private lastTime = 0;
        private rotSpeed = 60;
        private rotSpeed_light = 60;
        private animated = false;
        private animated_light = false;

        private tick(this_: app): void
        {
                //requestAnimationFrame(tick);
                setTimeout(this_.tick.bind(null, this_), 1000);

                var timeNow = new Date().getTime();
                if (this_.lastTime != 0) {
                        var elapsed = timeNow - this_.lastTime;
                        if (this_.animated)
                                this_.rotY += this_.rotSpeed * 0.0175 * elapsed / 1000.0;
                        if (this_.animated_light)
                                this_.rotY_light += this_.rotSpeed_light * 0.0175 * elapsed / 1000.0;
                }
                this_.lastTime = timeNow;

                this_.drawScene();
        }

        public webGLStart(): void
        {
                var canvas = <HTMLCanvasElement>($("#canvas0")[0]);
                this.width = canvas.width;
                this.height = canvas.height;

                this.backend = gl_create_backend_from_canvas(canvas);
                this.load_default_scenes();
                this.initBuffers();
                this.initShaders();

                this.currentProgram = this.shaderPrograms[7];
                this.tick(this);
        }
}