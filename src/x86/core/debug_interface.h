#ifndef CORE_MACHINE_DEBUG_INTERFACE_H
#define CORE_MACHINE_DEBUG_INTERFACE_H
#include "lib/types/types_interface.h"





#include "x86/core/machine_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

lib_status core_machine_debug_read_cpu(
    const core_machine *machine,
    core_machine_cpu_state *out_state);
lib_status core_machine_debug_read_memory(
    const core_machine *machine,
    lib_u32 physical,
    void *out_data,
    lib_size size);
lib_status core_machine_debug_step(
    core_machine *machine,
    core_machine_run_result *out_result);
lib_status core_machine_debug_continue(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *out_result);


lib_status core_machine_debug_capture_instruction_observation(
    const core_machine *machine,
    core_machine_debug_instruction_observation *out_observation);
lib_status core_machine_debug_capture_cpu_snapshot(const core_machine *machine,
    core_machine_cpu_snapshot_point point,
    core_machine_debug_cpu_snapshot *out_snapshot);
lib_status core_machine_debug_read_register(
    const core_machine *machine, core_machine_debug_register register_id,
    lib_u32 *out_value);
lib_status core_machine_debug_write_register(
    core_machine *machine, core_machine_debug_register register_id,
    lib_u32 value);
lib_status core_machine_debug_patch_registers(core_machine *machine,
    const core_machine_debug_register_patch *patch);
lib_status core_machine_debug_get_code_default_size(
    const core_machine *machine, lib_i32 *out_default_size);
lib_status core_machine_debug_get_code_base(
    const core_machine *machine, lib_u32 *out_base);
lib_status core_machine_debug_read_linear(core_machine *machine,
    lib_u32 address, void *out_data, lib_u8 size);
lib_status core_machine_debug_write_linear(core_machine *machine,
    lib_u32 address, const void *data, lib_u8 size);
lib_status core_machine_debug_read_real(core_machine *machine, lib_u16 segment,
    lib_u16 offset, void *out_data, lib_size size);
lib_status core_machine_debug_write_real(core_machine *machine, lib_u16 segment,
    lib_u16 offset, const void *data, lib_size size);
lib_status core_machine_debug_read_port(core_machine *machine, lib_u16 port,
    lib_u32 *out_value);
lib_status core_machine_debug_write_port(core_machine *machine, lib_u16 port,
    lib_u32 value);
lib_status core_machine_debug_set_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind, lib_u32 address);
lib_status core_machine_debug_clear_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind);
lib_status core_machine_debug_get_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind, lib_u8 *out_enabled,
    lib_u32 *out_address);

#ifdef __cplusplus
}
#endif

#endif
