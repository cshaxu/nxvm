#include "lib/types/test.h"
#include "x86-ibmpc/product/command_interface.h"

static lib_status result_status;
static lib_bool result_ready;

static lib_status result_value(x86_debug_result *out)
{
    *out = (x86_debug_result){.text = "Debug result", .prompt = "-",
        .prompt_ready = result_ready, .keep_active = LIB_TRUE};
    return result_status;
}

static lib_status submit(x86_debug *debug, const char *line, x86_debug_result *out)
{ (void)debug; (void)line; return result_value(out); }
static lib_status observe(x86_debug *debug, x86_debug_machine_state state,
    lib_status status, x86_debug_result *out)
{ (void)debug; (void)state; (void)status; return result_value(out); }

#define x86_debug_submit_line submit
#define x86_debug_observe_machine observe
#include "x86-ibmpc/product/command.c"

int main(void)
{
    x86_ibmpc_product_command_context command = {.debug_active = LIB_TRUE};
    emulator_session_command_result out;
    const lib_status failures[] = {LIB_STATUS_NO_MEMORY, LIB_STATUS_IO_ERROR};
    command.monitor.context = &command;
    result_ready = LIB_TRUE;
    for (lib_size index = 0u; index < sizeof(failures) / sizeof(failures[0]); ++index) {
        result_status = failures[index];
        for (lib_u32 runtime = 0u; runtime < 2u; ++runtime) {
            out = (emulator_session_command_result){.request = EMULATOR_SESSION_REQUEST_RESUME};
            if (runtime)
                x86_ibmpc_product_command_provider_note_runtime(&command.monitor,
                    EMULATOR_SESSION_MACHINE_RUNNING, EMULATOR_SESSION_MACHINE_PAUSED, &out);
            else
                x86_ibmpc_product_command_provider_submit_line(&command.monitor,
                    EMULATOR_SESSION_MACHINE_PAUSED, "r", &out);
            lib_test_assert(out.text[0] == '\0' && out.arm_prompt &&
                out.request == EMULATOR_SESSION_REQUEST_NONE);
            lib_test_assert(lib_text_compare(out.detail, "Debug result") == 0 &&
                lib_text_compare(out.prompt, "-") == 0);
        }
    }
    result_status = LIB_STATUS_OK;
    result_ready = LIB_FALSE;
    x86_ibmpc_product_command_provider_note_runtime(&command.monitor,
        EMULATOR_SESSION_MACHINE_RUNNING, EMULATOR_SESSION_MACHINE_PAUSED, &out);
    lib_test_assert(lib_text_compare(out.text, "Machine paused.\r\n") == 0);
    return 0;
}
