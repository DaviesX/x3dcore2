var g_glctx = null;
var g_viewport_width = null;
var g_viewport_height = null;
function gl_init_from_canvas(canvas) {
    try {
        g_viewport_width = canvas.width;
        g_viewport_height = canvas.height;
        g_glctx = canvas.getContext("experimental-webgl");
    }
    catch (e) {
        alert("Could not initialise WebGL, sorry :-(");
        throw e;
    }
    if (g_glctx == null) {
        throw new Error("Could not initialise WebGL, sorry :-(");
    }
}
function gl_viewport_width() {
    return g_viewport_width;
}
function gl_viewport_height() {
    return g_viewport_height;
}
function gl_rendering_context() {
    if (g_glctx != null) {
        return g_glctx;
    }
    else {
        throw new Error("gl_rendering_context(): context not initialized.");
    }
}
//# sourceMappingURL=gl.js.map