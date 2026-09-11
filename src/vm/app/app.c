#include "type.h"

#include <limits.h>

#include "vm/app/app.h"
#include "common/debug/debug_interface.h"
#include "lib/storage/medium_interface.h"
#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/machine_interface.h"
#include "vm/machine/request_factory.h"
#include "vm/presentation/frame.h"

struct vm_app {
    vm_machine *machine;
    common_session *session;
    common_debug *debug;
    vm_machine_debug_observer product_observer;
    C_VOID *product_observer_context;
};

static lib_status vm_app_debug_read_file(void *context, const char *path,
    lib_u64 offset, lib_u8 *bytes, lib_size capacity, lib_size *out_bytes)
{
    lib_storage_medium *medium = LIB_NULL;
    lib_size byte_count;
    lib_status status;

    (void)context;
    if (path == LIB_NULL || bytes == LIB_NULL || out_bytes == LIB_NULL ||
        offset > (lib_u64)SIZE_MAX) return LIB_STATUS_INVALID_ARGUMENT;
    *out_bytes = 0u;
    status = lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_READONLY, &medium);
    if (status != LIB_STATUS_OK) return status;
    byte_count = lib_storage_medium_byte_count(medium);
    if ((lib_size)offset < byte_count) {
        *out_bytes = byte_count - (lib_size)offset < capacity ?
            byte_count - (lib_size)offset : capacity;
        status = lib_storage_medium_read_at(medium, (lib_size)offset, bytes,
            *out_bytes);
    }
    lib_storage_medium_destroy(&medium);
    return status;
}

static lib_status vm_app_debug_write_file(void *context, const char *path,
    lib_u64 offset, const lib_u8 *bytes, lib_size byte_count)
{
    lib_storage_medium *medium = LIB_NULL;
    lib_status status;

    (void)context;
    if (path == LIB_NULL || bytes == LIB_NULL || offset > (lib_u64)SIZE_MAX)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_DIRECT, &medium);
    if (status == LIB_STATUS_OK)
        status = lib_storage_medium_write_at(medium, (lib_size)offset, bytes,
            byte_count);
    lib_storage_medium_destroy(&medium);
    return status;
}

static lib_status vm_app_debug_provider(void *context, const char *line,
    common_session_cli_result *out_result)
{
    vm_app *app = context;
    common_debug_result result;
    lib_status status;

    if (app == STD_NULL || out_result == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = common_debug_submit_line(app->debug, line, &result);
    if (status != LIB_STATUS_OK) return status;
    STD_MEMSET(out_result, 0, sizeof(*out_result));
    STD_MEMCPY(out_result->text, result.text, sizeof(out_result->text));
    STD_MEMCPY(out_result->prompt, result.prompt, sizeof(out_result->prompt));
    out_result->prompt_ready = result.prompt_ready;
    out_result->keep_active = result.keep_active;
    out_result->lifecycle_request = result.lifecycle_request == COMMON_DEBUG_LIFECYCLE_RESUME ?
        COMMON_SESSION_LIFECYCLE_RESUME : result.lifecycle_request == COMMON_DEBUG_LIFECYCLE_STEP ?
        COMMON_SESSION_LIFECYCLE_STEP : result.lifecycle_request == COMMON_DEBUG_LIFECYCLE_STOP ?
        COMMON_SESSION_LIFECYCLE_STOP : COMMON_SESSION_LIFECYCLE_NONE;
    if (!result.keep_active) common_debug_close(app->debug);
    return LIB_STATUS_OK;
}

static lib_status vm_app_debug_lifecycle(void *context,
    common_session_lifecycle_request_kind request)
{
    vm_app *app = context;
    if (app == STD_NULL || app->machine == STD_NULL) return LIB_STATUS_INVALID_STATE;
    switch (request) {
    case COMMON_SESSION_LIFECYCLE_RESUME: return (lib_status)vm_machine_resume(app->machine);
    case COMMON_SESSION_LIFECYCLE_STEP: return (lib_status)vm_machine_request_step(app->machine);
    case COMMON_SESSION_LIFECYCLE_STOP: vm_machine_stop(app->machine); return LIB_STATUS_OK;
    default: return LIB_STATUS_UNSUPPORTED;
    }
}

static C_VOID vm_app_debug_observe(C_VOID *context,
    const vm_machine_debug_observation *observation)
{
    common_debug_instruction_observation copied;
    type_unsigned_8 index;

    if (context == STD_NULL || observation == STD_NULL) return;
    copied.memory_access_count = observation->memory_access_count <
        COMMON_DEBUG_MEMORY_ACCESS_CAPACITY ? observation->memory_access_count :
        COMMON_DEBUG_MEMORY_ACCESS_CAPACITY;
    for (index = 0u; index < copied.memory_access_count; ++index) {
        copied.memory_accesses[index].write = observation->memory_accesses[index].write ?
            LIB_TRUE : LIB_FALSE;
        copied.memory_accesses[index].linear = observation->memory_accesses[index].linear;
        copied.memory_accesses[index].bytes = observation->memory_accesses[index].bytes;
        copied.memory_accesses[index].data = observation->memory_accesses[index].data;
    }
    common_debug_observe_instruction(((vm_app *)context)->debug, &copied);
    if (((vm_app *)context)->product_observer != STD_NULL)
        ((vm_app *)context)->product_observer(
            ((vm_app *)context)->product_observer_context, observation);
}

static common_session_machine_state vm_app_machine_state(vm_machine_result_kind kind)
{
    switch (kind) {
    case VM_MACHINE_RESULT_RUNNING: return COMMON_SESSION_MACHINE_RUNNING;
    case VM_MACHINE_RESULT_PAUSED: return COMMON_SESSION_MACHINE_PAUSED;
    case VM_MACHINE_RESULT_RESET: return COMMON_SESSION_MACHINE_RESET;
    case VM_MACHINE_RESULT_STOPPED: return COMMON_SESSION_MACHINE_STOPPED;
    default: return COMMON_SESSION_MACHINE_FAULT;
    }
}

static lib_status vm_app_debug_machine(void *context,
    common_session_machine_state state, lib_status status,
    common_session_cli_result *out_result)
{
    vm_app *app = context;
    common_debug_result result;
    common_debug_machine_state debug_state;
    lib_status debug_status;

    if (app == STD_NULL || out_result == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    switch (state) {
    case COMMON_SESSION_MACHINE_RUNNING: debug_state = COMMON_DEBUG_MACHINE_RUNNING; break;
    case COMMON_SESSION_MACHINE_PAUSED: debug_state = COMMON_DEBUG_MACHINE_PAUSED; break;
    case COMMON_SESSION_MACHINE_RESET: debug_state = COMMON_DEBUG_MACHINE_RESET; break;
    case COMMON_SESSION_MACHINE_STOPPED: debug_state = COMMON_DEBUG_MACHINE_STOPPED; break;
    default: debug_state = COMMON_DEBUG_MACHINE_FAULT; break;
    }
    debug_status = common_debug_observe_machine(app->debug, debug_state, status,
        &result);
    if (debug_status != LIB_STATUS_OK) return debug_status;
    STD_MEMSET(out_result, 0, sizeof(*out_result));
    STD_MEMCPY(out_result->text, result.text, sizeof(out_result->text));
    STD_MEMCPY(out_result->prompt, result.prompt, sizeof(out_result->prompt));
    out_result->prompt_ready = result.prompt_ready;
    out_result->keep_active = result.keep_active;
    out_result->lifecycle_request = result.lifecycle_request ==
        COMMON_DEBUG_LIFECYCLE_RESUME ? COMMON_SESSION_LIFECYCLE_RESUME :
        result.lifecycle_request == COMMON_DEBUG_LIFECYCLE_STEP ?
        COMMON_SESSION_LIFECYCLE_STEP : result.lifecycle_request ==
        COMMON_DEBUG_LIFECYCLE_STOP ? COMMON_SESSION_LIFECYCLE_STOP :
        COMMON_SESSION_LIFECYCLE_NONE;
    return LIB_STATUS_OK;
}

static C_VOID vm_app_machine_result(void *context, const vm_machine_result *result)
{
    vm_app *app = context;
    ui_frame frame;

    if (app == STD_NULL || result == STD_NULL) return;
    if (result->kind == VM_MACHINE_RESULT_DISPLAY) {
        if (vm_presentation_frame_from_core(&result->value.display, &frame) ==
            TYPE_STATUS_OK) (C_VOID)common_session_publish_frame(app->session, &frame);
        return;
    }
    (C_VOID)common_session_publish_machine(app->session,
        vm_app_machine_state(result->kind), (lib_status)result->status);
}

type_status vm_app_create(vm_app **out_app)
{
    vm_app *app;

    if (out_app == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_app = STD_NULL;
    app = STD_CALLOC(1u, sizeof(*app));
    if (app == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (common_session_create(&app->session) != LIB_STATUS_OK) {
        STD_FREE(app);
        return TYPE_STATUS_NO_MEMORY;
    }
    if (common_debug_create(&app->debug) != LIB_STATUS_OK) {
        common_session_destroy(app->session);
        STD_FREE(app);
        return TYPE_STATUS_NO_MEMORY;
    }
    (C_VOID)common_session_set_lifecycle_sink(app->session,
        vm_app_debug_lifecycle, app);
    *out_app = app;
    return TYPE_STATUS_OK;
}

C_VOID vm_app_destroy(vm_app *app)
{
    if (app == STD_NULL) return;
    vm_machine_destroy(app->machine);
    common_debug_destroy(app->debug);
    common_session_destroy(app->session);
    STD_FREE(app);
}

common_session *vm_app_session(vm_app *app)
{ return app == STD_NULL ? LIB_NULL : app->session; }

type_status vm_app_open_profile(vm_app *app, const vm_session_request *request)
{
    if (app == STD_NULL || request == STD_NULL || app->machine != STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    if (vm_machine_create_from_request(request, &app->machine) != TYPE_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    vm_machine_set_result_sink(app->machine, vm_app_machine_result, app);
    vm_machine_bind_debug_observer(app->machine, vm_app_debug_observe, app);
    return TYPE_STATUS_OK;
}

type_status vm_app_set_presentation_target(vm_app *app, common_session_target target)
{
    return app == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        (type_status)common_session_set_target(app->session, target);
}

void vm_app_bind_product_debug_observer(vm_app *app,
    vm_machine_debug_observer observer, C_VOID *context)
{
    if (app == STD_NULL) return;
    app->product_observer = observer;
    app->product_observer_context = context;
}

C_INT vm_app_is_running(const vm_app *app)
{ return app != STD_NULL && app->machine != STD_NULL && vm_machine_is_running(app->machine); }

C_VOID vm_app_print_machine(const vm_app *app)
{ if (app != STD_NULL && app->machine != STD_NULL) vm_machine_print_machine(app->machine); }
C_VOID vm_app_print_bios(const vm_app *app)
{ if (app != STD_NULL && app->machine != STD_NULL) vm_machine_print_bios(app->machine); }
C_VOID vm_app_print_status(const vm_app *app)
{ if (app != STD_NULL && app->machine != STD_NULL) vm_machine_print_status(app->machine); }

type_status vm_app_get_speed(const vm_app *app, vm_app_speed *out_speed)
{
    vm_machine_speed speed;
    if (app == STD_NULL || app->machine == STD_NULL || out_speed == STD_NULL ||
        vm_machine_get_speed(app->machine, &speed) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    *out_speed = speed == VM_MACHINE_SPEED_TURBO ? VM_APP_SPEED_TURBO : VM_APP_SPEED_STANDARD;
    return TYPE_STATUS_OK;
}

type_status vm_app_set_speed(vm_app *app, vm_app_speed speed)
{
    if (app == STD_NULL || app->machine == STD_NULL ||
        (speed != VM_APP_SPEED_STANDARD && speed != VM_APP_SPEED_TURBO)) return TYPE_STATUS_INVALID_STATE;
    return vm_machine_set_speed(app->machine, speed == VM_APP_SPEED_TURBO ?
        VM_MACHINE_SPEED_TURBO : VM_MACHINE_SPEED_STANDARD);
}

type_status vm_app_debug(vm_app *app)
{
    if (app == STD_NULL || app->machine == STD_NULL || app->debug == STD_NULL ||
        vm_machine_pause_for_debug(app->machine, 2000u) != TYPE_STATUS_OK ||
        common_debug_open(app->debug, vm_machine_common_machine(app->machine),
            &(common_debug_file_service){
                vm_app_debug_read_file, vm_app_debug_write_file, app
            }) != LIB_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    if (common_session_set_cli_provider(app->session, vm_app_debug_provider,
            app) != LIB_STATUS_OK ||
        common_session_set_cli_machine_observer(app->session,
            vm_app_debug_machine, app) != LIB_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    return TYPE_STATUS_OK;
}
C_INT vm_app_insert_fdd(vm_app *app, const C_CHAR *path)
{ return app == STD_NULL || app->machine == STD_NULL ? -1 : vm_machine_insert_fdd(app->machine, path); }
C_INT vm_app_remove_fdd(vm_app *app, const C_CHAR *path)
{ return app == STD_NULL || app->machine == STD_NULL ? -1 : vm_machine_remove_fdd(app->machine, path); }

static type_status vm_app_begin(vm_app *app, common_session_plan *out_plan, C_INT resume)
{
    lib_u32 run_id;

    if (app == STD_NULL || app->machine == STD_NULL || out_plan == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    run_id = common_session_begin_run(app->session, out_plan);
    if (run_id == 0u || vm_machine_bind_run(app->machine, run_id) != TYPE_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    return resume ? vm_machine_resume(app->machine) : vm_machine_start(app->machine);
}
type_status vm_app_start(vm_app *app, common_session_plan *out_plan)
{ return vm_app_begin(app, out_plan, TYPE_FALSE); }
type_status vm_app_resume(vm_app *app, common_session_plan *out_plan)
{ return vm_app_begin(app, out_plan, TYPE_TRUE); }
type_status vm_app_reset(vm_app *app)
{ return app == STD_NULL || app->machine == STD_NULL ? TYPE_STATUS_INVALID_STATE : vm_machine_reset(app->machine); }
type_status vm_app_stop(vm_app *app)
{ if (app == STD_NULL || app->machine == STD_NULL) return TYPE_STATUS_INVALID_STATE; vm_machine_stop(app->machine); return TYPE_STATUS_OK; }
type_status vm_app_request_pause(vm_app *app)
{ return app == STD_NULL || app->machine == STD_NULL ? TYPE_STATUS_INVALID_STATE : vm_machine_request_pause(app->machine); }

static C_INT vm_app_key_scan(ui_key key, type_unsigned_16 *scan)
{
    static const struct { ui_key key; type_unsigned_16 scan; } map[] = {
        { UI_KEY_ENTER, 0x1cu }, { UI_KEY_BACKSPACE, 0x0eu }, { UI_KEY_F1, 0x3bu },
        { UI_KEY_F2, 0x3cu }, { UI_KEY_F3, 0x3du }, { UI_KEY_F4, 0x3eu },
        { UI_KEY_F5, 0x3fu }, { UI_KEY_F6, 0x40u }, { UI_KEY_F7, 0x41u },
        { UI_KEY_F8, 0x42u }, { UI_KEY_F9, 0x43u }, { UI_KEY_F10, 0x44u },
        { UI_KEY_F11, 0x57u }, { UI_KEY_F12, 0x58u }, { UI_KEY_UP, 0x48u },
        { UI_KEY_DOWN, 0x50u }, { UI_KEY_LEFT, 0x4bu }, { UI_KEY_RIGHT, 0x4du },
        { UI_KEY_HOME, 0x47u }, { UI_KEY_END, 0x4fu }, { UI_KEY_PAGE_UP, 0x49u },
        { UI_KEY_PAGE_DOWN, 0x51u }, { UI_KEY_INSERT, 0x52u }, { UI_KEY_DELETE, 0x53u }
    };
    STD_SIZE_T index;
    if (scan == STD_NULL) return TYPE_FALSE;
    for (index = 0u; index < sizeof(map) / sizeof(map[0]); ++index)
        if (map[index].key == key) { *scan = map[index].scan; return TYPE_TRUE; }
    return TYPE_FALSE;
}

type_status vm_app_submit_ui_input(vm_app *app, const ui_input_event *event)
{
    vm_machine_input input = {0};
    type_unsigned_16 scan = 0u;
    if (app == STD_NULL || app->machine == STD_NULL || event == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (event->type == UI_EVENT_KEY) {
        scan = event->data.key.scan_code;
        if (scan == 0u && !vm_app_key_scan(event->data.key.key, &scan)) return TYPE_STATUS_UNSUPPORTED;
        input.kind = VM_MACHINE_INPUT_KEY_EVENT;
        input.data.key_event.scan_code = scan;
        input.data.key_event.virtual_key = (type_unsigned_16)event->data.key.key;
        input.data.key_event.pressed = event->data.key.pressed;
    } else if (event->type == UI_EVENT_TEXT && event->data.text.scalar <= 0xffffu) {
        input.kind = VM_MACHINE_INPUT_KEY_EVENT; input.data.key_event.virtual_key = (type_unsigned_16)event->data.text.scalar; input.data.key_event.pressed = TYPE_TRUE;
    } else if (event->type == UI_EVENT_MOUSE) {
        input.kind = VM_MACHINE_INPUT_MOUSE_EVENT;
        input.data.mouse_event.delta_x = event->data.mouse.delta_x < INT16_MIN ? INT16_MIN : event->data.mouse.delta_x > INT16_MAX ? INT16_MAX : event->data.mouse.delta_x;
        input.data.mouse_event.delta_y = event->data.mouse.delta_y < INT16_MIN ? INT16_MIN : event->data.mouse.delta_y > INT16_MAX ? INT16_MAX : event->data.mouse.delta_y;
        input.data.mouse_event.buttons = (event->data.mouse.buttons & UI_MOUSE_BUTTON_LEFT ? 1u : 0u) | (event->data.mouse.buttons & UI_MOUSE_BUTTON_RIGHT ? 2u : 0u);
    } else return TYPE_STATUS_UNSUPPORTED;
    return vm_machine_submit_input(app->machine, &input);
}

type_status vm_app_submit_key(vm_app *app, type_unsigned_16 scan_code,
    type_unsigned_16 virtual_key, C_INT pressed)
{
    vm_machine_input input = {0};
    if (app == STD_NULL || app->machine == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    input.kind = VM_MACHINE_INPUT_KEY_EVENT; input.data.key_event.scan_code = scan_code;
    input.data.key_event.virtual_key = virtual_key; input.data.key_event.pressed = pressed;
    return vm_machine_submit_input(app->machine, &input);
}

static lib_status vm_app_input_sink(void *context, const ui_input_event *event)
{ return (lib_status)vm_app_submit_ui_input(context, event); }

static C_VOID vm_app_submit_chord(vm_app *app, C_INT cad)
{
    const type_unsigned_16 scan[] = { cad ? 0x1du : 0x38u, 0x38u, cad ? 0x153u : 0u };
    const type_unsigned_16 key[] = { cad ? 0x11u : 0x12u, 0x12u, cad ? 0x2eu : 0u };
    const type_unsigned_32 count = cad ? 3u : 2u; type_unsigned_32 index;
    for (index = 0u; index < count; ++index) (C_VOID)vm_app_submit_key(app, scan[index], key[index], TYPE_TRUE);
    for (index = count; index-- != 0u;) (C_VOID)vm_app_submit_key(app, scan[index], key[index], TYPE_FALSE);
}

type_status vm_app_reduce_input(vm_app *app, const ui_input_event *event,
    common_session_plan *out_plan)
{
    const C_CHAR *name;
    if (app == STD_NULL || event == STD_NULL || out_plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (event->type == UI_EVENT_HOTKEY) {
        name = event->data.hotkey.identifier;
        if (!STD_STRCMP(name, "pause")) return vm_app_is_running(app) ? vm_app_request_pause(app) : vm_app_resume(app, out_plan);
        if (!STD_STRCMP(name, "release-mouse")) { out_plan->release_mouse = LIB_TRUE; return TYPE_STATUS_OK; }
        if (!STD_STRCMP(name, "cad") || !STD_STRCMP(name, "alt-enter")) { vm_app_submit_chord(app, !STD_STRCMP(name, "cad")); return TYPE_STATUS_OK; }
        return TYPE_STATUS_OK;
    }
    if (event->type == UI_EVENT_WINDOW_CLOSE) {
        if (vm_app_is_running(app)) (C_VOID)vm_app_request_pause(app);
        (C_VOID)common_session_set_target(app->session, COMMON_SESSION_TARGET_NONE);
        out_plan->target_changed = LIB_TRUE; out_plan->target = COMMON_SESSION_TARGET_NONE;
        out_plan->mouse_capturable_changed = LIB_TRUE; out_plan->mouse_capturable = LIB_FALSE; out_plan->release_mouse = LIB_TRUE;
        return TYPE_STATUS_OK;
    }
    return (type_status)common_session_dispatch_host_input(app->session, event,
        vm_app_input_sink, app);
}
