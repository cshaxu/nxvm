#include "type.h"

#include <stdio.h>

#include "lib/storage/file_interface.h"
#include "vm/machine/debug.h"

C_INT main(C_VOID)
{
    static const C_CHAR file_name[] = "vm-debugger-recording-lifecycle-smoke.tmp";
    t_debug first;
    t_debug second;
    core_machine_debug_instruction_observation observation = {0};
    C_VOID *record = STD_NULL;
    STD_SIZE_T record_size = 0u;

    vm_machine_debug_initialize(&first);
    vm_machine_debug_initialize(&second);
    vm_machine_debug_set_trace(&first, 2u);
    if (!first.data.flagTrace || second.data.flagTrace ||
        vm_machine_debug_record_start(&first, STD_NULL) !=
            TYPE_STATUS_INVALID_ARGUMENT) return 1;

    if (vm_machine_debug_record_start(&first, ".") != TYPE_STATUS_FAULT ||
        first.connect.recordFile != STD_NULL ||
        vm_machine_debug_record_status(&first) != TYPE_STATUS_FAULT) return 1;

    if (vm_machine_debug_record_start(&first, file_name) != TYPE_STATUS_OK ||
        first.connect.recordFile == STD_NULL || second.connect.recordFile != STD_NULL)
        return 1;
    vm_machine_debug_refresh(&first, &observation);
    if (vm_machine_debug_record_stop(&first) != TYPE_STATUS_OK ||
        first.connect.recordFile != STD_NULL ||
        vm_machine_debug_record_status(&first) != TYPE_STATUS_OK ||
        lib_storage_file_read_owned(file_name, 4096u, &record, &record_size) !=
            LIB_STATUS_OK || record_size == 0u) return 1;
    STD_FREE(record);
    if (remove(file_name) != 0) return 1;
    if (first.connect.recordFile != STD_NULL ||
        vm_machine_debug_record_status(&first) != TYPE_STATUS_OK)
        return 1;
    if (vm_machine_debug_record_stop(&second) != TYPE_STATUS_INVALID_STATE) return 1;
    if (vm_machine_debug_record_status(&second) != TYPE_STATUS_INVALID_STATE) return 1;
    puts("M5:T446:S1:VM-DEBUGGER-RECORDING-LIFECYCLE:OK");
    return 0;
}
