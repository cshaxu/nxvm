#include "lib/storage/file_interface.h"
#include "lib/types/file.h"
#include "myraidenii/core/board_interface.h"
#include "x86-core/machine/machine_interface.h"

#define MYRAIDENII_STARTUP_INSTRUCTION_BUDGET 2000000u

static lib_status myraidenii_startup_read_lane(const char *path, lib_u8 **out_lane)
{
    void *bytes = LIB_NULL;
    lib_size byte_count = 0u;
    lib_status status;

    if (path == LIB_NULL || out_lane == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_lane = LIB_NULL;
    status = lib_storage_file_read_owned(path, MYRAIDENII_CORE_PROGRAM_LANE_BYTES,
        &bytes, &byte_count);
    if (status != LIB_STATUS_OK) return status;
    if (byte_count != MYRAIDENII_CORE_PROGRAM_LANE_BYTES) {
        lib_release(bytes);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_lane = bytes;
    return LIB_STATUS_OK;
}

int main(int argument_count, char **arguments)
{
    const myraidenii_core_board_config config = {
        .program_lane_bytes = MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    lib_u8 *program_even = LIB_NULL;
    lib_u8 *program_odd = LIB_NULL;
    lib_u64 remaining = MYRAIDENII_STARTUP_INSTRUCTION_BUDGET;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_diagnostic diagnostic = {0};
    lib_status status;
    lib_bool terminal_fault = LIB_FALSE;
    int exit_code = 1;

    if (argument_count != 3) {
        lib_c_printf("MYRAIDENII:STARTUP-PROBE:USAGE\n");
        return 2;
    }
    status = myraidenii_startup_read_lane(arguments[1], &program_even);
    if (status == LIB_STATUS_OK)
        status = myraidenii_startup_read_lane(arguments[2], &program_odd);
    if (status == LIB_STATUS_OK) {
        myraidenii_core_board_config mutable_config = config;

        mutable_config.program_even = program_even;
        mutable_config.program_odd = program_odd;
        status = myraidenii_core_board_create(&mutable_config, &board);
    }
    machine = myraidenii_core_board_machine_get(board);
    while (status == LIB_STATUS_OK && remaining != 0u) {
        const lib_u64 quantum = remaining < 4096u ? remaining : 4096u;

        status = x86_core_machine_run(machine, (x86_core_machine_run_budget){quantum, 0u},
            &result);
        terminal_fault = status == LIB_STATUS_INTERNAL_ERROR &&
            result.reason == X86_CORE_MACHINE_STOP_FAULT;
        if ((!terminal_fault && status != LIB_STATUS_OK) || result.executed > remaining) break;
        remaining -= result.executed;
        if (terminal_fault || result.reason != X86_CORE_MACHINE_STOP_BUDGET) break;
        if (result.executed == 0u) {
            status = LIB_STATUS_INTERNAL_ERROR;
            break;
        }
    }
    if (status == LIB_STATUS_OK || terminal_fault) {
        const lib_u32 fault_code = terminal_fault &&
            x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) == LIB_STATUS_OK &&
            diagnostic.first_fault.valid ? diagnostic.first_fault.exception_code : 0u;

        lib_c_printf("MYRAIDENII:STARTUP-PROBE:OK reason=%u detail=%u fault-code=%u "
            "executed=%llu ticks=%llu\n",
            (lib_u32)result.reason, result.detail, fault_code,
            (unsigned long long)(MYRAIDENII_STARTUP_INSTRUCTION_BUDGET - remaining),
            (unsigned long long)result.elapsed_ticks);
        exit_code = 0;
    } else {
        lib_c_printf("MYRAIDENII:STARTUP-PROBE:FAIL status=%u\n", (lib_u32)status);
    }
    myraidenii_core_board_destroy(board);
    lib_release(program_odd);
    lib_release(program_even);
    return exit_code;
}
