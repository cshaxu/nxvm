#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "emulator/product/monitor_interface.h"
#include "product/surface/command_interface.h"
#include "product/surface/keyboard_interface.h"

static void product_surface_command_set_prompt(
    const product_surface_command_context *command,
    emulator_session_command_result *out);
static void product_surface_command_message(emulator_session_command_result *out,
    const char *text);

static lib_bool product_surface_command_snapshot_not_supported(void *opaque,
    emulator_machine *machine, emulator_product_monitor_command command,
    emulator_session_machine_state state, const char *arguments,
    emulator_session_command_result *out)
{
    (void)opaque;
    (void)machine;
    (void)command;
    (void)state;
    (void)arguments;
    product_surface_command_message(out, "Feature not implemented.");
    return LIB_TRUE;
}

static void product_surface_command_message(emulator_session_command_result *out,
    const char *text)
{
    (void)lib_c_snprintf(out->text, sizeof(out->text), "%s\r\n", text);
    out->arm_prompt = LIB_TRUE;
}

static lib_bool product_surface_command_submit_extension(void *opaque,
    emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out)
{
    product_surface_command_context *command = opaque;

    return command != LIB_NULL && command->extensions.submit != LIB_NULL &&
        command->extensions.submit(command->extensions.context, command->machine,
            state, line, out);
}

static lib_bool product_surface_command_submit_fixed(void *opaque,
    emulator_product_monitor_command fixed_command,
    emulator_session_machine_state state, const char *arguments,
    emulator_session_command_result *out)
{
    product_surface_command_context *command = opaque;

    (void)arguments;
    if (command == LIB_NULL || out == LIB_NULL) return LIB_FALSE;
    if (fixed_command == EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE ||
        fixed_command == EMULATOR_PRODUCT_MONITOR_COMMAND_LOAD)
        return command->extensions.submit_snapshot(command->extensions.context,
            command->machine, fixed_command, state, arguments, out);
    if (fixed_command == EMULATOR_PRODUCT_MONITOR_COMMAND_DEBUG) {
        if (product_debug_open(command->debug, command->machine) != LIB_STATUS_OK) {
            product_surface_command_message(out, "Cannot open debugger.");
            return LIB_TRUE;
        }
        command->debug_active = LIB_TRUE;
        (void)lib_c_snprintf(command->debug_prompt, sizeof(command->debug_prompt), "-");
        out->arm_prompt = LIB_TRUE;
        product_surface_command_set_prompt(command, out);
        return LIB_TRUE;
    }
    (void)state;
    return LIB_FALSE;
}

_Static_assert(EMULATOR_SESSION_PROMPT_CAPACITY >= PRODUCT_DEBUG_PROMPT_CAPACITY,
               "Session prompt must hold debugger continuation prompts");

static void product_surface_command_set_prompt(const product_surface_command_context *command,
                                   emulator_session_command_result *out)
{
    if (command == LIB_NULL || out == LIB_NULL)
        return;
    (void)lib_c_snprintf(out->prompt, sizeof(out->prompt), "%s",
        command->debug_active ? command->debug_prompt :
            EMULATOR_SESSION_MONITOR_PROMPT);
}

static void product_surface_command_copy_debug(product_surface_command_context *command,
                                   emulator_session_machine_state state, const product_debug_result *result,
                                   emulator_session_command_result *out)
{
    out->detail = result->text;
    (void)lib_c_snprintf(command->debug_prompt, sizeof(command->debug_prompt), "%s", result->prompt);
    if (!result->keep_active)
    {
        product_debug_close(command->debug);
        command->debug_active = LIB_FALSE;
    }
    if (result->lifecycle_request == PRODUCT_DEBUG_LIFECYCLE_RESUME &&
        state == EMULATOR_SESSION_MACHINE_PAUSED)
        out->request = EMULATOR_SESSION_REQUEST_RESUME;
    else if (result->lifecycle_request != PRODUCT_DEBUG_LIFECYCLE_NONE)
        (void)lib_c_snprintf(out->text, sizeof(out->text), "Debug lifecycle request is not applicable.");
}

void product_surface_command_provider_submit_line(void *opaque,
                                      emulator_session_machine_state state, const char *line,
                                      emulator_session_command_result *out)
{
    emulator_product_monitor_provider *monitor = opaque;
    product_surface_command_context *command = monitor != LIB_NULL ? monitor->context : LIB_NULL;
    if (command == LIB_NULL || out == LIB_NULL) return;
    if (command->debug_active)
    {
        product_debug_result result = {0};
        lib_status status = product_debug_submit_line(command->debug, line, &result);
        *out = (emulator_session_command_result){0};
        if (status != LIB_STATUS_OK)
        {
            (void)lib_c_snprintf(out->text, sizeof(out->text), "Debug command failed.");
            emulator_machine_debug_cancel(command->machine);
            (void)lib_c_snprintf(command->debug_prompt, sizeof(command->debug_prompt), "-");
        }
        else
        {
            product_surface_command_copy_debug(command, state, &result, out);
        }
        out->arm_prompt = out->request == EMULATOR_SESSION_REQUEST_NONE;
        product_surface_command_set_prompt(command, out);
        return;
    }
    emulator_product_monitor_provider_submit_line(opaque, state, line, out);
}

void product_surface_command_provider_note_runtime(void *opaque,
                                       emulator_session_machine_state prior, emulator_session_machine_state completed,
                                       emulator_session_command_result *out)
{
    emulator_product_monitor_provider *monitor = opaque;
    product_surface_command_context *command = monitor != LIB_NULL ? monitor->context : LIB_NULL;
    if (command == LIB_NULL || out == LIB_NULL) return;
    emulator_product_monitor_provider_note_runtime(monitor, prior, completed, out);
    if (command->debug_active)
    {
        product_debug_machine_state state = completed == EMULATOR_SESSION_MACHINE_PAUSED ? PRODUCT_DEBUG_MACHINE_PAUSED : completed == EMULATOR_SESSION_MACHINE_RUNNING ? PRODUCT_DEBUG_MACHINE_RUNNING
                                                                                                                                                                  : PRODUCT_DEBUG_MACHINE_STOPPED;
        product_debug_result result = {0};
        if (product_debug_observe_machine(command->debug, state, LIB_STATUS_OK, &result) != LIB_STATUS_OK)
        {
            (void)lib_c_snprintf(out->text, sizeof(out->text), "Debug command failed.");
            emulator_machine_debug_cancel(command->machine);
            (void)lib_c_snprintf(command->debug_prompt, sizeof(command->debug_prompt), "-");
        }
        else if (result.prompt_ready)
        {
            product_surface_command_copy_debug(command, completed, &result, out);
            out->arm_prompt = out->request == EMULATOR_SESSION_REQUEST_NONE;
            product_surface_command_set_prompt(command, out);
        }
    }
}

lib_status product_surface_command_initialize(product_surface_command_context *command,
                                  emulator_machine *machine, emulator_session_display display,
                                  const product_surface_command_extensions *extensions)
{
    if (command == LIB_NULL || machine == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(command, 0, sizeof(*command));
    command->machine = machine;
    if (extensions != LIB_NULL) command->extensions = *extensions;
    if (command->extensions.submit_snapshot == LIB_NULL)
        command->extensions.submit_snapshot = product_surface_command_snapshot_not_supported;
    command->monitor = (emulator_product_monitor_provider){
        .context = command,
        .extension_commands = command->extensions.help,
        .hotkeys = product_surface_keyboard_hotkey_help(),
        .submit_fixed = product_surface_command_submit_fixed,
        .submit_extension = product_surface_command_submit_extension};
    (void)display;
    return product_debug_create(&command->debug);
}

void product_surface_command_dispose(product_surface_command_context *command)
{
    if (command != LIB_NULL)
        product_debug_destroy(command->debug);
}
