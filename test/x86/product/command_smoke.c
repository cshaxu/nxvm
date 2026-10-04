#include "x86/product/command.c"
#include "x86/product/composition.c"

struct command_fixture { lib_bool active; };
static struct command_fixture fixture;
static vm_app_speed speed;

static lib_status command_get_speed(const void *machine, vm_app_speed *out_speed)
{
    (void)machine;
    *out_speed = speed;
    return LIB_STATUS_OK;
}

static lib_status command_set_speed(void *machine, vm_app_speed value)
{
    (void)machine;
    speed = value;
    return LIB_STATUS_OK;
}

static lib_status console_info_get_information(const void *machine,
    vm_app_information *information)
{
    (void)machine;
    *information = (vm_app_information){ .machine_name = "fixture", .cpu_name = "80386" };
    return LIB_STATUS_OK;
}

lib_i32 main(void)
{
    static const common_session_machine_state states[] = {
        COMMON_SESSION_MACHINE_INIT, COMMON_SESSION_MACHINE_STOPPED,
        COMMON_SESSION_MACHINE_RUNNING, COMMON_SESSION_MACHINE_PAUSED,
        COMMON_SESSION_MACHINE_ERROR, COMMON_SESSION_MACHINE_RESET_COMPLETED
    };
    vm_app app = { .machine = &fixture,
        .factory = { .information = console_info_get_information,
            .get_speed = command_get_speed, .set_speed = command_set_speed } };
    vm_app_console_context context = { .session = &app };
    common_session_command_result result;
    lib_size index;
    lib_u32 active;

    /* Worker lifetime must not decide the monitor's lifecycle report. */
    for (active = 0u; active < 2u; ++active) {
        fixture.active = active != 0u ? LIB_TRUE : LIB_FALSE;
        for (index = 0u; index < sizeof(states) / sizeof(states[0]); ++index) {
            const char *expected = states[index] == COMMON_SESSION_MACHINE_RUNNING ?
                "Running: Yes\n" : "Running: No\n";
            vm_app_console_submit_line(&context, states[index], "info", &result);
            if (lib_text_find_substring((const char *)result.text, expected) == LIB_NULL) return 1;
        }
    }
    {
        static const struct {
            const char *line;
            common_session_request request;
        } commands[] = {
            { "start", COMMON_SESSION_REQUEST_START },
            { "resume", COMMON_SESSION_REQUEST_RESUME },
            { "stop", COMMON_SESSION_REQUEST_STOP },
            { "reset", COMMON_SESSION_REQUEST_RESET }
        };
        for (index = 0u; index < sizeof(commands) / sizeof(commands[0]); ++index) {
            vm_app_console_submit_line(&context, COMMON_SESSION_MACHINE_PAUSED,
                commands[index].line, &result);
            if (result.request != commands[index].request) return 1;
        }
    }
    vm_app_console_submit_line(&context, COMMON_SESSION_MACHINE_PAUSED,
        "speed turbo", &result);
    if (speed != VM_APP_SPEED_TURBO || lib_text_compare((const char *)result.text,
            "Speed: turbo\n") != 0) return 1;
    vm_app_console_submit_line(&context, COMMON_SESSION_MACHINE_PAUSED,
        "speed standard", &result);
    if (speed != VM_APP_SPEED_STANDARD) return 1;
    vm_app_console_submit_line(&context, COMMON_SESSION_MACHINE_RUNNING,
        "debug", &result);
    if (!context.debug_requested || result.request != COMMON_SESSION_REQUEST_PAUSE)
        return 1;
    vm_app_console_note_runtime(&context, COMMON_SESSION_MACHINE_RUNNING,
        COMMON_SESSION_MACHINE_STOPPED, &result);
    if (lib_text_compare((const char *)result.text, "Machine stopped.\n") != 0)
        return 1;
    vm_app_console_note_monitor_current(&context, LIB_TRUE, &result);
    if (!result.arm_prompt || lib_text_compare((const char *)result.prompt,
            "Console> ") != 0) return 1;
    {
        x86_debug_result debug = { .lifecycle_request = X86_DEBUG_LIFECYCLE_STEP };
        vm_app_console_clear_result(&result);
        vm_app_console_debug_result(&result, &debug);
        if (result.request != COMMON_SESSION_REQUEST_RESUME) return 1;
        debug.lifecycle_request = X86_DEBUG_LIFECYCLE_STOP;
        vm_app_console_debug_result(&result, &debug);
        if (result.request != COMMON_SESSION_REQUEST_STOP) return 1;
    }
    lib_c_printf("PC console INFO lifecycle: OK\n");
    return 0;
}
