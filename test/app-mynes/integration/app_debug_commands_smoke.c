#include <assert.h>
#include <string.h>

#include "product/command.h"
#include "common/machine/machine_interface.h"
#include "core/driver_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-app-debug-commands-fixture.nes"

static int app_command_output_compare(const char *actual, const char *expected)
{
    lib_size length = strlen(expected);
    return memcmp(actual, expected, length) != 0 || actual[length] != '\n' ||
        actual[length + 1u] != '\0';
}

#undef lib_c_strcmp
#define lib_c_strcmp app_command_output_compare

static void make_fixture(lib_u8 *bytes)
{
    static const lib_u8 program[] = { 0xa9u, 0x2au, 0xeau, 0x4cu, 0x02u, 0x80u };
    memset(bytes, 0, 16u + 16384u);
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0x00u;
    bytes[16u + 0x3ffdu] = 0x80u;
}

static void write_fixture(const lib_u8 *bytes, lib_size size)
{
    lib_storage_file_writer *writer = 0;
    assert(lib_storage_file_writer_open(FIXTURE_PATH, LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, bytes, size) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void wait_for_paused(common_machine *machine)
{
    lib_u32 attempt;
    for (attempt = 0u; attempt < 1000u; ++attempt) {
        if (common_machine_state_get(machine) == COMMON_MACHINE_PAUSED) return;
        base_sync_sleep_milliseconds(1u);
    }
    assert(0 && "machine did not pause");
}

static void submit(app_command_context *context, const char *line,
    common_session_command_result *result)
{
    app_command_submit_line(context, COMMON_SESSION_MACHINE_PAUSED, line, result);
    assert(result->arm_prompt && !result->exit_requested);
}

int main(void)
{
    lib_u8 fixture[16u + 16384u];
    core_driver *driver = 0;
    common_machine *machine = 0;
    common_machine_driver common_driver;
    app_command_context command;
    common_session_command_result result;

    make_fixture(fixture);
    write_fixture(fixture, sizeof(fixture));
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_make_driver(driver, &common_driver) == LIB_STATUS_OK);
    assert(common_machine_create(&machine, &common_driver) == LIB_STATUS_OK);
    assert(common_machine_set_removable_media(machine, FIXTURE_PATH,
        LIB_STORAGE_MEDIUM_READONLY));
    assert(common_machine_reset(machine));
    wait_for_paused(machine);
    app_command_initialize(&command, machine, LIB_TRUE, LIB_FALSE,
        COMMON_SESSION_DISPLAY_WINDOW);

    submit(&command, "debug REGS", &result);
    assert(lib_c_strcmp(result.text, "A=00 X=00 Y=00 S=FD P=24 PC=8000 cycles=7 instructions=0\n") == 0);
    submit(&command, "debug PoKe $10 $55 170", &result);
    assert(lib_c_strcmp(result.text, "Memory updated.\n") == 0);
    submit(&command, "debug mem 16 2", &result);
    assert(lib_c_strcmp(result.text, "0010: 55 AA\n") == 0);
    submit(&command, "set mem.count 2", &result);
    assert(lib_c_strcmp(result.text, "Unknown command.\n") == 0);
    submit(&command, "debug mem 16 2", &result);
    assert(lib_c_strcmp(result.text, "0010: 55 AA\n") == 0);
    submit(&command, "set disasm.count 3", &result);
    assert(lib_c_strcmp(result.text, "Unknown command.\n") == 0);
    submit(&command, "debug disasm $8000 3", &result);
    assert(lib_c_strcmp(result.text,
        "8000: A9 2A    LDA #$2A\n"
        "8002: EA       NOP\n"
        "8003: 4C 02 80 JMP $8002\n") == 0);
    submit(&command, "debug step", &result);
    assert(lib_c_strcmp(result.text, "Stepped 1 instruction(s), 2 cycle(s); PC=8002.\n") == 0);
    submit(&command, "debug break $8002", &result);
    assert(lib_c_strcmp(result.text, "Set breakpoint 8002.\n") == 0);
    submit(&command, "debug breaks", &result);
    assert(lib_c_strcmp(result.text, "Breakpoints: 8002\n") == 0);
    submit(&command, "debug delete 32770", &result);
    assert(lib_c_strcmp(result.text, "Removed breakpoint 8002.\n") == 0);
    submit(&command, "debug reset", &result);
    assert(lib_c_strcmp(result.text, "Soft reset complete; machine paused.\n") == 0);
    app_command_submit_line(&command, COMMON_SESSION_MACHINE_RUNNING, "debug regs", &result);
    assert(lib_c_strcmp(result.text, "Pause with a cartridge before debugging.\n") == 0);

    assert(common_machine_shutdown(machine) == LIB_STATUS_OK);
    assert(common_machine_destroy(machine) == LIB_STATUS_OK);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    return 0;
}
