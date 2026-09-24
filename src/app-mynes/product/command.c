#include "product/command.h"

#include "core/debug_interface.h"
#include "core/opcode_interface.h"
#include "lib/types/file.h"
#include "lib/storage/file_interface.h"

#define APP_COMMAND_PATH_CAPACITY 1024u

/* App owns product terminology and the monitor's discoverable command
 * surface.  Keep this in the same grouped form as SoftPC's cooked monitor:
 * a user can see grammar, state policy and gameplay controls in one `help`. */
static const lib_u8 app_command_help[] =
    "MyNES\n"
    "=====\n"
    "  start               cold-reset and run the inserted cartridge\n"
    "  resume              continue a paused machine\n"
    "  pause               pause a running machine\n"
    "  stop                stop a running or paused machine\n"
    "  reset               power-reset and pause the inserted cartridge\n"
    "  rom insert <file>   insert a read-only iNES cartridge while stopped/paused\n"
    "  rom eject           eject the cartridge while stopped/paused\n"
    "  save <file>         save a running or paused machine\n"
    "  load <file>         load a snapshot while stopped\n"
    "  help                show this help\n"
    "  debug               enter the paused-machine debug command group\n"
    "  exit                close MyNES\n"
    "\n"
    "While the game is running:\n"
    "  W/A/S/D             D-pad\n"
    "  J/Ctrl              B\n"
    "  K/Alt               A\n"
    "  Enter               Start\n"
    "  Shift               Select\n"
    "  Esc                 Pause or Resume\n";

static lib_bool app_command_set_media(app_command_context *context,
                                      const char *path)
{
    if (context->set_media != LIB_NULL)
        return context->set_media(context->media_context, path);
    return common_machine_set_removable_media(context->machine, path,
                                              LIB_STORAGE_MEDIUM_READONLY);
}

static const lib_u8 app_command_debug_help[] =
    "MyNES debug commands (paused cartridge required)\n"
    "  debug regs                show RP2A03 registers\n"
    "  debug mem <addr> [count]  display 1..256 bytes\n"
    "  debug poke <addr> <bytes> write 1..64 RAM bytes\n"
    "  debug disasm <addr> [n]   disassemble 1..32 instructions\n"
    "  debug step [count]        execute 1..1000 instructions\n"
    "  debug break <addr>        add a breakpoint\n"
    "  debug delete <addr>       remove a breakpoint\n"
    "  debug breaks              list breakpoints\n"
    "  debug reset               soft-reset; preserves RAM\n";

static lib_u16 app_command_read_u16(const lib_u8 *bytes)
{
    return (lib_u16)bytes[0] | ((lib_u16)bytes[1] << 8u);
}

static lib_u32 app_command_read_u32(const lib_u8 *bytes)
{
    return (lib_u32)bytes[0] | ((lib_u32)bytes[1] << 8u) |
           ((lib_u32)bytes[2] << 16u) | ((lib_u32)bytes[3] << 24u);
}

static void app_command_write_u16(lib_u8 *bytes, lib_u16 value)
{
    bytes[0] = (lib_u8)value;
    bytes[1] = (lib_u8)(value >> 8u);
}

static void app_command_write_u32(lib_u8 *bytes, lib_u32 value)
{
    bytes[0] = (lib_u8)value;
    bytes[1] = (lib_u8)(value >> 8u);
    bytes[2] = (lib_u8)(value >> 16u);
    bytes[3] = (lib_u8)(value >> 24u);
}

static void app_command_debug_header(lib_u8 *request, lib_u16 operation,
                                     lib_u32 payload_size)
{
    app_command_write_u16(request, CORE_DEBUG_VERSION);
    app_command_write_u16(request + 2u, operation);
    app_command_write_u32(request + 4u, payload_size);
}

static lib_bool app_command_debug_execute(app_command_context *context,
                                          const lib_u8 *request, lib_size request_size, lib_u8 *response,
                                          lib_size response_capacity, lib_size *out_size)
{
    common_machine_debug_lease lease;

    if (context == LIB_NULL || context->machine == LIB_NULL || out_size == LIB_NULL ||
        common_machine_debug_acquire(context->machine, &lease) != LIB_STATUS_OK)
        return LIB_FALSE;
    return common_machine_debug_execute_with_lease(context->machine, &lease,
                                                   request, request_size, response, response_capacity, out_size) == LIB_STATUS_OK;
}

static lib_bool app_command_debug_ready(const app_command_context *context,
                                        common_session_machine_state state)
{
    return context != LIB_NULL && context->machine != LIB_NULL &&
           state == COMMON_SESSION_MACHINE_PAUSED && context->cartridge_present;
}

static lib_bool app_command_guest_trapped(app_command_context *context,
                                          lib_bool *out_trapped)
{
    lib_u8 request[8] = {0};
    lib_u8 response[64];
    lib_size size = 0u;

    if (out_trapped == LIB_NULL)
        return LIB_FALSE;
    app_command_debug_header(request, CORE_DEBUG_OBSERVE, 0u);
    if (!app_command_debug_execute(context, request, sizeof(request), response,
                                   sizeof(response), &size) ||
        size != 52u)
        return LIB_FALSE;
    *out_trapped = response[46] != 0u;
    return LIB_TRUE;
}

static const char *app_command_skip_space(const char *text)
{
    while (*text == ' ' || *text == '\t')
        ++text;
    return text;
}

static lib_bool app_command_parse_u16(const char *text, const char **out_end,
                                      lib_u16 *out_value)
{
    lib_u32 value = 0u;
    lib_u32 base = 10u;
    lib_u32 digit;
    const char *cursor = text;

    if (*cursor == '$')
    {
        base = 16u;
        ++cursor;
    }
    else if (cursor[0] == '0' && (cursor[1] == 'x' || cursor[1] == 'X'))
    {
        base = 16u;
        cursor += 2;
    }
    if (*cursor == '\0')
        return LIB_FALSE;
    for (;;)
    {
        if (*cursor >= '0' && *cursor <= '9')
            digit = (lib_u32)(*cursor - '0');
        else if (base == 16u && *cursor >= 'a' && *cursor <= 'f')
            digit = (lib_u32)(*cursor - 'a' + 10);
        else if (base == 16u && *cursor >= 'A' && *cursor <= 'F')
            digit = (lib_u32)(*cursor - 'A' + 10);
        else
            break;
        if (digit >= base || value > (65535u - digit) / base)
            return LIB_FALSE;
        value = value * base + digit;
        ++cursor;
    }
    if (cursor == text || (base == 16u && (cursor == text + 1u ||
                                           (text[0] == '0' && cursor == text + 2u))))
        return LIB_FALSE;
    *out_value = (lib_u16)value;
    *out_end = cursor;
    return LIB_TRUE;
}

static lib_bool app_command_end(const char *text)
{
    return *app_command_skip_space(text) == '\0';
}

/* Media paths cross the App/Common boundary as one bounded ASCII argument.
 * Quoting is deliberately limited to grouping; backslashes stay literal for
 * Win32 paths and no shell-style escaping is implied. */
static lib_bool app_command_parse_path(const char *text, char *out_path,
                                       lib_size capacity)
{
    const char *cursor = app_command_skip_space(text);
    lib_size length = 0u;
    lib_bool quoted = *cursor == '"';

    if (quoted)
        ++cursor;
    while (*cursor != '\0' && (quoted ? *cursor != '"' : *cursor != ' ' && *cursor != '\t'))
    {
        if ((lib_u8)*cursor < 0x20u || (lib_u8)*cursor == 0x7fu ||
            (lib_u8)*cursor > 0x7fu || length + 1u >= capacity)
            return LIB_FALSE;
        out_path[length++] = *cursor++;
    }
    if (length == 0u || (quoted && *cursor != '"'))
        return LIB_FALSE;
    if (quoted)
        ++cursor;
    if (!app_command_end(cursor))
        return LIB_FALSE;
    out_path[length] = '\0';
    return LIB_TRUE;
}

static void app_command_message(char *target, lib_size target_capacity,
                                const char *text);

static void app_command_result(common_session_command_result *out_result,
                               const char *text)
{
    *out_result = (common_session_command_result){0};
    app_command_message(out_result->text, sizeof(out_result->text), text);
    out_result->arm_prompt = LIB_TRUE;
    out_result->prompt[0] = 'M';
    out_result->prompt[1] = 'y';
    out_result->prompt[2] = 'N';
    out_result->prompt[3] = 'e';
    out_result->prompt[4] = 's';
    out_result->prompt[5] = '>';
    out_result->prompt[6] = ' ';
}

static lib_status app_command_snapshot_write(void *opaque, const lib_u8 *bytes,
                                             lib_size count)
{
    return lib_storage_file_writer_write((lib_storage_file_writer *)opaque,
                                         bytes, count);
}

static lib_status app_command_snapshot_read(void *opaque, lib_u8 *bytes,
                                            lib_size count)
{
    return lib_storage_file_reader_read((lib_storage_file_reader *)opaque, bytes, count);
}

static void app_command_complete_snapshot(app_command_context *context,
                                          app_command_snapshot_result pending, lib_status status, lib_bool was_paused,
                                          common_session_command_result *out_result)
{
    if (pending == APP_COMMAND_SNAPSHOT_SAVED && was_paused)
    {
        app_command_result(out_result, status == LIB_STATUS_OK ? "Machine saved and paused.\n" : "Cannot save machine state.\n");
    }
    else if (status == LIB_STATUS_OK ||
             (pending == APP_COMMAND_SNAPSHOT_SAVED &&
              common_machine_state_get(context->machine) == COMMON_MACHINE_PAUSED))
    {
        context->session.pending_snapshot = status == LIB_STATUS_OK ? pending : APP_COMMAND_SNAPSHOT_SAVE_FAILED;
        context->session.transition_pending = LIB_TRUE;
        context->session.prompt_due = LIB_FALSE;
        *out_result = (common_session_command_result){0};
    }
    else
        app_command_result(out_result, pending == APP_COMMAND_SNAPSHOT_SAVED ? "Cannot save machine state.\n" : "Cannot load machine state.\n");
}

static void app_command_save_snapshot(app_command_context *context, const char *path,
                                      common_session_command_result *out_result)
{
    lib_storage_file_writer *writer = LIB_NULL;
    lib_bool was_paused = common_machine_state_get(context->machine) ==
                          COMMON_MACHINE_PAUSED;
    lib_status status = lib_storage_file_writer_open(path,
                                                     LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer);
    if (status == LIB_STATUS_OK)
        status = common_machine_read_state(context->machine,
                                           &(common_machine_state_writer){app_command_snapshot_write, writer});
    if (writer != LIB_NULL && lib_storage_file_writer_close(writer) != LIB_STATUS_OK &&
        status == LIB_STATUS_OK)
        status = LIB_STATUS_IO_ERROR;
    app_command_complete_snapshot(context, APP_COMMAND_SNAPSHOT_SAVED, status,
                                  was_paused, out_result);
}

static void app_command_load_snapshot(app_command_context *context, const char *path,
                                      common_session_command_result *out_result)
{
    lib_storage_file_reader *reader = LIB_NULL;
    lib_status status = lib_storage_file_reader_open(path, &reader);
    if (status == LIB_STATUS_OK)
        status = common_machine_write_state(context->machine,
                                            &(common_machine_state_reader){app_command_snapshot_read, reader});
    if (reader != LIB_NULL && lib_storage_file_reader_close(reader) != LIB_STATUS_OK &&
        status == LIB_STATUS_OK)
        status = LIB_STATUS_IO_ERROR;
    app_command_complete_snapshot(context, APP_COMMAND_SNAPSHOT_LOADED, status,
                                  LIB_FALSE, out_result);
}

static void app_command_message(char *target, lib_size target_capacity,
                                const char *text)
{
    lib_size length = text == LIB_NULL ? 0u : lib_text_length(text);

    if (target == LIB_NULL || target_capacity == 0u)
        return;
    if (length >= target_capacity)
        length = target_capacity - 1u;
    if (length != 0u)
        lib_memory_copy(target, text, length);
    target[length] = '\0';
    if (length != 0u && target[length - 1u] != '\n' && length + 1u < target_capacity)
        target[length++] = '\n';
    if (length != 0u && length + 1u < target_capacity)
        target[length++] = '\n';
    target[length] = '\0';
}

static void app_command_outcome(app_command_context *context, const char *text)
{
    if (context == LIB_NULL)
        return;
    app_command_message((char *)context->session.pending_monitor_text,
                        sizeof(context->session.pending_monitor_text), text);
    context->session.prompt_due = LIB_TRUE;
}

static void app_command_finish_output(common_session_command_result *out_result)
{
    lib_size length;

    if (out_result == LIB_NULL || out_result->text[0] == '\0')
        return;
    length = lib_text_length(out_result->text);
    if (out_result->text[length - 1u] != '\n' &&
        length + 1u < sizeof(out_result->text))
        out_result->text[length++] = '\n';
    if (length + 1u < sizeof(out_result->text))
        out_result->text[length++] = '\n';
    out_result->text[length] = '\0';
}

static void app_command_request(app_command_context *context,
                                common_session_command_result *out_result, common_session_request request)
{
    context->session.transition_pending = LIB_TRUE;
    context->session.prompt_due = LIB_FALSE;
    out_result->request = request;
    out_result->arm_prompt = LIB_FALSE;
}

static lib_bool app_command_equal(const char *left, const char *right)
{
    if (left == LIB_NULL || right == LIB_NULL)
        return LIB_FALSE;
    while (*left != '\0' && *right != '\0')
    {
        char left_char = *left;
        char right_char = *right;
        if (left_char >= 'A' && left_char <= 'Z')
            left_char = (char)(left_char + ('a' - 'A'));
        if (right_char >= 'A' && right_char <= 'Z')
            right_char = (char)(right_char + ('a' - 'A'));
        if (left_char != right_char)
            return LIB_FALSE;
        ++left;
        ++right;
    }
    return *left == '\0' && *right == '\0';
}

static const char *app_command_after(const char *line, const char *prefix)
{
    lib_size length = lib_text_length(prefix);
    lib_size index;

    if (line == LIB_NULL || lib_text_length(line) <= length)
        return LIB_NULL;
    for (index = 0u; index < length; ++index)
    {
        char left = line[index];
        char right = prefix[index];
        if (left >= 'A' && left <= 'Z')
            left = (char)(left + ('a' - 'A'));
        if (right >= 'A' && right <= 'Z')
            right = (char)(right + ('a' - 'A'));
        if (left != right)
            return LIB_NULL;
    }
    return line + length;
}

static void app_command_regs(app_command_context *context,
                             common_session_command_result *out_result)
{
    lib_u8 request[8] = {0};
    lib_u8 response[64];
    lib_size size = 0u;

    app_command_debug_header(request, CORE_DEBUG_OBSERVE, 0u);
    if (!app_command_debug_execute(context, request, sizeof(request), response,
                                   sizeof(response), &size) ||
        size != 52u)
    {
        app_command_result(out_result, "Debugger request failed.\n");
        return;
    }
    lib_c_snprintf(out_result->text, sizeof(out_result->text),
                   "A=%02X X=%02X Y=%02X S=%02X P=%02X PC=%04X cycles=%llu instructions=%llu\n",
                   response[12], response[13], response[14], response[15], response[16],
                   app_command_read_u16(response + 18u),
                   (unsigned long long)((lib_u64)app_command_read_u32(response + 20u) |
                                        ((lib_u64)app_command_read_u32(response + 24u) << 32u)),
                   (unsigned long long)((lib_u64)app_command_read_u32(response + 28u) |
                                        ((lib_u64)app_command_read_u32(response + 32u) << 32u)));
    app_command_finish_output(out_result);
}

static void app_command_mem(app_command_context *context, const char *arguments,
                            common_session_command_result *out_result)
{
    lib_u8 request[12] = {0};
    lib_u8 response[272];
    lib_u16 address;
    lib_u16 count = 64u;
    const char *cursor = app_command_skip_space(arguments);
    lib_size size = 0u;
    lib_size written = 0u;
    lib_u32 index;

    if (!app_command_parse_u16(cursor, &cursor, &address))
    {
        app_command_result(out_result, "mem requires an address.\n");
        return;
    }
    cursor = app_command_skip_space(cursor);
    if (!app_command_end(cursor) && (!app_command_parse_u16(cursor, &cursor, &count) ||
                                     count == 0u || count > 256u || !app_command_end(cursor)))
    {
        app_command_result(out_result, "mem count must be 1..256.\n");
        return;
    }
    app_command_debug_header(request, CORE_DEBUG_PEEK, 4u);
    app_command_write_u16(request + 8u, address);
    app_command_write_u16(request + 10u, count);
    if (!app_command_debug_execute(context, request, sizeof(request), response,
                                   sizeof(response), &size) ||
        size != 16u + count)
    {
        app_command_result(out_result, "Memory is not readable at that range.\n");
        return;
    }
    for (index = 0u; index < count && written + 4u < sizeof(out_result->text); ++index)
    {
        if (index % 16u == 0u)
            written += (lib_size)lib_c_snprintf(out_result->text + written,
                                                sizeof(out_result->text) - written, "%04X:", (unsigned)(address + index));
        written += (lib_size)lib_c_snprintf(out_result->text + written,
                                            sizeof(out_result->text) - written, " %02X", response[16u + index]);
        if (index % 16u == 15u || index + 1u == count)
            written += (lib_size)lib_c_snprintf(out_result->text + written,
                                                sizeof(out_result->text) - written, "\n");
    }
    app_command_finish_output(out_result);
}

static void app_command_poke(app_command_context *context, const char *arguments,
                             common_session_command_result *out_result)
{
    lib_u8 request[76] = {0};
    lib_u8 response[16];
    lib_u16 address;
    lib_u16 value;
    const char *cursor = app_command_skip_space(arguments);
    lib_u32 count = 0u;
    lib_size size = 0u;

    if (!app_command_parse_u16(cursor, &cursor, &address))
    {
        app_command_result(out_result, "poke requires an address and bytes.\n");
        return;
    }
    while (!app_command_end(cursor))
    {
        cursor = app_command_skip_space(cursor);
        if (!app_command_parse_u16(cursor, &cursor, &value) || value > 255u || count == 64u)
        {
            app_command_result(out_result, "poke accepts 1..64 byte values.\n");
            return;
        }
        request[12u + count++] = (lib_u8)value;
    }
    if (count == 0u)
    {
        app_command_result(out_result, "poke requires at least one byte.\n");
        return;
    }
    app_command_debug_header(request, CORE_DEBUG_POKE, 4u + count);
    app_command_write_u16(request + 8u, address);
    app_command_write_u16(request + 10u, (lib_u16)count);
    if (!app_command_debug_execute(context, request, 12u + count, response,
                                   sizeof(response), &size) ||
        size != 12u)
    {
        app_command_result(out_result, "poke accepts RAM addresses only.\n");
        return;
    }
    app_command_result(out_result, "Memory updated.\n");
}

static void app_command_step(app_command_context *context, const char *arguments,
                             common_session_command_result *out_result)
{
    lib_u8 request[12] = {0};
    lib_u8 response[64];
    lib_u16 count = 1u;
    const char *cursor = app_command_skip_space(arguments);
    lib_size size = 0u;
    lib_bool trapped;

    if (!app_command_end(cursor) && (!app_command_parse_u16(cursor, &cursor, &count) ||
                                     count == 0u || count > 1000u || !app_command_end(cursor)))
    {
        app_command_result(out_result, "step count must be 1..1000.\n");
        return;
    }
    app_command_debug_header(request, CORE_DEBUG_STEP, 4u);
    app_command_write_u32(request + 8u, count);
    if (!app_command_guest_trapped(context, &trapped))
    {
        app_command_result(out_result, "Debugger request failed.\n");
        return;
    }
    if (trapped)
    {
        app_command_result(out_result, "Guest trap is latched; reset or replace the cartridge.\n");
        return;
    }
    if (!app_command_debug_execute(context, request, sizeof(request), response,
                                   sizeof(response), &size) ||
        size != 60u)
    {
        app_command_result(out_result, "step failed.\n");
        return;
    }
    lib_c_snprintf(out_result->text, sizeof(out_result->text),
                   "Stepped %u instruction(s), %u cycle(s); PC=%04X.\n",
                   (unsigned)app_command_read_u32(response + 12u),
                   (unsigned)app_command_read_u32(response + 16u),
                   app_command_read_u16(response + 26u));
    app_command_finish_output(out_result);
}

static void app_command_break_set(app_command_context *context, const char *arguments,
                                  lib_bool enabled, common_session_command_result *out_result)
{
    lib_u8 request[12] = {0};
    lib_u8 response[16];
    lib_u16 address;
    const char *cursor = app_command_skip_space(arguments);
    lib_size size = 0u;

    if (!app_command_parse_u16(cursor, &cursor, &address) || !app_command_end(cursor))
    {
        app_command_result(out_result, "break requires one address.\n");
        return;
    }
    app_command_debug_header(request, CORE_DEBUG_BREAK_SET, 4u);
    app_command_write_u16(request + 8u, address);
    app_command_write_u16(request + 10u, enabled ? 1u : 0u);
    if (!app_command_debug_execute(context, request, sizeof(request), response,
                                   sizeof(response), &size) ||
        size != 12u)
    {
        app_command_result(out_result, "breakpoint request failed.\n");
        return;
    }
    lib_c_snprintf(out_result->text, sizeof(out_result->text), "%s breakpoint %04X.\n",
                   enabled ? "Set" : "Removed", address);
    app_command_finish_output(out_result);
}

static void app_command_break_list(app_command_context *context,
                                   common_session_command_result *out_result)
{
    lib_u8 request[8] = {0};
    lib_u8 response[64];
    lib_u16 count;
    lib_size size = 0u;
    lib_u32 index;
    lib_size written;

    app_command_debug_header(request, CORE_DEBUG_BREAK_LIST, 0u);
    if (!app_command_debug_execute(context, request, sizeof(request), response,
                                   sizeof(response), &size) ||
        size < 16u)
    {
        app_command_result(out_result, "breakpoint request failed.\n");
        return;
    }
    count = app_command_read_u16(response + 12u);
    if (size != 16u + (lib_size)count * 2u)
    {
        app_command_result(out_result, "Debugger response was invalid.\n");
        return;
    }
    written = (lib_size)lib_c_snprintf(out_result->text, sizeof(out_result->text),
                                       "Breakpoints:");
    for (index = 0u; index < count; ++index)
        written += (lib_size)lib_c_snprintf(out_result->text + written,
                                            sizeof(out_result->text) - written, " %04X", app_command_read_u16(response + 16u + index * 2u));
    (void)lib_c_snprintf(out_result->text + written,
                         sizeof(out_result->text) - written, "\n");
    app_command_finish_output(out_result);
}

static lib_size app_command_format_instruction(char *text, lib_size capacity,
                                               lib_u16 address, const lib_u8 *bytes)
{
    core_opcode_metadata info;
    lib_u16 word = (lib_u16)bytes[1] | ((lib_u16)bytes[2] << 8u);

    if (!core_opcode_describe(bytes[0], &info))
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X       .byte $%02X\n",
                                        address, bytes[0], bytes[0]);
    if (info.mode == CORE_OPCODE_ADDRESS_IMPLIED)
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X       %s\n",
                                        address, bytes[0], (const char *)info.mnemonic);
    if (info.mode == CORE_OPCODE_ADDRESS_ACCUMULATOR)
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X       %s A\n",
                                        address, bytes[0], (const char *)info.mnemonic);
    if (info.mode == CORE_OPCODE_ADDRESS_IMMEDIATE)
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X %02X    %s #$%02X\n",
                                        address, bytes[0], bytes[1], (const char *)info.mnemonic, bytes[1]);
    if (info.mode == CORE_OPCODE_ADDRESS_ZERO_PAGE)
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X %02X    %s $%02X\n",
                                        address, bytes[0], bytes[1], (const char *)info.mnemonic, bytes[1]);
    if (info.mode == CORE_OPCODE_ADDRESS_ZERO_PAGE_X ||
        info.mode == CORE_OPCODE_ADDRESS_ZERO_PAGE_Y)
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X %02X    %s $%02X,%c\n",
                                        address, bytes[0], bytes[1], (const char *)info.mnemonic, bytes[1],
                                        info.mode == CORE_OPCODE_ADDRESS_ZERO_PAGE_X ? 'X' : 'Y');
    if (info.mode == CORE_OPCODE_ADDRESS_INDIRECT_X)
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X %02X    %s ($%02X,X)\n",
                                        address, bytes[0], bytes[1], (const char *)info.mnemonic, bytes[1]);
    if (info.mode == CORE_OPCODE_ADDRESS_INDIRECT_Y)
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X %02X    %s ($%02X),Y\n",
                                        address, bytes[0], bytes[1], (const char *)info.mnemonic, bytes[1]);
    if (info.mode == CORE_OPCODE_ADDRESS_RELATIVE)
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X %02X    %s $%04X\n",
                                        address, bytes[0], bytes[1], (const char *)info.mnemonic,
                                        (lib_u16)(address + 2u + (bytes[1] < 0x80u ? bytes[1] : (lib_u16)(0xff00u | bytes[1]))));
    if (info.mode == CORE_OPCODE_ADDRESS_INDIRECT)
        return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X %02X %02X %s ($%04X)\n",
                                        address, bytes[0], bytes[1], bytes[2], (const char *)info.mnemonic, word);
    return (lib_size)lib_c_snprintf(text, capacity, "%04X: %02X %02X %02X %s $%04X%s\n",
                                    address, bytes[0], bytes[1], bytes[2], (const char *)info.mnemonic, word,
                                    info.mode == CORE_OPCODE_ADDRESS_ABSOLUTE_X ? ",X" : info.mode == CORE_OPCODE_ADDRESS_ABSOLUTE_Y ? ",Y"
                                                                                                                                     : "");
}

static void app_command_disasm(app_command_context *context, const char *arguments,
                               common_session_command_result *out_result)
{
    lib_u8 request[12] = {0};
    lib_u8 response[112];
    lib_u16 address;
    lib_u16 count = 8u;
    const char *cursor = app_command_skip_space(arguments);
    lib_size size = 0u;
    lib_size written = 0u;
    lib_u32 index;
    lib_u32 byte_offset = 0u;

    if (!app_command_parse_u16(cursor, &cursor, &address))
    {
        app_command_result(out_result, "disasm requires an address.\n");
        return;
    }
    cursor = app_command_skip_space(cursor);
    if (!app_command_end(cursor) && (!app_command_parse_u16(cursor, &cursor, &count) ||
                                     count == 0u || count > 32u || !app_command_end(cursor)))
    {
        app_command_result(out_result, "disasm count must be 1..32.\n");
        return;
    }
    if ((lib_u32)address + (lib_u32)count * 3u > 65536u)
    {
        app_command_result(out_result, "disasm range wraps the address space.\n");
        return;
    }
    app_command_debug_header(request, CORE_DEBUG_PEEK, 4u);
    app_command_write_u16(request + 8u, address);
    app_command_write_u16(request + 10u, (lib_u16)(count * 3u));
    if (!app_command_debug_execute(context, request, sizeof(request), response,
                                   sizeof(response), &size) ||
        size != 16u + (lib_size)count * 3u)
    {
        app_command_result(out_result, "Disassembly range is not readable.\n");
        return;
    }
    for (index = 0u; index < count && written < sizeof(out_result->text); ++index)
    {
        core_opcode_metadata info;
        lib_bool known = core_opcode_describe(response[16u + byte_offset], &info);

        written += app_command_format_instruction(out_result->text + written,
                                                  sizeof(out_result->text) - written, (lib_u16)(address + byte_offset),
                                                  response + 16u + byte_offset);
        byte_offset += known ? info.bytes : 1u;
    }
    app_command_finish_output(out_result);
}

static void app_command_debug_submit(app_command_context *context,
                                     common_session_machine_state state, const char *arguments,
                                     common_session_command_result *out_result)
{
    const char *path;

    if (app_command_end(arguments) || app_command_equal(arguments, "help"))
    {
        app_command_result(out_result, (const char *)app_command_debug_help);
    }
    else if (!app_command_debug_ready(context, state))
    {
        app_command_result(out_result, "Pause with a cartridge before debugging.\n");
    }
    else if (app_command_equal(arguments, "regs"))
    {
        app_command_regs(context, out_result);
    }
    else if ((path = app_command_after(arguments, "mem ")) != LIB_NULL)
    {
        app_command_mem(context, path, out_result);
    }
    else if ((path = app_command_after(arguments, "poke ")) != LIB_NULL)
    {
        app_command_poke(context, path, out_result);
    }
    else if ((path = app_command_after(arguments, "disasm ")) != LIB_NULL)
    {
        app_command_disasm(context, path, out_result);
    }
    else if (app_command_equal(arguments, "step") ||
             (path = app_command_after(arguments, "step ")) != LIB_NULL)
    {
        app_command_step(context, path == LIB_NULL ? "" : path, out_result);
    }
    else if ((path = app_command_after(arguments, "break ")) != LIB_NULL)
    {
        app_command_break_set(context, path, LIB_TRUE, out_result);
    }
    else if ((path = app_command_after(arguments, "delete ")) != LIB_NULL)
    {
        app_command_break_set(context, path, LIB_FALSE, out_result);
    }
    else if (app_command_equal(arguments, "breaks"))
    {
        app_command_break_list(context, out_result);
    }
    else if (app_command_equal(arguments, "reset"))
    {
        lib_u8 request[8] = {0};
        lib_u8 response[64];
        lib_size response_size = 0u;

        app_command_debug_header(request, CORE_DEBUG_WARM_RESET, 0u);
        if (!app_command_debug_execute(context, request, sizeof(request), response,
                                       sizeof(response), &response_size) ||
            response_size != 52u)
            app_command_result(out_result, "Soft reset failed.\n");
        else
            app_command_result(out_result, "Soft reset complete; machine paused.\n");
    }
    else
    {
        app_command_result(out_result, "Unknown debug command; enter debug help.\n");
    }
}

void app_command_initialize(app_command_context *context, common_machine *machine,
                            lib_bool cartridge_present, lib_bool initial_reset,
                            common_session_display display)
{
    *context = (app_command_context){
        .machine = machine,
        .cartridge_present = cartridge_present,
        .initial_reset = initial_reset,
        .initial_state_pending = !initial_reset,
        .suppress_window_after_reset = initial_reset &&
                                       display == COMMON_SESSION_DISPLAY_WINDOW,
        .session = {.display = display}};
}

void app_command_open(void *opaque, common_session_command_result *out_result)
{
    app_command_context *context = opaque;
    app_command_result(out_result,
                       (const char *)app_command_help);
    if (context != LIB_NULL && context->initial_reset)
    {
        context->initial_reset = LIB_FALSE;
        context->run_after_reset = LIB_FALSE;
        app_command_request(context, out_result, COMMON_SESSION_REQUEST_RESET);
    }
    else if (context != LIB_NULL && context->initial_state_pending)
        out_result->arm_prompt = LIB_FALSE;
}

void app_command_reject_line(void *opaque, common_session_command_result *out_result)
{
    (void)opaque;
    app_command_result(out_result, "Command is too long.\n");
}

void app_command_submit_line(void *opaque, common_session_machine_state state,
                             const char *line, common_session_command_result *out_result)
{
    app_command_context *context = opaque;
    const char *path;
    char media_path[APP_COMMAND_PATH_CAPACITY];

    app_command_result(out_result, LIB_NULL);
    if (context == LIB_NULL || line == LIB_NULL)
    {
        app_command_result(out_result, "Invalid command context.\n");
        return;
    }
    if (app_command_end(line))
        return;
    if (state == COMMON_SESSION_MACHINE_ERROR && !app_command_equal(line, "help") &&
        !app_command_equal(line, "exit"))
    {
        app_command_result(out_result, "Machine host error; exit and restart.\n");
        return;
    }
    if (context->session.transition_pending && !app_command_equal(line, "help") &&
        !app_command_equal(line, "exit"))
    {
        app_command_result(out_result, "Machine command is still pending.\n");
        return;
    }
    if (app_command_equal(line, "help"))
    {
        app_command_result(out_result, (const char *)app_command_help);
    }
    else if (app_command_equal(line, "debug"))
    {
        app_command_debug_submit(context, state, "", out_result);
    }
    else if ((path = app_command_after(line, "debug ")) != LIB_NULL)
    {
        app_command_debug_submit(context, state, path, out_result);
    }
    else if (app_command_equal(line, "save") ||
             (path = app_command_after(line, "save ")) != LIB_NULL)
    {
        if (!app_command_parse_path(path == LIB_NULL ? "" : path, media_path, sizeof(media_path)))
            app_command_result(out_result, "Usage: save <file>\n");
        else if (state != COMMON_SESSION_MACHINE_RUNNING && state != COMMON_SESSION_MACHINE_PAUSED)
            app_command_result(out_result, "Machine is stopped; use start before save.\n");
        else
            app_command_save_snapshot(context, media_path, out_result);
    }
    else if (app_command_equal(line, "load") ||
             (path = app_command_after(line, "load ")) != LIB_NULL)
    {
        if (!app_command_parse_path(path == LIB_NULL ? "" : path, media_path, sizeof(media_path)))
            app_command_result(out_result, "Usage: load <file>\n");
        else if (state != COMMON_SESSION_MACHINE_STOPPED)
            app_command_result(out_result, state == COMMON_SESSION_MACHINE_RUNNING ? "Machine is running; stop it before load.\n" : "Machine is paused; stop it before load.\n");
        else
            app_command_load_snapshot(context, media_path, out_result);
    }
    else if (app_command_equal(line, "rom insert") ||
             (path = app_command_after(line, "rom insert ")) != LIB_NULL)
    {
        if (!app_command_parse_path(path == LIB_NULL ? "" : path, media_path,
                                    sizeof(media_path)))
        {
            app_command_result(out_result, "rom insert requires one ASCII file path.\n");
        }
        else if (state != COMMON_SESSION_MACHINE_STOPPED && state != COMMON_SESSION_MACHINE_PAUSED)
        {
            app_command_result(out_result, "Stop or pause before replacing the cartridge.\n");
        }
        else if (!app_command_set_media(context, media_path))
        {
            app_command_result(out_result, "Cartridge insertion failed.\n");
        }
        else
        {
            context->cartridge_present = LIB_TRUE;
            context->run_after_reset = LIB_FALSE;
            context->suppress_window_after_reset =
                state == COMMON_SESSION_MACHINE_STOPPED &&
                context->session.display == COMMON_SESSION_DISPLAY_WINDOW;
            app_command_request(context, out_result, COMMON_SESSION_REQUEST_RESET);
        }
    }
    else if (app_command_equal(line, "rom eject"))
    {
        if (state != COMMON_SESSION_MACHINE_STOPPED && state != COMMON_SESSION_MACHINE_PAUSED)
        {
            app_command_result(out_result, "Stop or pause before ejecting the cartridge.\n");
        }
        else if (!app_command_set_media(context, LIB_NULL))
        {
            app_command_result(out_result, "Cartridge ejection failed.\n");
        }
        else
        {
            context->cartridge_present = LIB_FALSE;
            if (state == COMMON_SESSION_MACHINE_PAUSED)
            {
                app_command_request(context, out_result, COMMON_SESSION_REQUEST_STOP);
            }
            else
            {
                app_command_result(out_result, "Cartridge ejected.\n");
            }
        }
    }
    else if (app_command_after(line, "rom eject") != LIB_NULL)
    {
        app_command_result(out_result, "rom eject accepts no arguments.\n");
    }
    else if (app_command_equal(line, "start"))
    {
        if (!context->cartridge_present)
        {
            app_command_result(out_result, "Insert a cartridge before starting.\n");
        }
        else if (state == COMMON_SESSION_MACHINE_STOPPED)
        {
            context->run_after_reset = LIB_TRUE;
            context->started_after_reset = LIB_TRUE;
            app_command_request(context, out_result, COMMON_SESSION_REQUEST_RESET);
        }
        else if (state == COMMON_SESSION_MACHINE_PAUSED)
        {
            app_command_result(out_result, "Machine is paused; use resume, reset, or stop.\n");
        }
        else
        {
            app_command_result(out_result, "Machine is already running; use pause, reset, or stop.\n");
        }
    }
    else if (app_command_equal(line, "resume"))
    {
        lib_bool trapped;
        if (state == COMMON_SESSION_MACHINE_STOPPED)
            app_command_result(out_result, "Machine is stopped; use start or reset.\n");
        else if (state == COMMON_SESSION_MACHINE_RUNNING)
            app_command_result(out_result, "Machine is already running; use pause, reset, or stop.\n");
        else if (!context->cartridge_present)
            app_command_result(out_result, "Insert a cartridge before resuming.\n");
        else if (!app_command_guest_trapped(context, &trapped))
            app_command_result(out_result, "Debugger request failed.\n");
        else if (trapped)
            app_command_result(out_result,
                               "Guest trap is latched; reset or replace the cartridge.\n");
        else
            app_command_request(context, out_result, COMMON_SESSION_REQUEST_RESUME);
    }
    else if (app_command_equal(line, "pause"))
    {
        if (state == COMMON_SESSION_MACHINE_RUNNING)
        {
            app_command_request(context, out_result, COMMON_SESSION_REQUEST_PAUSE);
        }
        else
            app_command_result(out_result, "Machine is not running.\n");
    }
    else if (app_command_equal(line, "stop"))
    {
        if (state == COMMON_SESSION_MACHINE_RUNNING || state == COMMON_SESSION_MACHINE_PAUSED)
        {
            app_command_request(context, out_result, COMMON_SESSION_REQUEST_STOP);
        }
        else
            app_command_result(out_result, "Machine is already stopped.\n");
    }
    else if (app_command_equal(line, "reset"))
    {
        if (!context->cartridge_present)
        {
            app_command_result(out_result, "Insert a cartridge before reset.\n");
        }
        else
        {
            context->run_after_reset = LIB_FALSE;
            context->suppress_window_after_reset =
                state == COMMON_SESSION_MACHINE_STOPPED &&
                context->session.display == COMMON_SESSION_DISPLAY_WINDOW;
            app_command_request(context, out_result, COMMON_SESSION_REQUEST_RESET);
        }
    }
    else if (app_command_equal(line, "exit"))
    {
        out_result->exit_requested = LIB_TRUE;
        out_result->arm_prompt = LIB_FALSE;
    }
    else
    {
        app_command_result(out_result, "Unknown command.\n");
    }
}

lib_bool app_command_begin_external(void *opaque, common_session_machine_state state,
                                    common_session_request request)
{
    app_command_context *context = opaque;

    (void)state;
    if (context == LIB_NULL || context->session.transition_pending ||
        request == COMMON_SESSION_REQUEST_NONE)
        return LIB_FALSE;
    context->session.transition_pending = LIB_TRUE;
    context->session.prompt_due = LIB_FALSE;
    return LIB_TRUE;
}

lib_bool app_command_handle_hotkey(void *opaque, common_session_machine_state state,
                                   const lib_u8 *identifier, common_session_command_result *out_result)
{
    app_command_context *context = opaque;

    if (context == LIB_NULL || identifier == LIB_NULL || out_result == LIB_NULL ||
        context->session.transition_pending)
        return LIB_FALSE;
    app_command_result(out_result, LIB_NULL);
    if (lib_text_compare((const char *)identifier, "pause-toggle") == 0)
    {
        if (state == COMMON_SESSION_MACHINE_RUNNING)
            app_command_request(context, out_result, COMMON_SESSION_REQUEST_PAUSE);
        else if (state == COMMON_SESSION_MACHINE_PAUSED && context->cartridge_present)
            app_command_request(context, out_result, COMMON_SESSION_REQUEST_RESUME);
        else
            return LIB_FALSE;
        return LIB_TRUE;
    }
    return LIB_FALSE;
}

void app_command_note_runtime(void *opaque, common_session_machine_state prior,
                              common_session_machine_state completed, common_session_command_result *out_result)
{
    app_command_context *context = opaque;

    *out_result = (common_session_command_result){0};
    /* INIT only acknowledges that Common accepted a request.  It is not its
     * completion: keeping the command pending prevents Session from arming a
     * second cooked prompt between `rom insert`/`reset` and RESET_COMPLETED. */
    if (completed == COMMON_SESSION_MACHINE_INIT)
    {
        out_result->arm_prompt = LIB_FALSE;
        return;
    }
    if (context != LIB_NULL)
        context->session.transition_pending = LIB_FALSE;
    if (context != LIB_NULL && completed == COMMON_SESSION_MACHINE_RESET_COMPLETED)
    {
        if (context->run_after_reset)
        {
            context->run_after_reset = LIB_FALSE;
            app_command_request(context, out_result, COMMON_SESSION_REQUEST_RESUME);
        }
        else
            app_command_outcome(context, "Reset complete; machine paused.\n");
    }
    else if (completed == COMMON_SESSION_MACHINE_PAUSED)
    {
        if (context != LIB_NULL &&
            context->session.pending_snapshot != APP_COMMAND_SNAPSHOT_NONE)
        {
            app_command_snapshot_result pending = context->session.pending_snapshot;
            context->session.pending_snapshot = APP_COMMAND_SNAPSHOT_NONE;
            app_command_outcome(context, pending == APP_COMMAND_SNAPSHOT_SAVED ? "Machine saved and paused.\n" : pending == APP_COMMAND_SNAPSHOT_LOADED ? "Machine loaded and paused.\n"
                                                                                                                                                        : "Cannot save machine state.\n");
        }
        else if (context != LIB_NULL && context->report_suppressed_reset)
        {
            context->report_suppressed_reset = LIB_FALSE;
            app_command_outcome(context, "Reset complete; machine paused.\n");
        }
        else if (prior != COMMON_SESSION_MACHINE_PAUSED)
            app_command_outcome(context, "Machine paused.\n");
    }
    else if (completed == COMMON_SESSION_MACHINE_RUNNING)
    {
        if (context != LIB_NULL && context->session.display == COMMON_SESSION_DISPLAY_WINDOW)
        {
            app_command_outcome(context, context->started_after_reset ? "Machine started.\n" : "Machine resumed.\n");
        }
        if (context != LIB_NULL)
            context->started_after_reset = LIB_FALSE;
    }
    else if (completed == COMMON_SESSION_MACHINE_STOPPED)
    {
        if (context != LIB_NULL && context->initial_state_pending)
        {
            context->initial_state_pending = LIB_FALSE;
            context->session.prompt_due = LIB_TRUE;
        }
        else if (prior != COMMON_SESSION_MACHINE_STOPPED)
            app_command_outcome(context, "Machine stopped.\n");
    }
    else if (completed == COMMON_SESSION_MACHINE_ERROR)
    {
        if (context != LIB_NULL)
        {
            context->run_after_reset = LIB_FALSE;
            context->started_after_reset = LIB_FALSE;
        }
        app_command_outcome(context, "Machine host error; exit and restart.\n");
    }
}

void app_command_note_broker(void *opaque, common_session_machine_state state,
                             lib_bool vm_console_current, lib_bool monitor_running_surface)
{
    app_command_context *context = opaque;

    if (context == LIB_NULL)
        return;
    if (vm_console_current && state == COMMON_SESSION_MACHINE_RUNNING)
    {
        context->session.prompt_due = LIB_FALSE;
        context->session.pending_monitor_text[0] = '\0';
    }
    else if (!vm_console_current && state == COMMON_SESSION_MACHINE_RUNNING &&
             monitor_running_surface)
    {
        context->session.prompt_due = LIB_TRUE;
    }
}

void app_command_note_monitor_current(void *opaque, lib_bool current,
                                      common_session_command_result *out_result)
{
    app_command_context *context = opaque;
    app_command_result(out_result, LIB_NULL);
    out_result->arm_prompt = context != LIB_NULL && current &&
                             context->session.prompt_due;
    if (out_result->arm_prompt)
    {
        lib_size length = lib_text_length((const char *)context->session.pending_monitor_text);
        if (length >= sizeof(out_result->text))
            length = sizeof(out_result->text) - 1u;
        lib_memory_copy(out_result->text, context->session.pending_monitor_text, length);
        out_result->text[length] = '\0';
        context->session.pending_monitor_text[0] = '\0';
    }
}
