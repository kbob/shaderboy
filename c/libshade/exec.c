#define _GNU_SOURCE
#include "exec.h"

#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "bcm.h"
#include "queue.h"
#include "render.h"

#define FBQ_SIZE 200
#define CBQ_SIZE 200

struct exec {
    // borrowed objects
    bcm_context      *bcm;
    LEDs_context     *leds;

    // start/stop control
    bool            running;
    bool            shutdown;
    unsigned        running_count;
    pthread_cond_t  running_cond;
    pthread_mutex_t running_lock;

    // FPS calculation
    unsigned        frame_count;
    struct timespec time_zero;
    pthread_mutex_t fps_lock;

    // current program
    const prog     *prog;
    pthread_mutex_t prog_lock;

    // worker threads
    pthread_t       render_thread;
    pthread_t       cmd_thread;
    pthread_t       output_thread;

    // inter-worker queues
    queue          *framebuffer_queue;
    queue          *cmdbuffer_queue;

    // inter-worker data buffers
    LED_pixel      *framebuffers[FBQ_SIZE];
    LED_cmd        *cmdbuffers[CBQ_SIZE];
};

static void thread_started(exec *ex, const char *name)
{
    pthread_setname_np(pthread_self(), name);
    pthread_mutex_lock(&ex->running_lock);
    ex->running_count++;
    pthread_mutex_unlock(&ex->running_lock);
}

static void thread_finished(exec *ex)
{
    pthread_mutex_lock(&ex->running_lock);
    ex->running_count--;
    pthread_mutex_unlock(&ex->running_lock);
}

static bool check_running(exec *ex)
{
    bool go = true;
    pthread_mutex_lock(&ex->running_lock);
    while (!ex->running && !ex->shutdown) {
        ex->running_count--;
        pthread_cond_broadcast(&ex->running_cond);
        pthread_cond_wait(&ex->running_cond, &ex->running_lock);
        ex->running_count++;
    }
    go = !ex->shutdown;
    pthread_mutex_unlock(&ex->running_lock);
    return go;
}

static void shutdown(exec *ex)
{
    pthread_mutex_lock(&ex->running_lock);
    ex->shutdown = true;
    pthread_cond_broadcast(&ex->running_cond);
    pthread_mutex_unlock(&ex->running_lock);
}

static const prog *get_prog(exec *ex)
{
    pthread_mutex_lock(&ex->prog_lock);
    const prog *pp = ex->prog;
    pthread_mutex_unlock(&ex->prog_lock);
    return pp;
}

static void set_prog(exec *ex, const prog *pp)
{
    pthread_mutex_lock(&ex->prog_lock);
    ex->prog = pp;
    pthread_mutex_unlock(&ex->prog_lock);
}

static void reset_fps(exec *ex)
{
    pthread_mutex_lock(&ex->fps_lock);
    clock_gettime(CLOCK_MONOTONIC, &ex->time_zero);
    ex->frame_count = 0;
    pthread_mutex_unlock(&ex->fps_lock);
}

static void count_frame(exec *ex)
{
    pthread_mutex_lock(&ex->fps_lock);
    ex->frame_count++;
    pthread_mutex_unlock(&ex->fps_lock);
}

static void *render_thread_main(void *user_data)
{
    exec *ex = user_data;
    thread_started(ex, "SHD Render");

    render_state *rs = render_init(ex->bcm);
    while (check_running(ex)) {
        const prog *pp = get_prog(ex);
        render_frame(rs, pp);
        size_t index = queue_acquire_empty(ex->framebuffer_queue);
        LED_pixel *pixels = ex->framebuffers[index];
        bcm_read_pixels(ex->bcm, pixels, LEDs_framebuffer_pitch(ex->leds));
        queue_release_full(ex->framebuffer_queue);
    }
    render_deinit(rs);
    thread_finished(ex);
    return NULL;
}

static void *cmd_thread_main(void *user_data)
{
    exec *ex = user_data;
    thread_started(ex, "SHD cmd");
    while (check_running(ex)) {
        size_t cb_idx = queue_acquire_empty(ex->cmdbuffer_queue);
        size_t fb_idx = queue_acquire_full(ex->framebuffer_queue);

        LED_pixel *pixels = ex->framebuffers[fb_idx];
        LED_cmd   *cmds   = ex->cmdbuffers[cb_idx];
        LEDs_create_cmds(ex->leds, pixels, cmds);

        queue_release_full(ex->cmdbuffer_queue);
        queue_release_empty(ex->framebuffer_queue);
    }
    thread_finished(ex);
    return NULL;
}

static void *output_thread_main(void *user_data)
{
    exec *ex = user_data;
    thread_started(ex, "SHD Output");
    while (check_running(ex)) {
        size_t index = queue_acquire_full(ex->cmdbuffer_queue);
        LED_cmd *cmds = ex->cmdbuffers[index];

        LEDs_write_cmds(ex->leds, cmds);
        
        queue_release_empty(ex->cmdbuffer_queue);
        
        count_frame(ex);
    }
    thread_finished(ex);
    return NULL;
}

exec *create_exec(bcm_context *bcm, LEDs_context *leds)
{
    exec *ex = calloc(1, sizeof *ex);
    if (!ex)
        goto FAIL;

    ex->bcm  = bcm;
    ex->leds = leds;

    ex->framebuffer_queue = create_queue(FBQ_SIZE);
    ex->cmdbuffer_queue = create_queue(CBQ_SIZE);

    if (pthread_cond_init(&ex->running_cond, NULL))
        goto FAIL;

    if (pthread_mutex_init(&ex->running_lock, NULL))
        goto FAIL;

    if (pthread_mutex_init(&ex->prog_lock, NULL))
        goto FAIL;

    for (size_t i = 0; i < FBQ_SIZE; i++)
        ex->framebuffers[i] = LEDs_alloc_framebuffer(leds);

    for (size_t i = 0; i < CBQ_SIZE; i++)
        ex->cmdbuffers[i] = LEDs_alloc_cmdbuffer(leds);

    if (pthread_create(&ex->render_thread, NULL, render_thread_main, ex))
        goto FAIL;

    if (pthread_create(&ex->cmd_thread, NULL, cmd_thread_main, ex))
        goto FAIL;

    if (pthread_create(&ex->output_thread, NULL, output_thread_main, ex))
        goto FAIL;

    return ex;

FAIL:
    if (ex)
        destroy_exec(ex);
    return NULL;
}

void destroy_exec(exec *ex)
{
    shutdown(ex);

    if (ex->output_thread) {
        // pthread_cancel(ex->output_thread);
        pthread_join(ex->output_thread, NULL);
    }
    if (ex->cmd_thread) {
        // pthread_cancel(ex->cmd_thread);
        pthread_join(ex->cmd_thread, NULL);
    }
    if (ex->render_thread) {
        // pthread_cancel(ex->render_thread);
        pthread_join(ex->render_thread, NULL);
    }

    for (size_t i = 0; i < FBQ_SIZE; i++)
        if (ex->framebuffers[i])
            LEDs_free_framebuffer(ex->framebuffers[i]);

    for (size_t i = 0; i < CBQ_SIZE; i++)
        if (ex->cmdbuffers[i])
            LEDs_free_cmdbuffer(ex->cmdbuffers[i]);

    if (ex->cmdbuffer_queue)
        destroy_queue(ex->cmdbuffer_queue);
    if (ex->framebuffer_queue)
        destroy_queue(ex->framebuffer_queue);

    (void)pthread_mutex_destroy(&ex->prog_lock);
    (void)pthread_cond_destroy(&ex->running_cond);
    (void)pthread_mutex_destroy(&ex->running_lock);

    free(ex);
}

void exec_start(exec *ex)
{
    pthread_mutex_lock(&ex->running_lock);
    ex->running = true;
    pthread_cond_broadcast(&ex->running_cond);
    pthread_mutex_unlock(&ex->running_lock);
    reset_fps(ex);
}


void exec_stop(exec *ex)
{
    pthread_mutex_lock(&ex->running_lock);
    ex->running = false;
    while (ex->running_count) {
        pthread_cond_wait(&ex->running_cond, &ex->running_lock);
    }
    pthread_mutex_unlock(&ex->running_lock);
}

double exec_fps(exec *ex)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    pthread_mutex_lock(&ex->fps_lock);
    double seconds = (now.tv_nsec - ex->time_zero.tv_nsec) / 1.e9;
    seconds += now.tv_sec - ex->time_zero.tv_sec;
    double fps = ex->frame_count / seconds;
    pthread_mutex_unlock(&ex->fps_lock);
    return fps;
}

void exec_use_prog(exec *ex, const prog *pp)
{
    set_prog(ex, pp);
    reset_fps(ex);
}
