#include "ibmpc/product/entry.c"

struct vm_app { lib_bool live; };
struct vm_app_console_context { lib_bool live; };

static struct vm_app app;
static struct vm_app_console_context console;
static lib_u32 failure;
static lib_u32 cleanup;
static lib_u32 create_count;

lib_status vm_app_ini_executable_path(lib_u8 *out_path, lib_size capacity)
{
    if (failure == 1u) return LIB_STATUS_IO_ERROR;
    if (capacity < 9u) return LIB_STATUS_LIMIT_EXCEEDED;
    lib_memory_copy(out_path, "NXVM.ini", 9u);
    return LIB_STATUS_OK;
}

lib_status vm_app_create(const vm_app_factory *factory, vm_app **out_app)
{
    (void)factory;
    ++create_count;
    if (failure == 2u) return LIB_STATUS_NO_MEMORY;
    app.live = LIB_TRUE;
    *out_app = &app;
    return LIB_STATUS_OK;
}

lib_status vm_app_destroy(vm_app *session)
{
    if (session == LIB_NULL) return LIB_STATUS_OK;
    cleanup = cleanup * 10u + 2u;
    if (failure == 5u) return LIB_STATUS_INTERNAL_ERROR;
    session->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status vm_app_console_context_create(vm_app_console_context **out_context)
{
    if (failure == 3u) return LIB_STATUS_NO_MEMORY;
    console.live = LIB_TRUE;
    *out_context = &console;
    return LIB_STATUS_OK;
}

void vm_app_console_context_destroy(vm_app_console_context *context)
{
    context->live = LIB_FALSE;
    cleanup = cleanup * 10u + 1u;
}

lib_status vm_app_console_main(vm_app_console_context *context,
    vm_app *session, const lib_u8 *path)
{
    if (!context->live || !session->live ||
        lib_text_compare((const char *)path, "NXVM.ini") != 0)
        return LIB_STATUS_INVALID_STATE;
    return failure == 4u ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

lib_i32 main(void)
{
    const vm_app_definition definition = {
        .name = "PC", .version = "test", .copyright = "fixture", .build_time = "fixed"
    };

    if (vm_app_run(LIB_NULL) != 1) return 1;
    for (failure = 0u; failure <= 5u; ++failure) {
        lib_i32 result;
        app.live = console.live = LIB_FALSE;
        cleanup = create_count = 0u;
        result = vm_app_run(&definition);
        if (result != (failure == 0u ? 0 : 1) || console.live) return 1;
        if (failure == 1u && create_count != 0u) return 1;
        if ((failure == 1u || failure == 2u) && cleanup != 0u) return 1;
        if (failure == 3u && cleanup != 2u) return 1;
        if ((failure == 0u || failure >= 4u) && cleanup != 12u) return 1;
        if (app.live != (failure == 5u ? LIB_TRUE : LIB_FALSE)) return 1;
    }
    return 0;
}
