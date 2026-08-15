#include "type.h"

#include "core/product/session/command_interface.h"

static C_VOID core_product_session_command_write(
    const core_product_session_output_provider *output, const C_CHAR *line)
{
    if (output != STD_NULL && output->write_line != STD_NULL) {
        output->write_line(output->context, line);
    }
}

static const C_CHAR *core_product_session_command_state(
    core_product_session_state state)
{
    switch (state) {
    case CORE_PRODUCT_SESSION_STATE_READY: return "ready";
    case CORE_PRODUCT_SESSION_STATE_RUNNING: return "running";
    case CORE_PRODUCT_SESSION_STATE_PAUSED: return "paused";
    case CORE_PRODUCT_SESSION_STATE_STOPPED: return "stopped";
    case CORE_PRODUCT_SESSION_STATE_FAULTED: return "faulted";
    default: return "unknown";
    }
}

static const C_CHAR *core_product_session_command_display(
    core_product_session_display display)
{
    switch (display) {
    case CORE_PRODUCT_SESSION_DISPLAY_CONSOLE: return "console";
    case CORE_PRODUCT_SESSION_DISPLAY_WINDOW: return "window";
    default: return "unknown";
    }
}

static C_INT core_product_session_command_list(core_product_session_manager *manager,
    const core_product_session_output_provider *output)
{
    core_product_session_snapshot *snapshots;
    STD_SIZE_T count;
    STD_SIZE_T index;
    C_CHAR line[176];

    if (core_product_session_manager_get_count(manager, &count) != TYPE_STATUS_OK) {
        core_product_session_command_write(output, "Session manager is unavailable.");
        return 0;
    }
    snapshots = (core_product_session_snapshot *)STD_CALLOC(count, sizeof(*snapshots));
    if (snapshots == STD_NULL || core_product_session_manager_list(manager, snapshots,
            count, &count) != TYPE_STATUS_OK) {
        STD_FREE(snapshots);
        core_product_session_command_write(output, "Unable to list sessions.");
        return 0;
    }
    for (index = 0u; index < count; ++index) {
        STD_SNPRINTF(line, sizeof(line), "%c %u %s %s%s%s", snapshots[index].selected ?
            '*' : ' ', (unsigned int)snapshots[index].id,
            core_product_session_command_state(snapshots[index].state),
            core_product_session_command_display(snapshots[index].display),
            snapshots[index].details[0] ? " " : "", snapshots[index].details);
        core_product_session_command_write(output, line);
    }
    STD_FREE(snapshots);
    return 1;
}

C_INT core_product_session_command_execute(core_product_session_manager *manager,
    C_INT argument_count, C_CHAR **arguments,
    const core_product_session_output_provider *output)
{
    core_product_session_id id;
    C_CHAR line[96];

    if (manager == STD_NULL || arguments == STD_NULL || argument_count < 2) {
        core_product_session_command_write(output,
            "Usage: SESSION LIST | OPEN | SELECT <id> | CLOSE [id]");
        return 0;
    }
    if (!STD_STRCMP(arguments[1], "list") && argument_count == 2) {
        return core_product_session_command_list(manager, output);
    }
    if (!STD_STRCMP(arguments[1], "open")) {
        const core_product_session_open_options options = {
            argument_count - 2, arguments + 2
        };
        type_status status = core_product_session_manager_open_with_options(
            manager, &options, &id);
        if (status != TYPE_STATUS_OK) {
            core_product_session_command_write(output,
                status == TYPE_STATUS_INVALID_ARGUMENT ?
                "Invalid session options." :
                status == TYPE_STATUS_INVALID_STATE ?
                "Requested session configuration is unavailable." :
                "Unable to open session.");
            return 0;
        }
        if (core_product_session_manager_select(manager, id) != TYPE_STATUS_OK) {
            core_product_session_command_write(output, "Unable to select opened session.");
            return 0;
        }
        STD_SNPRINTF(line, sizeof(line), "Opened and selected session %u.",
            (unsigned int)id);
        core_product_session_command_write(output, line);
        return 1;
    }
    if (!STD_STRCMP(arguments[1], "select") && argument_count == 3) {
        C_INT parsed = STD_ATOI(arguments[2]);
        if (parsed < 0 || core_product_session_manager_select(manager,
                (core_product_session_id)parsed) != TYPE_STATUS_OK) {
            core_product_session_command_write(output, "Unknown session.");
            return 0;
        }
        STD_SNPRINTF(line, sizeof(line), "Selected session %u.",
            (unsigned int)parsed);
        core_product_session_command_write(output, line);
        return 1;
    }
    if (!STD_STRCMP(arguments[1], "close") &&
        (argument_count == 2 || argument_count == 3)) {
        STD_SIZE_T count;
        if (argument_count == 2) {
            if (core_product_session_manager_get_selected_id(manager, &id) !=
                    TYPE_STATUS_OK) {
                core_product_session_command_write(output, "Session manager is unavailable.");
                return 0;
            }
        } else {
            C_INT parsed = STD_ATOI(arguments[2]);
            if (parsed < 0) {
                core_product_session_command_write(output, "Unknown session.");
                return 0;
            }
            id = (core_product_session_id)parsed;
        }
        if (core_product_session_manager_get_count(manager, &count) !=
            TYPE_STATUS_OK) {
            core_product_session_command_write(output, "Session manager is unavailable.");
            return 0;
        }
        if (count == 1u) {
            core_product_session_command_write(output,
                "Cannot close the final session.");
            return 0;
        }
        if (core_product_session_manager_close(manager, id) != TYPE_STATUS_OK) {
            core_product_session_command_write(output, "Unknown session.");
            return 0;
        }
        STD_SNPRINTF(line, sizeof(line), "Closed session %u.", (unsigned int)id);
        core_product_session_command_write(output, line);
        return 1;
    }
    core_product_session_command_write(output,
        "Usage: SESSION LIST | OPEN | SELECT <id> | CLOSE [id]");
    return 0;
}
