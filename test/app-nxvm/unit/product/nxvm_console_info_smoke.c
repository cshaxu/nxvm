#define vm_app_machine console_info_machine
#define vm_machine_get_information console_info_get_information
/* Preserve the included owner's existing unsigned-text vocabulary. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-sign"
#include "app-nxvm/product/command.c"
#pragma GCC diagnostic pop
#undef vm_machine_get_information
#undef vm_app_machine

struct vm_machine { lib_bool active; };
static struct vm_machine fixture;

vm_machine *console_info_machine(const vm_app *app)
{
    (void)app;
    return &fixture;
}

lib_status console_info_get_information(const vm_machine *machine,
    vm_machine_information *information)
{
    *information = (vm_machine_information){0};
    information->active = machine->active;
    return LIB_STATUS_OK;
}

lib_i32 main(void)
{
    static const common_session_machine_state states[] = {
        COMMON_SESSION_MACHINE_INIT, COMMON_SESSION_MACHINE_STOPPED,
        COMMON_SESSION_MACHINE_RUNNING, COMMON_SESSION_MACHINE_PAUSED,
        COMMON_SESSION_MACHINE_ERROR, COMMON_SESSION_MACHINE_RESET_COMPLETED
    };
    vm_app_console_context context = {0};
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
            if (strstr(result.text, expected) == LIB_NULL) return 1;
        }
    }
    puts("NXVM console INFO lifecycle: OK");
    return 0;
}
