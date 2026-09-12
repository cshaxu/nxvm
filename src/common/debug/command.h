/* Copyright 2012-2014 Neko. */

#ifndef COMMON_DEBUG_COMMAND_H
#define COMMON_DEBUG_COMMAND_H

#include "common/debug/debug_interface.h"

typedef struct common_debug_command common_debug_command;

lib_status common_debug_command_create(common_debug_command **out_command);
void common_debug_command_destroy(common_debug_command *command);
lib_status common_debug_command_open(common_debug_command *command,
    common_machine *machine);
void common_debug_command_close(common_debug_command *command);
lib_status common_debug_command_submit_line(common_debug_command *command,
    const char *line, common_debug_result *out_result);
void common_debug_command_observe_instruction(common_debug_command *command,
    const common_debug_instruction_observation *observation);
lib_status common_debug_command_observe_machine(common_debug_command *command,
    common_debug_machine_state state, lib_status status,
    common_debug_result *out_result);

#endif
