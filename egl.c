#include "egl.h"

#include <stdio.h>

#include <EGL/egl.h>

struct EGL_context {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    int        native_window[3];
};

static __thread char *last_error;
static __thread char last_error_buf[256];

static char *egl_error_string(const char *function, EGLint err)
{
    const char *reason;
    switch (err) {

    case EGL_NOT_INITIALIZED:
        reason = "not initialized";
        break;

    case EGL_BAD_ALLOC:
        reason = "bad alloc";
        break;

    case EGL_BAD_ATTRIBUTE:
        reason = "bad attribute";
        break;

    case EGL_BAD_CONFIG:
        reason = "bad config";
        break;

    case EGL_BAD_DISPLAY:
        reason = "bad display";
        break;

    case EGL_BAD_MATCH:
        reason = "bad match";
        break;

    case EGL_BAD_NATIVE_WINDOW:
        reason = "bad native window";
        break;

    case EGL_BAD_PARAMETER:
        reason = "bad parameter";
        break;

    default:
        reason = "unknown error";
        break;
    }
    static __thread char buf[256];
    snprintf(buf, sizeof buf, "%s: %s", function, reason);
    return buf;
}

const char *EGL_last_error(void)
{
    return last_error;
}

EGL_context *init_EGL(uint32_t native_surface,
                      uint32_t surface_width,
                      uint32_t surface_height)
{
    EGL_context *ctx = calloc(1, sizeof *ctx);

    ctx->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (ctx->display == EGL_NO_DISPLAY) {
        strerror_r(errno, last_error_buf, sizeof last_error_buf);
        last_error = last_error_buf;
        free(ctx);
        return NULL;
    }

    EGLint major = 0, minor = 0;
    if (eglInitialize(ctx->display, &major, &minor) == EGL_FALSE) {
        last_error = egl_error_string("eglInitialize", eglGetError());
        eglTerminate(ctx->display);
        free(ctx);
        return NULL;
    }
    // fprintf(stderr, "This is EGL %d.%d\n", major, minor);

    static const EGLint attribute_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_SAMPLES, 4,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NATIVE_RENDERABLE, EGL_TRUE,
        EGL_NONE
    };
    EGLConfig config = 0;
    EGLint numconfig = 0;
    if (!eglChooseConfig(ctx->display,
                         attribute_list,
                         &config, 1,
                         &numconfig)) {
        last_error = egl_error_string("eglChooseConfig", eglGetError());
        eglTerminate(ctx->display);
        free(ctx);
        return NULL;
    }

    static const EGLint context_attributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    ctx->context =
        eglCreateContext(ctx->display,
                         config,
                         EGL_NO_CONTEXT,
                         context_attributes);
    if (ctx->context == EGL_NO_CONTEXT) {
        last_error = "could not create EGL context";
        eglTerminate(ctx->display);
        free(ctx);
        return NULL;
    }

    ctx->native_window[0] = native_surface;
    ctx->native_window[1] = surface_width;
    ctx->native_window[2] = surface_height;
    ctx->surface =
        eglCreateWindowSurface(ctx->display, config, ctx->native_window, NULL);
    if (ctx->surface == EGL_NO_SURFACE) {
        last_error = egl_error_string("eglCreateWindowSurface", eglGetError());
        eglDestroyContext(ctx->display, ctx->context);
        eglTerminate(ctx->display);
        free(ctx);
        return NULL;
    }

    if (eglMakeCurrent(ctx->display,
                       ctx->surface,
                       ctx->surface,
                       ctx->context) == EGL_FALSE) {
        last_error = egl_error_string("eglMakeCurrent", eglGetError());
        eglDestroyContext(ctx->display, ctx->context);
        eglTerminate(ctx->display);
        free(ctx);
        return NULL;
    }

    // Don't wait for vsync.
    eglSwapInterval(ctx->display, 0);

    return ctx;
}

void deinit_EGL(EGL_context *ctx)
{
    if (eglMakeCurrent(ctx->display,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT) != EGL_TRUE) {
        fprintf(stderr, "MakeCUrrent failed\n");
    }
    if (eglDestroySurface(ctx->display, ctx->surface) != EGL_TRUE) {
        fprintf(stderr, "eglDestroySurface failed\n");
    }
    if (eglDestroyContext(ctx->display, ctx->context) != EGL_TRUE) {
        fprintf(stderr, "eglDestroyContext failed\n");
    }
    if (eglTerminate(ctx->display) != EGL_TRUE) {
        fprintf(stderr, "eglTerminate failed\n");
    }
    free(ctx);
}

void EGL_swap_buffers(EGL_context *ctx)
{
    eglSwapBuffers(ctx->display, ctx->surface);
}
