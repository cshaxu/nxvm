#include "common/debug/debug_interface.h"

#include "common/debug/command.h"

struct common_debug {
    common_debug_command *command;
};

lib_status common_debug_create(common_debug **out_debug)
{
    common_debug *debug;

    if (out_debug == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_debug = LIB_NULL;
    debug = lib_allocate_zero(1u, sizeof(*debug));
    if (debug == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (common_debug_command_create(&debug->command) != LIB_STATUS_OK) {
        lib_release(debug);
        return LIB_STATUS_NO_MEMORY;
    }
    *out_debug = debug;
    return LIB_STATUS_OK;
}

void common_debug_destroy(common_debug *debug)
{
    if (debug == LIB_NULL) return;
    common_debug_command_destroy(debug->command);
    lib_release(debug);
}

lib_status common_debug_open(common_debug *debug, common_machine *machine)
{
    if (debug == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return common_debug_command_open(debug->command, machine);
}

void common_debug_close(common_debug *debug)
{
    if (debug != LIB_NULL) common_debug_command_close(debug->command);
}

lib_status common_debug_submit_line(common_debug *debug, const char *line,
    common_debug_result *out_result)
{
    if (debug == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return common_debug_command_submit_line(debug->command, line, out_result);
}

void common_debug_observe_instruction(common_debug *debug,
    const common_debug_instruction_observation *observation)
{
    if (debug != LIB_NULL)
        common_debug_command_observe_instruction(debug->command, observation);
}

lib_status common_debug_observe_machine(common_debug *debug,
    common_debug_machine_state state, lib_status status,
    common_debug_result *out_result)
{
    if (debug == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return common_debug_command_observe_machine(debug->command, state, status,
        out_result);
}
