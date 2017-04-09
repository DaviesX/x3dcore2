var g_glctx: WebGLRenderingContext = null;
var g_viewport_width: number = null;
var g_viewport_height: number = null;

function gl_init_from_canvas(canvas: HTMLCanvasElement)
{
        try {
                g_viewport_width = canvas.width;
                g_viewport_height = canvas.height;
                g_glctx = canvas.getContext("experimental-webgl");
        } catch (e) {
                alert("Could not initialise WebGL, sorry :-(");
                throw e;
        }
        if (g_glctx == null) {
                throw new Error("Could not initialise WebGL, sorry :-(");
        }
}

function gl_viewport_width(): number
{
        return g_viewport_width;
}

function gl_viewport_height(): number
{
        return g_viewport_height;
}

function gl_rendering_context(): WebGLRenderingContext
{
        if (g_glctx != null) {
                return g_glctx;
        } else {
                throw new Error("gl_rendering_context(): context not initialized.");
        }
}