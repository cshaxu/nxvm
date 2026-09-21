#ifndef TEST_LINUX_WAIT_FAKES_H
#define TEST_LINUX_WAIT_FAKES_H

/* Deterministic POSIX boundary substitute. The production Linux algorithm
 * runs unchanged; no OS scheduling or elapsed sleep decides the assertions. */
#define LIB_TYPES_LINUX_SYNC_H
#define LIB_TYPES_LINUX_CLOCK_H
#include "lib/types/types_interface.h"
#include <assert.h>

typedef struct { int alive, locked; } lib_linux_pthread_mutex_t;
typedef struct { int alive; } lib_linux_pthread_cond_t;
typedef struct { int alive, clock; } lib_linux_pthread_condattr_t;
typedef int lib_linux_pthread_once_t;
typedef int lib_linux_pthread_t;
typedef long lib_linux_time_t;
typedef struct { lib_linux_time_t tv_sec; long tv_nsec; } lib_linux_timespec;
#define LIB_LINUX_PTHREAD_MUTEX_INITIALIZER { 1, 0 }
#define LIB_LINUX_PTHREAD_ONCE_INIT 0
#define LIB_LINUX_CLOCK_MONOTONIC 1
#define LIB_LINUX_ETIMEDOUT 110
#define LIB_LINUX_EINTR 4

static int lib_linux_errno;
static int fail_init_step, init_step, live_mutexes, live_conditions, live_attributes;
static int wait_calls, wait_result, clock_failure, sleep_calls;
static int interrupt_sleep;
static int fail_signal, fail_lock, fail_unlock;
static int fail_thread, fail_join, thread_joins;
static void *(*thread_entry)(void *);
static void *thread_context;
static void (*wait_hook)(void);
static lib_linux_timespec observed_deadline;
static int init_failed(void) { return ++init_step == fail_init_step; }
static inline int lib_linux_pthread_mutex_init(lib_linux_pthread_mutex_t *m, const void *a)
{ (void)a; if (init_failed()) return 1; m->alive = 1; m->locked = 0; ++live_mutexes; return 0; }
static inline int lib_linux_pthread_mutex_destroy(lib_linux_pthread_mutex_t *m)
{ assert(m->alive && !m->locked); m->alive = 0; --live_mutexes; return 0; }
static inline int lib_linux_pthread_mutex_lock(lib_linux_pthread_mutex_t *m)
{ assert(m->alive && !m->locked); if(fail_lock) return 1; m->locked = 1; return 0; }
static inline int lib_linux_pthread_mutex_unlock(lib_linux_pthread_mutex_t *m)
{ assert(m->alive && m->locked); m->locked = 0; return fail_unlock; }
static inline int lib_linux_pthread_condattr_init(lib_linux_pthread_condattr_t *a)
{ if (init_failed()) return 1; a->alive = 1; a->clock = 0; ++live_attributes; return 0; }
static inline int lib_linux_pthread_condattr_setclock(lib_linux_pthread_condattr_t *a, int clock)
{ assert(a->alive && clock == LIB_LINUX_CLOCK_MONOTONIC); if (init_failed()) return 1; a->clock = clock; return 0; }
static inline int lib_linux_pthread_condattr_destroy(lib_linux_pthread_condattr_t *a)
{ assert(a->alive); a->alive = 0; --live_attributes; return 0; }
static inline int lib_linux_pthread_cond_init(lib_linux_pthread_cond_t *c, const lib_linux_pthread_condattr_t *a)
{ assert(a && a->alive && a->clock == LIB_LINUX_CLOCK_MONOTONIC); if (init_failed()) return 1; c->alive = 1; ++live_conditions; return 0; }
static inline int lib_linux_pthread_cond_destroy(lib_linux_pthread_cond_t *c)
{ assert(c->alive); c->alive = 0; --live_conditions; return 0; }
static inline int lib_linux_pthread_cond_signal(lib_linux_pthread_cond_t *c)
{ assert(c->alive); return fail_signal; }
static inline int lib_linux_pthread_cond_broadcast(lib_linux_pthread_cond_t *c)
{ return lib_linux_pthread_cond_signal(c); }
static inline int lib_linux_pthread_once(lib_linux_pthread_once_t *once, void (*fn)(void))
{ if (!*once) { *once = 1; fn(); } return 0; }
static inline int lib_linux_pthread_cond_wait(lib_linux_pthread_cond_t *c, lib_linux_pthread_mutex_t *m)
{
    assert(c->alive && m->locked && ++wait_calls < 10);
    lib_linux_pthread_mutex_unlock(m);
    if (wait_hook) wait_hook();
    lib_linux_pthread_mutex_lock(m);
    return wait_result;
}
static inline int lib_linux_pthread_cond_timedwait(lib_linux_pthread_cond_t *c,
    lib_linux_pthread_mutex_t *m, const lib_linux_timespec *deadline)
{
    if (wait_calls) assert(deadline->tv_sec == observed_deadline.tv_sec &&
        deadline->tv_nsec == observed_deadline.tv_nsec);
    observed_deadline = *deadline;
    return lib_linux_pthread_cond_wait(c, m);
}
static inline int lib_linux_clock_gettime(int clock, lib_linux_timespec *value)
{
    assert(clock == LIB_LINUX_CLOCK_MONOTONIC);
    if (clock_failure) return 1;
    *value = (lib_linux_timespec){ 100 + wait_calls, 900000000L };
    return 0;
}
static inline int lib_linux_nanosleep(const lib_linux_timespec *duration, lib_linux_timespec *remaining)
{
    ++sleep_calls;
    if (interrupt_sleep) {
        interrupt_sleep = 0; lib_linux_errno = LIB_LINUX_EINTR;
        *remaining = (lib_linux_timespec){ 0, 1 }; return -1;
    }
    assert(duration->tv_sec == 0 && duration->tv_nsec == 1);
    return 0;
}
static inline int lib_linux_sched_yield(void) { return 0; }
static inline int lib_linux_pthread_create(lib_linux_pthread_t *t, const void *a, void *(*fn)(void *), void *ctx)
{
    (void)a;
    if (fail_thread) return 1;
    *t = 1; thread_entry = fn; thread_context = ctx; return 0;
}
static inline int lib_linux_pthread_join(lib_linux_pthread_t t, void **result)
{
    (void)result; assert(t == 1 && thread_entry); ++thread_joins;
    if (fail_join) return 1;
    thread_entry(thread_context); thread_entry = NULL; return 0;
}
#endif
