#include "type.h"

#include "vm/machine/debug.h"

static C_INT open_fails;
static C_INT write_fails;
static C_INT close_fails;
static STD_SIZE_T close_count;

STD_FILE *test_debug_record_open(const C_CHAR *file_name, const C_CHAR *mode)
{
    (C_VOID)file_name;
    (C_VOID)mode;
    return open_fails ? STD_NULL : (STD_FILE *)1;
}

C_INT test_debug_record_write(STD_FILE *file, const C_CHAR *format, ...)
{
    (C_VOID)file;
    (C_VOID)format;
    return write_fails ? -1 : 1;
}

C_INT test_debug_record_close(STD_FILE *file)
{
    (C_VOID)file;
    close_count++;
    return close_fails ? -1 : 0;
}

C_INT main(C_VOID)
{
    t_debug first;
    t_debug second;
    core_machine_debug_instruction_observation observation = {0};

    vm_machine_debug_initialize(&first);
    vm_machine_debug_initialize(&second);
    vm_machine_debug_set_trace(&first, 2u);
    if (!first.data.flagTrace || second.data.flagTrace ||
        vm_machine_debug_record_start(&first, STD_NULL) !=
            TYPE_STATUS_INVALID_ARGUMENT) return 1;

    open_fails = 1;
    if (vm_machine_debug_record_start(&first, "ignored") != TYPE_STATUS_FAULT ||
        first.connect.recordFile != STD_NULL ||
        vm_machine_debug_record_status(&first) != TYPE_STATUS_FAULT) return 1;

    open_fails = 0;
    if (vm_machine_debug_record_start(&first, "ignored") != TYPE_STATUS_OK ||
        first.connect.recordFile == STD_NULL || second.connect.recordFile != STD_NULL)
        return 1;
    write_fails = 1;
    vm_machine_debug_refresh(&first, &observation);
    write_fails = 0;
    if (first.connect.recordFile != STD_NULL ||
        vm_machine_debug_record_status(&first) != TYPE_STATUS_FAULT ||
        close_count != 1u) return 1;

    if (vm_machine_debug_record_start(&first, "ignored") != TYPE_STATUS_OK) return 1;
    close_fails = 1;
    if (vm_machine_debug_record_stop(&first) != TYPE_STATUS_FAULT ||
        first.connect.recordFile != STD_NULL ||
        vm_machine_debug_record_status(&first) != TYPE_STATUS_FAULT) return 1;

    close_fails = 0;
    if (vm_machine_debug_record_start(&first, "ignored") != TYPE_STATUS_OK) return 1;
    vm_machine_debug_finalize(&first);
    if (first.connect.recordFile != STD_NULL ||
        vm_machine_debug_record_status(&first) != TYPE_STATUS_OK || close_count != 3u)
        return 1;
    if (vm_machine_debug_record_stop(&second) != TYPE_STATUS_INVALID_STATE) return 1;
    if (vm_machine_debug_record_status(&second) != TYPE_STATUS_INVALID_STATE) return 1;
    puts("M5:T446:S1:VM-DEBUGGER-RECORDING-LIFECYCLE:OK");
    return 0;
}
