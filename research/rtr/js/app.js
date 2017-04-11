var teapot_scene;
var bunny_scene;
var mountain_scene;
var plate_scene;
var cornell_scene;
var teapot_id;
var bunny_id;
var mountain_id;
var plate_id;
var DefaultScenes;
(function (DefaultScenes) {
    DefaultScenes[DefaultScenes["Teapot"] = 0] = "Teapot";
    DefaultScenes[DefaultScenes["StandfordBunny"] = 1] = "StandfordBunny";
    DefaultScenes[DefaultScenes["Mountain"] = 2] = "Mountain";
    DefaultScenes[DefaultScenes["Plate"] = 3] = "Plate";
    DefaultScenes[DefaultScenes["CornellBox"] = 4] = "CornellBox";
})(DefaultScenes || (DefaultScenes = {}));
function load_default_scenes() {
    teapot_scene = new scene();
    bunny_scene = new scene();
    mountain_scene = new scene();
    plate_scene = new scene();
    cornell_scene = new scene();
    var tteapot = mat4_trota(-1.5708, new vec3(1, 0, 0)).
        mul(mat4_tscale(new vec3(0.15, 0.15, 0.15)));
    teapot_scene.load_from_obj_str(eval("teapot_mesh_str"), tteapot, true);
    var tbunny = mat4_ttrans(new vec3(0.5, 0, 0));
    bunny_scene.load_from_obj_str(eval("bunny_mesh_str"), tbunny, true);
    var tmountain = mat4_ttrans(new vec3(0.5, -3, -10));
    mountain_scene.load_from_obj_str(eval("mountain_mesh_str"), tmountain, true);
    var tplate = mat4_ttrans(new vec3(0.5, 0, -5));
    plate_scene.load_from_obj_str(eval("cornell_mesh_str"), tplate, true);
}
function load_default_materials() {
}
var shaderPrograms;
var currentProgram;
var lightProgram;
function createShader(vs_id, fs_id, is_ext = true) {
    var gl = gl_rendering_context();
    var shaderProg = create_shader_program(vs_id, fs_id, is_ext);
    var vertex_ptr = gl.getAttribLocation(shaderProg, "aVertexPosition");
    gl.enableVertexAttribArray(vertex_ptr);
    var normal_ptr = gl.getAttribLocation(shaderProg, "aVertexNormal");
    gl.enableVertexAttribArray(normal_ptr);
    return shaderProg;
}
function initShaders() {
    var gl = gl_rendering_context();
    shaderPrograms = [
        createShader("shader-vs", "shader-fs0"),
        createShader("shader-vs", "shader-fs1-1"),
        createShader("shader-vs", "shader-fs1-2"),
        createShader("shader-vs", "shader-fs1-3"),
        createShader("shader-vs", "shader-fs2"),
        createShader("shader-vs", "shader-fs3-1"),
        createShader("shader-vs", "shader-fs3-2"),
        createShader("shader-vs", "shader-fs4"),
    ];
    currentProgram = shaderPrograms[7];
    var exponentUniform_ptr = gl.getUniformLocation(shaderPrograms[5], "uExponent");
    gl.useProgram(shaderPrograms[5]);
    gl.uniform1f(exponentUniform_ptr, 50.0);
    var exponentUniform_ptr = gl.getUniformLocation(shaderPrograms[6], "uExponent");
    gl.useProgram(shaderPrograms[6]);
    gl.uniform1f(exponentUniform_ptr, 50.0);
    var iorUniform_ptr = gl.getUniformLocation(shaderPrograms[7], "uIOR");
    var betaUniform_ptr = gl.getUniformLocation(shaderPrograms[7], "uBeta");
    gl.useProgram(shaderPrograms[7]);
    gl.uniform1f(iorUniform_ptr, 5.0);
    gl.uniform1f(betaUniform_ptr, 0.2);
    lightProgram = create_shader_program("shader-vs-light", "shader-fs-light");
    var vertexPositionAttribute_ptr = gl.getAttribLocation(lightProgram, "aVertexPosition");
    gl.enableVertexAttribArray(vertexPositionAttribute_ptr);
    var pMatrixUniform_ptr = gl.getUniformLocation(lightProgram, "uPMatrix");
}
var lightPositionBuffer;
function initBuffers() {
    var gl = gl_rendering_context();
    lightPositionBuffer = gl.createBuffer();
}
var mvMatrix = mat4_identity();
var nMatrix = mat4_identity();
var pMatrix = mat4_identity();
var lightMatrix = mat4_identity();
var lightPos = new vec4(0, 0, 0, 1);
var lightPower = 5.0;
var diffuseColor = new vec3(0.2392, 0.5216, 0.7765);
var specularColor = new vec3(1, 1, 1);
var ambientIntensity = 0.1;
var rotY = 0.0;
var rotY_light = 0.0;
function setUniforms(prog) {
    var gl = gl_rendering_context();
    var pMatrixUniform = gl.getUniformLocation(prog, "uPMatrix");
    var mvMatrixUniform = gl.getUniformLocation(prog, "uMVMatrix");
    var nMatrixUniform = gl.getUniformLocation(prog, "uNMatrix");
    var lightPosUniform = gl.getUniformLocation(prog, "uLightPos");
    var lightPowerUniform = gl.getUniformLocation(prog, "uLightPower");
    var kdUniform = gl.getUniformLocation(prog, "uDiffuseColor");
    var ksUniform = gl.getUniformLocation(prog, "uSpecularColor");
    var ambientUniform = gl.getUniformLocation(prog, "uAmbient");
    gl.uniformMatrix4fv(pMatrixUniform, false, pMatrix.toarray());
    gl.uniformMatrix4fv(mvMatrixUniform, false, mvMatrix.toarray());
    gl.uniformMatrix4fv(nMatrixUniform, false, nMatrix.toarray());
    gl.uniform3fv(lightPosUniform, lightPos.tovec3().toarray());
    gl.uniform1f(lightPowerUniform, lightPower);
    gl.uniform3fv(kdUniform, diffuseColor.toarray());
    gl.uniform3fv(ksUniform, specularColor.toarray());
    gl.uniform1f(ambientUniform, ambientIntensity);
}
var draw_light = false;
function drawScene() {
    var current_scene = teapot_scene;
    var gl = gl_rendering_context();
    pMatrix = mat4_viewport(0, 0, gl_viewport_width(), gl_viewport_height());
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    pMatrix = pMatrix.mul(mat4_perspective(35, gl_viewport_width() / gl_viewport_height(), 0.1, 1000.0));
    lightMatrix = mat4_ttrans(new vec3(0.0, -1.0, -7.0)).
        mul(mat4_trota(0.3, new vec3(1, 0, 0))).
        mul(mat4_trota(rotY_light, new vec3(0, 1, 0)));
    lightPos = lightMatrix.apply(new vec4(0.0, 2.5, 3.0, 1));
    var glb_mvt = mat4_ttrans(new vec3(0.0, -1.0, -7.0)).
        mul(mat4_trota(0.3, new vec3(1, 0, 0))).
        mul(mat4_trota(rotY, new vec3(0, 1, 0)));
    gl.useProgram(currentProgram);
    var vertex_ptr = gl.getAttribLocation(currentProgram, "aVertexPosition");
    var norm_ptr = gl.getAttribLocation(currentProgram, "aVertexNormal");
    var cache = current_scene.upload();
    var mesh_ids = current_scene.get_all_mesh_ids();
    for (var i = 0; i < mesh_ids.length; i++) {
        var mesh = current_scene.get_mesh(mesh_ids[i]);
        mvMatrix = glb_mvt.mul(mesh.get_vertex_transform());
        nMatrix = mat4_normal_affine(mvMatrix);
        setUniforms(currentProgram);
        var vbo = cache.get_mesh_buffer(mesh_ids[i]);
        vbo.bind_attri_buffer(vbo.LOC_VERT);
        gl.vertexAttribPointer(vertex_ptr, 3, gl.FLOAT, false, 0, 0);
        vbo.bind_attri_buffer(vbo.LOC_NORM);
        gl.vertexAttribPointer(norm_ptr, 3, gl.FLOAT, false, 0, 0);
        var n_idx_buf = vbo.idx_buf_count();
        for (var j = 0; j < n_idx_buf; j++) {
            vbo.bind_idx_buffer(n_idx_buf);
            gl.drawElements(gl.TRIANGLES, vbo.idx_buf_length(j), gl.UNSIGNED_SHORT, 0);
        }
    }
}
var lastTime = 0;
var rotSpeed = 60, rotSpeed_light = 60;
var animated = false, animated_light = false;
function tick() {
    requestAnimationFrame(tick);
    var timeNow = new Date().getTime();
    if (lastTime != 0) {
        var elapsed = timeNow - lastTime;
        if (animated)
            rotY += rotSpeed * 0.0175 * elapsed / 1000.0;
        if (animated_light)
            rotY_light += rotSpeed_light * 0.0175 * elapsed / 1000.0;
    }
    lastTime = timeNow;
    drawScene();
}
function webGLStart() {
    var canvas = ($("#canvas0")[0]);
    var gl = gl_init_from_canvas(canvas);
    load_default_scenes();
    initShaders();
    initBuffers();
    gl.clearColor(0.3, 0.3, 0.3, 1.0);
    gl.enable(gl.DEPTH_TEST);
    currentProgram = shaderPrograms[0];
    tick();
}
//# sourceMappingURL=app.js.map