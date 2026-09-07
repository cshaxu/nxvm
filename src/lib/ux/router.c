#include "router.h"

#include <string.h>

static void ux_router_lock(ux_router *router)
{
    while (atomic_flag_test_and_set_explicit(&router->title_lock,
        memory_order_acquire)) {}
}

static void ux_router_unlock(ux_router *router)
{
    atomic_flag_clear_explicit(&router->title_lock, memory_order_release);
}

void ux_router_initialize(ux_router *router, ux_target target)
{
    if (router == LIB_NULL) return;
    atomic_init(&router->target, (unsigned int)target);
    atomic_init(&router->active_target, (unsigned int)UX_TARGET_NONE);
    router->title_lock = (atomic_flag)ATOMIC_FLAG_INIT;
    atomic_flag_clear_explicit(&router->title_lock, memory_order_release);
    router->title_generation = 0u;
    router->window_title[0] = '\0';
}

ux_target ux_router_target(const ux_router *router)
{
    return router == LIB_NULL ? UX_TARGET_NONE :
        (ux_target)atomic_load(&router->target);
}

void ux_router_request(ux_router *router, ux_target target)
{
    if (router != LIB_NULL) atomic_store(&router->target, (unsigned int)target);
}

void ux_router_set_active_target(ux_router *router, ux_target target)
{
    if (router == LIB_NULL) return;
    ux_router_lock(router);
    atomic_store(&router->active_target, (unsigned int)target);
    if (target != UX_TARGET_WINDOW) {
        router->title_generation = 0u;
        router->window_title[0] = '\0';
    }
    ux_router_unlock(router);
}

ux_target ux_router_active_target(const ux_router *router)
{
    return router == LIB_NULL ? UX_TARGET_NONE :
        (ux_target)atomic_load(&router->active_target);
}

lib_bool ux_router_request_window_title(ux_router *router, const char *title)
{
    size_t length;

    if (router == LIB_NULL || title == LIB_NULL) return LIB_FALSE;
    ux_router_lock(router);
    if ((ux_target)atomic_load(&router->active_target) != UX_TARGET_WINDOW) {
        ux_router_unlock(router);
        return LIB_FALSE;
    }
    length = strlen(title);
    if (length >= UX_WINDOW_TITLE_CAPACITY) length = UX_WINDOW_TITLE_CAPACITY - 1u;
    if (strlen(router->window_title) == length &&
        memcmp(router->window_title, title, length) == 0) {
        ux_router_unlock(router);
        return LIB_FALSE;
    }
    memcpy(router->window_title, title, length);
    router->window_title[length] = '\0';
    ++router->title_generation;
    ux_router_unlock(router);
    return LIB_TRUE;
}

lib_status ux_router_capture_window_title(const ux_router *router,
    char *buffer, lib_u32 buffer_size, lib_u32 *out_generation)
{
    ux_router *mutable_router = (ux_router *)router;
    size_t length;

    if (router == LIB_NULL || buffer == LIB_NULL || buffer_size == 0u ||
        out_generation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ux_router_lock(mutable_router);
    length = strlen(router->window_title);
    if (length >= buffer_size) length = buffer_size - 1u;
    memcpy(buffer, router->window_title, length);
    buffer[length] = '\0';
    *out_generation = router->title_generation;
    ux_router_unlock(mutable_router);
    return LIB_STATUS_OK;
}
