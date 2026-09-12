/* Copyright 2012-2014 Neko. */

/*
 * Console provides a command-line interface for users
 * to configure, debug and run the virtual machine.
 */

#include "type.h"

#include <limits.h>

#include "vm/product/console.h"
#include "vm/app/app.h"
#include "common/debug/debug_interface.h"
#include "common/ui/ui_interface.h"
#include "vm/product/catalog.h"
#include "vm/product/recorder.h"
#include "common/session/session_interface.h"
#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/machine_interface.h"

struct vm_product_console_context {
    STD_SIZE_T argument_count;
    C_CHAR **arguments;
    C_INT exit_requested;
    C_CHAR command_buffer[0x100];
    vm_app *session;
    vm_product_session_catalog *catalog;
    common_session *control;
    common_debug *debug;
    vm_product_recorder *recorder;
    C_INT session_stopped;
};
#define CONSOLE_MAXNARG 256

#define consoleContext context

#define numArgs (consoleContext->argument_count)
#define argArray (consoleContext->arguments)
#define flagExit (consoleContext->exit_requested)
#define strCmdBuff (consoleContext->command_buffer)
#define currentSession (consoleContext->session)

static common_ui *vm_product_console_ui(
    const vm_product_console_context *context);

static C_INT vm_product_console_printf(const vm_product_console_context *context,
    const C_CHAR *format, ...)
{
    C_CHAR text[4096];
    STD_VA_LIST arguments;
    C_INT written;

    if (context == STD_NULL || format == STD_NULL) return -1;
    va_start(arguments, format);
    written = vsnprintf(text, sizeof(text), format, arguments);
    va_end(arguments);
    if (written < 0 || (STD_SIZE_T)written >= sizeof(text)) return -1;
    return common_ui_write_console(vm_app_ui(context->session), text) ==
        LIB_STATUS_OK ? written : -1;
}

static lib_status vm_product_console_line_sink(void *context,
    const C_CHAR *line)
{
    return common_session_publish_console_line(context, line);
}

static lib_status vm_product_console_input_sink(void *context,
    const ui_input_event *event)
{
    return common_session_publish_ui_input(context, event);
}

static vm_machine *vm_product_console_machine(
    const vm_product_console_context *context)
{
    return context == STD_NULL ? STD_NULL : vm_app_machine(context->session);
}

static common_ui *vm_product_console_ui(
    const vm_product_console_context *context)
{
    return context == STD_NULL ? LIB_NULL : vm_app_ui(context->session);
}

static C_INT vm_product_console_is_running(
    const vm_product_console_context *context)
{
    vm_machine *machine = vm_product_console_machine(context);
    return machine != STD_NULL && vm_machine_is_running(machine);
}

static common_debug_machine_state vm_product_console_debug_state(
    common_session_machine_state state)
{
    switch (state) {
    case COMMON_SESSION_MACHINE_RUNNING: return COMMON_DEBUG_MACHINE_RUNNING;
    case COMMON_SESSION_MACHINE_PAUSED: return COMMON_DEBUG_MACHINE_PAUSED;
    case COMMON_SESSION_MACHINE_RESET: return COMMON_DEBUG_MACHINE_RESET;
    case COMMON_SESSION_MACHINE_STOPPED: return COMMON_DEBUG_MACHINE_STOPPED;
    default: return COMMON_DEBUG_MACHINE_FAULT;
    }
}

static common_session_lifecycle_request_kind
vm_product_console_debug_lifecycle_request(
    common_debug_lifecycle_request request)
{
    switch (request) {
    case COMMON_DEBUG_LIFECYCLE_RESUME: return COMMON_SESSION_LIFECYCLE_RESUME;
    case COMMON_DEBUG_LIFECYCLE_STEP: return COMMON_SESSION_LIFECYCLE_STEP;
    case COMMON_DEBUG_LIFECYCLE_STOP: return COMMON_SESSION_LIFECYCLE_STOP;
    default: return COMMON_SESSION_LIFECYCLE_NONE;
    }
}

static void vm_product_console_debug_result(common_session_cli_result *destination,
    const common_debug_result *source)
{
    if (destination == LIB_NULL || source == LIB_NULL) return;
    STD_MEMSET(destination, 0, sizeof(*destination));
    STD_MEMCPY(destination->text, source->text, sizeof(destination->text));
    STD_MEMCPY(destination->prompt, source->prompt, sizeof(destination->prompt));
    destination->prompt_ready = source->prompt_ready;
    destination->keep_active = source->keep_active;
    destination->lifecycle_request =
        vm_product_console_debug_lifecycle_request(source->lifecycle_request);
}

static lib_status vm_product_console_debug_provider(void *opaque,
    const char *line, common_session_cli_result *out_result)
{
    vm_product_console_context *context = opaque;
    common_debug_result result;
    lib_status status;

    if (context == STD_NULL || context->debug == STD_NULL || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = common_debug_submit_line(context->debug, line, &result);
    if (status != LIB_STATUS_OK) return status;
    vm_product_console_debug_result(out_result, &result);
    if (!result.keep_active) common_debug_close(context->debug);
    return LIB_STATUS_OK;
}

static lib_status vm_product_console_debug_machine(void *opaque,
    common_session_machine_state state, lib_status status,
    common_session_cli_result *out_result)
{
    vm_product_console_context *context = opaque;
    common_debug_result result;
    lib_status debug_status;

    if (context == STD_NULL || context->debug == STD_NULL || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    debug_status = common_debug_observe_machine(context->debug,
        vm_product_console_debug_state(state), status, &result);
    if (debug_status != LIB_STATUS_OK) return debug_status;
    vm_product_console_debug_result(out_result, &result);
    return LIB_STATUS_OK;
}

static C_VOID vm_product_console_debug_observe(C_VOID *opaque,
    const vm_machine_debug_observation *observation)
{
    vm_product_console_context *context = opaque;
    common_debug_instruction_observation copied = {0};
    type_unsigned_8 index;

    if (context == STD_NULL || context->debug == STD_NULL || observation == STD_NULL)
        return;
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
    common_debug_observe_instruction(context->debug, &copied);
    if (context->recorder != STD_NULL)
        vm_product_recorder_observe(context->recorder, observation);
}

static lib_status vm_product_console_lifecycle(void *opaque,
    common_session_lifecycle_request_kind request)
{
    vm_machine *machine = vm_product_console_machine(opaque);

    if (machine == STD_NULL) return LIB_STATUS_INVALID_STATE;
    switch (request) {
    case COMMON_SESSION_LIFECYCLE_RESUME: return (lib_status)vm_machine_resume(machine);
    case COMMON_SESSION_LIFECYCLE_STEP: return (lib_status)vm_machine_request_step(machine);
    case COMMON_SESSION_LIFECYCLE_STOP: vm_machine_stop(machine); return LIB_STATUS_OK;
    default: return LIB_STATUS_UNSUPPORTED;
    }
}

static type_status vm_product_console_begin(vm_product_console_context *context,
    common_session_plan *out_plan, C_INT resume)
{
    vm_machine *machine = vm_product_console_machine(context);
    lib_u32 run_id;

    if (machine == STD_NULL || context == STD_NULL || out_plan == STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    run_id = common_session_begin_run(context->control, out_plan);
    if (run_id == 0u || vm_machine_bind_run(machine, run_id) != TYPE_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    return resume ? vm_machine_resume(machine) : vm_machine_start(machine);
}

static type_status vm_product_console_start(vm_product_console_context *context,
    common_session_plan *out_plan)
{ return vm_product_console_begin(context, out_plan, TYPE_FALSE); }

static type_status vm_product_console_resume(vm_product_console_context *context,
    common_session_plan *out_plan)
{ return vm_product_console_begin(context, out_plan, TYPE_TRUE); }

static type_status vm_product_console_request_pause(vm_product_console_context *context)
{
    vm_machine *machine = vm_product_console_machine(context);
    return machine == STD_NULL ? TYPE_STATUS_INVALID_STATE :
        vm_machine_request_pause(machine);
}

static type_status vm_product_console_stop(vm_product_console_context *context)
{
    vm_machine *machine = vm_product_console_machine(context);
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    vm_machine_stop(machine);
    return TYPE_STATUS_OK;
}

static C_INT vm_product_console_key_scan(ui_key key, type_unsigned_16 *scan)
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
        if (map[index].key == key) {
            *scan = map[index].scan;
            return TYPE_TRUE;
        }
    return TYPE_FALSE;
}

static type_status vm_product_console_submit_key(vm_product_console_context *context,
    type_unsigned_16 scan_code, type_unsigned_16 virtual_key, C_INT pressed)
{
    vm_machine_input input = {0};
    vm_machine *machine = vm_product_console_machine(context);

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    input.kind = VM_MACHINE_INPUT_KEY_EVENT;
    input.data.key_event.scan_code = scan_code;
    input.data.key_event.virtual_key = virtual_key;
    input.data.key_event.pressed = pressed;
    return vm_machine_submit_input(machine, &input);
}

static type_status vm_product_console_submit_input(vm_product_console_context *context,
    const ui_input_event *event)
{
    vm_machine_input input = {0};
    vm_machine *machine = vm_product_console_machine(context);
    type_unsigned_16 scan = 0u;

    if (machine == STD_NULL || event == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (event->type == UI_EVENT_KEY) {
        scan = event->data.key.scan_code;
        if (scan == 0u && !vm_product_console_key_scan(event->data.key.key, &scan))
            return TYPE_STATUS_UNSUPPORTED;
        input.kind = VM_MACHINE_INPUT_KEY_EVENT;
        input.data.key_event.scan_code = scan;
        input.data.key_event.virtual_key = (type_unsigned_16)event->data.key.key;
        input.data.key_event.pressed = event->data.key.pressed;
    } else if (event->type == UI_EVENT_TEXT && event->data.text.scalar <= 0xffffu) {
        input.kind = VM_MACHINE_INPUT_KEY_EVENT;
        input.data.key_event.virtual_key = (type_unsigned_16)event->data.text.scalar;
        input.data.key_event.pressed = TYPE_TRUE;
    } else if (event->type == UI_EVENT_MOUSE) {
        input.kind = VM_MACHINE_INPUT_MOUSE_EVENT;
        input.data.mouse_event.delta_x = event->data.mouse.delta_x < INT16_MIN ? INT16_MIN :
            event->data.mouse.delta_x > INT16_MAX ? INT16_MAX : event->data.mouse.delta_x;
        input.data.mouse_event.delta_y = event->data.mouse.delta_y < INT16_MIN ? INT16_MIN :
            event->data.mouse.delta_y > INT16_MAX ? INT16_MAX : event->data.mouse.delta_y;
        input.data.mouse_event.buttons =
            (event->data.mouse.buttons & UI_MOUSE_BUTTON_LEFT ? 1u : 0u) |
            (event->data.mouse.buttons & UI_MOUSE_BUTTON_RIGHT ? 2u : 0u);
    } else return TYPE_STATUS_UNSUPPORTED;
    return vm_machine_submit_input(machine, &input);
}

static lib_status vm_product_console_input_dispatch(void *opaque,
    const ui_input_event *event)
{ return (lib_status)vm_product_console_submit_input(opaque, event); }

static C_VOID vm_product_console_submit_chord(vm_product_console_context *context,
    C_INT cad)
{
    const type_unsigned_16 scan[] = { cad ? 0x1du : 0x38u, 0x38u,
        cad ? 0x153u : 0u };
    const type_unsigned_16 key[] = { cad ? 0x11u : 0x12u, 0x12u,
        cad ? 0x2eu : 0u };
    const type_unsigned_32 count = cad ? 3u : 2u;
    type_unsigned_32 index;

    for (index = 0u; index < count; ++index)
        (C_VOID)vm_product_console_submit_key(context, scan[index], key[index], TYPE_TRUE);
    for (index = count; index-- != 0u;)
        (C_VOID)vm_product_console_submit_key(context, scan[index], key[index], TYPE_FALSE);
}

static type_status vm_product_console_reduce_input(vm_product_console_context *context,
    const ui_input_event *event, common_session_plan *out_plan)
{
    const C_CHAR *name;

    if (context == STD_NULL || event == STD_NULL || out_plan == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (event->type == UI_EVENT_HOTKEY) {
        name = event->data.hotkey.identifier;
        if (!STD_STRCMP(name, "pause"))
            return vm_product_console_is_running(context) ?
                vm_product_console_request_pause(context) :
                vm_product_console_resume(context, out_plan);
        if (!STD_STRCMP(name, "release-mouse")) {
            out_plan->release_mouse = LIB_TRUE;
            return TYPE_STATUS_OK;
        }
        if (!STD_STRCMP(name, "cad") || !STD_STRCMP(name, "alt-enter")) {
            vm_product_console_submit_chord(context, !STD_STRCMP(name, "cad"));
            return TYPE_STATUS_OK;
        }
        return TYPE_STATUS_OK;
    }
    if (event->type == UI_EVENT_WINDOW_CLOSE) {
        if (vm_product_console_is_running(context))
            (C_VOID)vm_product_console_request_pause(context);
        (C_VOID)common_session_set_target(context->control, COMMON_SESSION_TARGET_NONE);
        out_plan->target_changed = LIB_TRUE;
        out_plan->target = COMMON_SESSION_TARGET_NONE;
        out_plan->mouse_capturable_changed = LIB_TRUE;
        out_plan->mouse_capturable = LIB_FALSE;
        out_plan->release_mouse = LIB_TRUE;
        return TYPE_STATUS_OK;
    }
    return (type_status)common_session_dispatch_host_input(context->control, event,
        vm_product_console_input_dispatch, context);
}

static void vm_product_console_ui_failure(void *context, lib_u64 source_identity,
    lib_status status)
{
    vm_product_console_context *console = context;

    (void)source_identity;
    if (console != STD_NULL)
        (void)vm_product_console_printf(console, "UI failure: %d.\n", (C_INT)status);
}

static common_ui_target vm_product_console_ui_target(common_session_target target)
{
    return target == COMMON_SESSION_TARGET_WINDOW ? COMMON_UI_TARGET_WINDOW :
        target == COMMON_SESSION_TARGET_CONSOLE ? COMMON_UI_TARGET_CONSOLE :
        COMMON_UI_TARGET_NONE;
}

static void vm_product_console_apply_ui_plan(vm_product_console_context *context,
    const common_session_plan *plan)
{
    common_ui_plan ui_plan = {0};

    if (context == STD_NULL || vm_product_console_ui(context) == LIB_NULL || plan == STD_NULL)
        return;
    ui_plan.target_changed = plan->target_changed;
    ui_plan.target = vm_product_console_ui_target(plan->target);
    ui_plan.mouse_capturable_changed = plan->mouse_capturable_changed;
    ui_plan.mouse_capturable = plan->mouse_capturable;
    ui_plan.release_mouse = plan->release_mouse;
    ui_plan.frame_ready = plan->frame_ready;
    ui_plan.frame = plan->frame;
    (void)common_ui_apply(vm_product_console_ui(context), &ui_plan);
}

static C_VOID vm_product_console_apply_plan(vm_product_console_context *context,
    const common_session_plan *plan, C_INT restore_prompt)
{
    const C_CHAR *state = STD_NULL;

    if (context == STD_NULL || plan == STD_NULL) return;
    vm_product_console_apply_ui_plan(context, plan);
    switch (plan->notice) {
    case COMMON_SESSION_NOTICE_STARTED: state = "started"; break;
    case COMMON_SESSION_NOTICE_RESUMED: state = "resumed"; break;
    case COMMON_SESSION_NOTICE_PAUSED: state = "paused"; break;
    case COMMON_SESSION_NOTICE_RESET: state = "reset"; break;
    case COMMON_SESSION_NOTICE_STOPPED: state = "stopped"; break;
    default: break;
    }
    if (vm_product_console_ui(context) != LIB_NULL && state != STD_NULL &&
        (plan->notice == COMMON_SESSION_NOTICE_STARTED ||
         plan->notice == COMMON_SESSION_NOTICE_RESUMED ||
         plan->notice == COMMON_SESSION_NOTICE_PAUSED))
        (void)common_ui_set_window_title(vm_product_console_ui(context),
            plan->notice == COMMON_SESSION_NOTICE_PAUSED ? "NXVM (Paused)" :
            "NXVM (Running)");
    if (state != STD_NULL) (C_VOID)vm_product_console_printf(context, restore_prompt ?
        "\r\nMachine %s.\nConsole> " : "Machine %s.\n", state);
    if (plan->console_text[0] != '\0')
        (C_VOID)vm_product_console_printf(context, "%s", plan->console_text);
    if (plan->console_prompt_ready)
        (C_VOID)vm_product_console_printf(context, "%s", plan->console_prompt);
}

static C_VOID vm_product_console_drain_lifecycle(vm_product_console_context *context,
    C_INT restore_prompt)
{
    common_session_fact fact;
    ui_frame display;
    common_session_plan plan;

    if (context == STD_NULL) return;
    while (common_session_take(context->control, &fact, &display, 0u) == TYPE_STATUS_OK) {
        if (common_session_reduce_fact(context->control, &fact, &display, &plan) ==
            LIB_STATUS_OK) vm_product_console_apply_plan(context, &plan, restore_prompt);
        if (fact.kind == COMMON_SESSION_FACT_UI_INPUT &&
            vm_product_console_reduce_input(context, &fact.value.input, &plan) == TYPE_STATUS_OK)
            vm_product_console_apply_plan(context, &plan, restore_prompt);
    }
}


#define STD_PRINTF(...) vm_product_console_printf(context, __VA_ARGS__)

/*
 * Parses command-line input.
 *
 * strCmdBuff [IN]  String buffer of command-line input
 * numArgs       [OUT] Number of argArrayuments
 * argArray        [OUT] Array of argArrayuments
 */
static C_VOID parse(vm_product_console_context *context)
{
    numArgs = 0;
    argArray[numArgs] = STD_STRTOK(strCmdBuff, " \t\n\r\f");
    if (!argArray[numArgs])
    {
        return;
    }
    type_string_lower(argArray[numArgs++]);
    while (numArgs < CONSOLE_MAXNARG)
    {
        argArray[numArgs] = STD_STRTOK(STD_NULL, " \t\n\r\f");
        if (argArray[numArgs])
        {
            type_string_lower(argArray[numArgs++]);
        }
        else
        {
            break;
        }
    }
}

static C_INT vm_product_console_read_line(vm_product_console_context *context,
    C_CHAR *buffer, STD_SIZE_T buffer_size)
{
    common_session_fact fact;
    ui_frame display;
    common_session_plan plan;

    if (context == STD_NULL || buffer == STD_NULL || buffer_size == 0u ||
        vm_product_console_ui(context) == LIB_NULL || context->control == STD_NULL)
        return 0;
    buffer[0] = '\0';
    if (common_ui_request_console_line(vm_product_console_ui(context)) != LIB_STATUS_OK) return 0;
    for (;;) {
        if (common_session_take(context->control, &fact, &display, 0xffffffffu) !=
                TYPE_STATUS_OK) return 0;
        if (fact.kind != COMMON_SESSION_FACT_CONSOLE_LINE) {
            if (common_session_reduce_fact(context->control, &fact, &display, &plan) ==
                LIB_STATUS_OK) vm_product_console_apply_plan(context, &plan, TYPE_TRUE);
            if (fact.kind == COMMON_SESSION_FACT_UI_INPUT &&
                vm_product_console_reduce_input(context, &fact.value.input, &plan) == TYPE_STATUS_OK)
                vm_product_console_apply_plan(context, &plan, TYPE_TRUE);
            continue;
        }
        if (common_session_has_cli_provider(context->control)) {
            if (common_session_reduce_fact(context->control, &fact, &display, &plan) ==
                LIB_STATUS_OK) vm_product_console_apply_plan(context, &plan, TYPE_FALSE);
            continue;
        }
        if (fact.kind != COMMON_SESSION_FACT_CONSOLE_LINE) return 0;
        if (STD_STRLEN(fact.value.line) >= buffer_size) return 0;
        STD_MEMCPY(buffer, fact.value.line, STD_STRLEN(fact.value.line) + 1u);
        return 1;
    }
}

/* Prints help commands. */
#define GetHelp do { doHelp(context); return; } while (0)
static C_VOID doHelp(vm_product_console_context *context)
{
    if (STD_STRCMP(argArray[0], "help"))
    {
        numArgs = 2;
        argArray[1] = argArray[0];
    }
    switch (numArgs)
    {
    case 2:
        if (!STD_STRCMP(argArray[1], "help"))
        {
            STD_PRINTF("Show help info\n");
            STD_PRINTF("\nHELP\n");
            STD_PRINTF("  show menu of all commands\n");
            STD_PRINTF("\nHELP <command>\n");
            STD_PRINTF("  show help info for command\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "exit"))
        {
            STD_PRINTF("Stop the machine and quit the console\n");
            STD_PRINTF("\nEXIT\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "info"))
        {
            STD_PRINTF("List virtual machine status\n");
            STD_PRINTF("\nINFO\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "session"))
        {
            STD_PRINTF("NXVM runs one startup-selected virtual machine.\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "speed"))
        {
            STD_PRINTF("Show or select machine speed\n");
            STD_PRINTF("\nSPEED [STANDARD|TURBO]\n");
            STD_PRINTF("  standard: retain L2 HLT host-load backoff\n");
            STD_PRINTF("  turbo:    reserved for Core-deadline fast-forward\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "debug"))
        {
            STD_PRINTF("Launch VM hardware debugger\n");
            STD_PRINTF("\nDEBUG\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "debug32"))
        {
            STD_PRINTF("Launch VM 32-bit hardware debugger\n");
            STD_PRINTF("\nDEBUG\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "record"))
        {
            STD_PRINTF("Record cpu status in each iteration for futher dumping\n");
            STD_PRINTF("\nRECORD start <file> | stop\n");
            STD_PRINTF("  start: open output file for record writes\n");
            STD_PRINTF("  stop:  close output file to finish recording\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "floppy"))
        {
            STD_PRINTF("Change removable floppy media while stopped\n");
            STD_PRINTF("\nFLOPPY INSERT <image> | EJECT\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "start"))
        {
            STD_PRINTF("Start virtual machine\n");
            STD_PRINTF("\nSTART\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "reset"))
        {
            STD_PRINTF("Reset virtual machine\n");
            STD_PRINTF("\nRESET\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "stop"))
        {
            STD_PRINTF("Stop virtual machine\n");
            STD_PRINTF("\nSTOP\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "resume"))
        {
            STD_PRINTF("Resume virtual machine\n");
            STD_PRINTF("\nRESUME\n");
            break;
        }
        break;
    case 1:
    default:
        STD_PRINTF("VM Console Commands\n");
        STD_PRINTF("=====================\n");
        STD_PRINTF("HELP    Show help info\n");
        STD_PRINTF("EXIT    Stop the machine and quit the console\n");
        STD_PRINTF("INFO    List all device info\n");
        STD_PRINTF("SPEED   Show or select machine speed\n");
        STD_PRINTF("\n");
        STD_PRINTF("DEBUG   Launch hardware debugger\n");
        STD_PRINTF("RECORD  Record cpu status for each instruction\n");
        STD_PRINTF("\n");
        STD_PRINTF("FLOPPY  Insert or eject removable floppy media\n");
        STD_PRINTF("\n");
        STD_PRINTF("START   Start virtual machine\n");
        STD_PRINTF("RESET   Reset virtual machine\n");
        STD_PRINTF("STOP    Stop virtual machine\n");
        STD_PRINTF("RESUME  Resume virtual machine\n");
        STD_PRINTF("\n");
        STD_PRINTF("For command usage, type 'HELP <command>'.\n");
        break;
    }
}

/* Quits product. */
static C_VOID doExit(vm_product_console_context *context)
{
    if (numArgs != 1)
    {
        GetHelp;
    }
    if (vm_product_console_stop(context) != TYPE_STATUS_OK) {
        STD_PRINTF("Unable to stop machine.\n");
        return;
    }
    context->session_stopped = TYPE_TRUE;
    flagExit = 1;
}

/* Prints virtual machine status */
static C_VOID doInfo(vm_product_console_context *context)
{
    vm_machine_information information;

    if (numArgs != 1)
    {
        GetHelp;
    }
    if (vm_machine_get_information(vm_product_console_machine(context),
            &information) != TYPE_STATUS_OK) {
        STD_PRINTF("Machine information unavailable.\n");
        return;
    }
    STD_PRINTF("Device Info\n");
    STD_PRINTF("================\n");
    STD_PRINTF("Machine:           IBM PC/AT\n");
    STD_PRINTF("Profile:           %s\n",
        vm_machine_profile_name(information.profile_kind));
    STD_PRINTF("CPU:               Intel %s\n",
        core_machine_cpu_profile_name(information.cpu_profile));
    if (information.memory_bytes < (1u << 20)) {
        STD_PRINTF("RAM Size:          %u KB\n",
            (unsigned int)(information.memory_bytes >> 10));
    } else {
        STD_PRINTF("RAM Size:          %u MB\n",
            (unsigned int)(information.memory_bytes >> 20));
    }
    STD_PRINTF("Floppy Disk Drive: Floppy Disk Drive, %.2f MB, %s\n",
        information.floppy_image_bytes * 1. / ((1 << 10) * 1000),
        information.floppy_media_inserted ? "inserted" : "not inserted");
    if (information.fixed_disk_present) {
        STD_PRINTF("Hard Disk Drive:   %u cylinders, %.2f MB, %s\n",
            (unsigned int)information.fixed_disk_cylinders,
            information.fixed_disk_image_bytes * 1. / (1 << 20),
            information.fixed_disk_media_connected ? "connected" : "disconnected");
    }
    STD_PRINTF("\n");
    STD_PRINTF("Platform Info\n");
    STD_PRINTF("==================\n");
    switch (vm_product_console_ui(context) == LIB_NULL ? COMMON_UI_TARGET_NONE :
        common_ui_get_target(vm_product_console_ui(context))) {
    case COMMON_UI_TARGET_WINDOW:
        STD_PRINTF("Display Type: Window\n");
        break;
    default:
        STD_PRINTF("Display Type: Console\n");
        break;
    }
    STD_PRINTF("\n");
    STD_PRINTF("BIOS Settings\n");
    STD_PRINTF("==================\n");
    STD_PRINTF("BIOS: %s\n", information.external_firmware ?
        "external ROM mapped at F0000h" : "profile ROM mapped");
    STD_PRINTF("\n");
    STD_PRINTF("Device Status\n");
    STD_PRINTF("==================\n");
    STD_PRINTF("Running:   %s\n", information.active ? "Yes" : "No");
    if (information.fault_valid) {
        STD_PRINTF("Fault:     detail=%08X pc=%08X\n", information.fault_detail,
            information.fault_linear_pc);
        if (information.fault_exception_valid) {
            STD_PRINTF("Exception: mask=%08X code=%08X at %04X:%08X\n",
                information.fault_exception_mask, information.fault_exception_code,
                information.fault_exception_cs, information.fault_exception_eip);
        }
    }
}

/* Starts internal debugger */
static C_VOID doDebug(vm_product_console_context *context)
{
    if (numArgs != 1)
    {
        GetHelp;
    }
    if (vm_product_console_machine(context) == STD_NULL || context->debug == STD_NULL ||
        vm_machine_pause_for_debug(vm_product_console_machine(context), 2000u) !=
            TYPE_STATUS_OK ||
        common_debug_open(context->debug, vm_machine_common_machine(
            vm_product_console_machine(context))) != LIB_STATUS_OK ||
        common_session_set_cli_provider(context->control,
            vm_product_console_debug_provider, context) != LIB_STATUS_OK ||
        common_session_set_cli_machine_observer(context->control,
            vm_product_console_debug_machine, context) != LIB_STATUS_OK)
        STD_PRINTF("Unable to enter debugger.\n");
}

/* Executes cpu instruction recorder */
static C_VOID doRecord(vm_product_console_context *context)
{
    if (numArgs < 2)
    {
        GetHelp;
    }
    if (vm_product_console_is_running(context))
    {
        STD_PRINTF("Cannot change record status or dump record now.\n");
        return;
    }
    if (!STD_STRCMP(argArray[1], "start"))
    {
        if (numArgs != 3)
        {
            GetHelp;
        }
        if (vm_product_recorder_start(context->recorder, argArray[2]) ==
            TYPE_STATUS_OK) STD_PRINTF("Record started.\n");
        else STD_PRINTF("ERROR:\trecorder cannot open output file.\n");
    }
    else if (!STD_STRCMP(argArray[1], "stop"))
    {
        if (vm_product_recorder_stop(context->recorder) == TYPE_STATUS_OK)
            STD_PRINTF("Record finished.\n");
        else STD_PRINTF("ERROR:\trecorder not turned on.\n");
    }
    else
    {
        GetHelp;
    }
}

static const C_CHAR *vm_product_console_speed_name(vm_machine_speed speed)
{
    return speed == VM_MACHINE_SPEED_TURBO ? "turbo" : "standard";
}

static C_VOID doSpeed(vm_product_console_context *context)
{
    vm_machine_speed speed;
    type_status status;

    if (numArgs == 1u) {
        if (vm_product_console_machine(context) != STD_NULL &&
            vm_machine_get_speed(vm_product_console_machine(context), &speed) == TYPE_STATUS_OK) {
            STD_PRINTF("Speed: %s\n", vm_product_console_speed_name(speed));
        }
        return;
    }
    if (numArgs != 2u) { GetHelp; }
    if (!STD_STRCMP(argArray[1], "standard")) speed = VM_MACHINE_SPEED_STANDARD;
    else if (!STD_STRCMP(argArray[1], "turbo")) speed = VM_MACHINE_SPEED_TURBO;
    else { GetHelp; }
    status = vm_product_console_machine(context) == STD_NULL ? TYPE_STATUS_INVALID_STATE :
        vm_machine_set_speed(vm_product_console_machine(context), speed);
    if (status == TYPE_STATUS_OK) {
        STD_PRINTF("Speed: %s\n", vm_product_console_speed_name(speed));
    } else if (status == TYPE_STATUS_INVALID_STATE) {
        STD_PRINTF("Cannot change speed while session is running.\n");
    } else {
        STD_PRINTF("Cannot change speed.\n");
    }
}

/* Set hardware connections */
static C_VOID doFloppy(vm_product_console_context *context)
{
    if (numArgs == 3u && !STD_STRCMP(argArray[1], "insert")) {
        if (vm_product_console_is_running(context)) {
            STD_PRINTF("Cannot change floppy media now.\n");
        } else if (vm_machine_insert_fdd(vm_product_console_machine(context), argArray[2])) {
            STD_PRINTF("Cannot read floppy disk from '%s'.\n", argArray[2]);
        } else {
            STD_PRINTF("Floppy disk inserted.\n");
        }
        return;
    }
    if (numArgs == 2u && !STD_STRCMP(argArray[1], "eject")) {
        if (vm_product_console_is_running(context)) {
            STD_PRINTF("Cannot change floppy media now.\n");
        } else if (vm_machine_remove_fdd(vm_product_console_machine(context), STD_NULL)) {
            STD_PRINTF("Cannot eject floppy disk.\n");
        } else {
            STD_PRINTF("Floppy disk ejected.\n");
        }
        return;
    }
    STD_PRINTF("Usage: FLOPPY INSERT <image> | EJECT\n");
}

static C_INT vm_product_console_choose_profile(const vm_product_console_context *context,
    vm_session_request *out_entry)
{
    C_CHAR selection[32];
    STD_SIZE_T index;
    STD_SIZE_T count;
    C_INT choice;

    if (context == STD_NULL || out_entry == STD_NULL ||
        (count = vm_product_session_catalog_count(context->catalog)) == 0u) {
        STD_PRINTF("No session configuration files found.\n");
        return 0;
    }
    STD_PRINTF("Available session profiles:\n");
    for (index = 0u; index < count; ++index) {
        vm_session_request entry;

        if (vm_product_session_catalog_get_request(context->catalog, index, &entry) !=
            TYPE_STATUS_OK) return 0;
        STD_PRINTF("  %u  %s\n", (unsigned int)(index + 1u), entry.file_name);
    }
    STD_PRINTF("Select profile [1-%u, Enter to cancel]: ",
        (unsigned int)count);
    if (!vm_product_console_read_line((vm_product_console_context *)context,
            selection, sizeof(selection))) return 0;
    if (selection[0] == '\n' || selection[0] == '\r' || selection[0] == '\0') return 0;
    choice = STD_ATOI(selection);
    if (choice > 0 && (STD_SIZE_T)choice <= count) {
        return vm_product_session_catalog_get_request(context->catalog,
            (STD_SIZE_T)(choice - 1), out_entry) == TYPE_STATUS_OK;
    }
    STD_PRINTF("Unknown profile selection.\n");
    return 0;
}

static C_VOID vm_product_console_open_profile(vm_product_console_context *context)
{
    vm_session_request selected_entry;
    common_session_target target;

    if (context == STD_NULL || !vm_product_console_choose_profile(context,
            &selected_entry)) return;
    if (vm_app_compose_machine(currentSession, &selected_entry) != TYPE_STATUS_OK) {
        STD_PRINTF("Unable to create session from '%s'.\n", selected_entry.file_name);
        return;
    }
    target = !STD_STRCMP(selected_entry.display, "window") ?
        COMMON_SESSION_TARGET_WINDOW : COMMON_SESSION_TARGET_CONSOLE;
    (C_VOID)common_session_set_target(context->control, target);
    if (context->debug != STD_NULL)
        vm_machine_bind_debug_observer(vm_product_console_machine(context),
            vm_product_console_debug_observe, context);
}

/* Executes commands */
static C_VOID execute(vm_product_console_context *context)
{
    if (!argArray[0] || !STD_STRLEN(argArray[0]))
    {
        return;
    }
    else if (!STD_STRCMP(argArray[0], "help") || !STD_STRCMP(argArray[0], "exit"))
    {
        if (!STD_STRCMP(argArray[0], "help")) doHelp(context);
        else doExit(context);
    }
    else if (!STD_STRCMP(argArray[0], "info"))
    {
        doInfo(context);
    }
    else if (!STD_STRCMP(argArray[0], "debug"))
    {
        doDebug(context);
    }
    else if (!STD_STRCMP(argArray[0], "record"))
    {
        doRecord(context);
    }
    else if (!STD_STRCMP(argArray[0], "speed"))
    {
        doSpeed(context);
    }
    else if (!STD_STRCMP(argArray[0], "floppy"))
    {
        doFloppy(context);
    }
    else if (!STD_STRCMP(argArray[0], "start"))
    {
        common_session_plan plan;
        type_status status = vm_product_console_start(context, &plan);
        if (status == TYPE_STATUS_OK)
            vm_product_console_apply_plan(context, &plan, TYPE_FALSE);
        if (status != TYPE_STATUS_OK) {
            STD_PRINTF("START failed: %d.\n", (C_INT)status);
        }
    }
    else if (!STD_STRCMP(argArray[0], "reset"))
    {
        if (vm_product_console_machine(context) != STD_NULL)
            (C_VOID)vm_machine_reset(vm_product_console_machine(context));
    }
    else if (!STD_STRCMP(argArray[0], "stop"))
    {
        (C_VOID)vm_product_console_stop(context);
    }
    else if (!STD_STRCMP(argArray[0], "resume"))
    {
        common_session_plan plan;
        type_status status = vm_product_console_resume(context, &plan);
        if (status == TYPE_STATUS_OK)
            vm_product_console_apply_plan(context, &plan, TYPE_FALSE);
        if (status != TYPE_STATUS_OK) {
            STD_PRINTF("RESUME failed: %d.\n", (C_INT)status);
        }
    }
    else
    {
        STD_PRINTF("Illegal command '%s'.\n", argArray[0]);
    }
    STD_PRINTF("\n");
}

/* Initializes console */
static C_INT vm_product_console_initialize(vm_product_console_context *context,
    const C_CHAR *profile_directory)
{
    common_ui_options ui_options = {0};

    argArray = (C_CHAR **)STD_MALLOC(CONSOLE_MAXNARG * sizeof(C_CHAR *));
    if (argArray == STD_NULL) return TYPE_FALSE;
    flagExit = 0;
    context->session_stopped = TYPE_FALSE;
    context->control = vm_app_session(currentSession);
    if (context->control == STD_NULL) {
        context->control = STD_NULL;
        return TYPE_FALSE;
    }
    ui_options.input_context = context->control;
    ui_options.input_sink = vm_product_console_input_sink;
    ui_options.console_line_context = context->control;
    ui_options.console_line_sink = vm_product_console_line_sink;
    ui_options.failure_context = context;
    ui_options.failure_sink = vm_product_console_ui_failure;
    ui_hotkey_registry_initialize(&ui_options.hotkeys);
    (void)ui_hotkey_registry_register(&ui_options.hotkeys, 'P',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "pause");
    (void)ui_hotkey_registry_register(&ui_options.hotkeys, 'D',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "cad");
    (void)ui_hotkey_registry_register(&ui_options.hotkeys, 'F',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "alt-enter");
    (void)ui_hotkey_registry_register(&ui_options.hotkeys, 'M',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "release-mouse");
    ui_options.initial_window_title = "NXVM (Running)";
    if (vm_app_compose_ui(context->session, &ui_options) != TYPE_STATUS_OK) {
        context->control = STD_NULL;
        return TYPE_FALSE;
    }
    if (vm_product_recorder_create(&context->recorder) != TYPE_STATUS_OK) {
        context->control = STD_NULL;
        return TYPE_FALSE;
    }
    if (common_debug_create(&context->debug) != LIB_STATUS_OK ||
        common_session_set_lifecycle_sink(context->control,
            vm_product_console_lifecycle, context) != LIB_STATUS_OK) {
        common_debug_destroy(context->debug);
        context->debug = STD_NULL;
        vm_product_recorder_destroy(context->recorder);
        context->recorder = STD_NULL;
        context->control = STD_NULL;
        STD_FREE(argArray);
        argArray = STD_NULL;
        return TYPE_FALSE;
    }
    if (vm_product_session_catalog_create(profile_directory, &context->catalog) ==
            TYPE_STATUS_OK) return TYPE_TRUE;
    vm_product_session_catalog_destroy(context->catalog);
    context->catalog = STD_NULL;
    context->control = STD_NULL;
    common_debug_destroy(context->debug);
    context->debug = STD_NULL;
    vm_product_recorder_destroy(context->recorder);
    context->recorder = STD_NULL;
    STD_FREE(argArray);
    argArray = STD_NULL;
    return TYPE_FALSE;
}

/* Finalizes console */
static C_VOID vm_product_console_finalize(vm_product_console_context *context)
{
    if (argArray)
    {
        STD_FREE((C_VOID *)argArray);
    }
    argArray = STD_NULL;
    if (!context->session_stopped && vm_product_console_stop(context) == TYPE_STATUS_OK) {
        context->session_stopped = TYPE_TRUE;
    }
    vm_product_console_drain_lifecycle(context, TYPE_FALSE);
    vm_product_session_catalog_destroy(context->catalog);
    context->catalog = STD_NULL;
    if (vm_product_console_machine(context) != STD_NULL)
        vm_machine_bind_debug_observer(vm_product_console_machine(context), STD_NULL,
            STD_NULL);
    common_debug_destroy(context->debug);
    vm_product_recorder_destroy(context->recorder);
    context->recorder = STD_NULL;
    context->debug = STD_NULL;
    common_session_close(context->control);
    context->control = STD_NULL;
}

type_status vm_product_console_context_create(
    vm_product_console_context **out_context)
{
    vm_product_console_context *context;

    if (out_context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_context = STD_NULL;
    context = (vm_product_console_context *)STD_CALLOC(1u, sizeof(*context));
    if (context == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    *out_context = context;
    return TYPE_STATUS_OK;
}

C_VOID vm_product_console_context_destroy(vm_product_console_context *context)
{
    if (context == STD_NULL) return;
    if (vm_product_console_machine(context) != STD_NULL)
        vm_machine_bind_debug_observer(vm_product_console_machine(context), STD_NULL,
            STD_NULL);
    common_debug_destroy(context->debug);
    vm_product_session_catalog_destroy(context->catalog);
    vm_product_recorder_destroy(context->recorder);
    STD_FREE(context);
}

C_VOID vm_product_console_main(vm_product_console_context *context,
                               vm_app *machine_session,
                               const C_CHAR *profile_directory)
{
    if (context == STD_NULL || machine_session == STD_NULL ||
        profile_directory == STD_NULL)
        return;
    context->session = machine_session;
    if (!vm_product_console_initialize(context, profile_directory)) return;
    STD_PRINTF("\nType HELP for help.\n\n");
    vm_product_console_open_profile(context);
    while (!flagExit)
    {
        STD_PRINTF("Console> ");
        if (!vm_product_console_read_line(context, strCmdBuff,
                sizeof(strCmdBuff))) break;
        parse(context);
        execute(context);
        vm_product_console_drain_lifecycle(context, TYPE_FALSE);
    }
    vm_product_console_finalize(context);
}
