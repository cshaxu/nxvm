#include "type.h"

#include "core/product/session/command_interface.h"

static C_VOID core_product_session_command_write(
    const core_product_session_output_provider *output, const C_CHAR *line)
{
    if (output != STD_NULL && output->write_line != STD_NULL) {
        output->write_line(output->context, line);
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
        STD_SNPRINTF(line, sizeof(line), "%c %u %s", snapshots[index].selected ?
            '*' : ' ', (unsigned int)snapshots[index].id, snapshots[index].details);
        core_product_session_command_write(output, line);
    }
    STD_FREE(snapshots);
    return 1;
}

C_INT core_product_session_command_execute(core_product_session_manager *manager,
    C_INT argument_count, const C_CHAR *const *arguments,
    const core_product_session_open_options *open_options,
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
        const core_product_session_open_options argument_options = {
            argument_count - 2, arguments + 2, STD_NULL, 0u
        };
        if (open_options != STD_NULL && argument_count != 2) {
            core_product_session_command_write(output, "Invalid session options.");
            return 0;
        }
        const core_product_session_open_options *options = open_options == STD_NULL ?
            &argument_options : open_options;
        type_status status = core_product_session_manager_open_with_options(
            manager, options, &id);
        if (status != TYPE_STATUS_OK) {
            core_product_session_command_write(output,
                status == TYPE_STATUS_INVALID_ARGUMENT ?
                "Invalid session options." :
                status == TYPE_STATUS_INVALID_STATE ?
                "Requested session configuration is unavailable." :
                status == TYPE_STATUS_NO_MEMORY ?
                "Unable to open session: insufficient host memory." :
                "Unable to open session: selected assets could not be read or the machine could not be created.");
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
                core_product_session_command_write(output,
                    "No session selected. Use SESSION OPEN.");
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
