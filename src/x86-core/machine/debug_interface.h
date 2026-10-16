#ifndef X86_CORE_MACHINE_DEBUG_INTERFACE_H
#define X86_CORE_MACHINE_DEBUG_INTERFACE_H
#include "lib/types/types_interface.h"





#include "x86-core/machine/machine_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

lib_status x86_core_machine_debug_read_cpu(
    const x86_core_machine *machine,
    x86_core_cpu_state *out_state);
lib_status x86_core_machine_debug_read_memory(
    const x86_core_machine *machine,
    lib_u32 physical,
    void *out_data,
    lib_size size);
lib_status x86_core_machine_debug_step(
    x86_core_machine *machine,
    x86_core_machine_run_result *out_result);
lib_status x86_core_machine_debug_continue(
    x86_core_machine *machine,
    x86_core_machine_run_budget budget,
    x86_core_machine_run_result *out_result);


lib_status x86_core_machine_debug_capture_instruction_observation(
    const x86_core_machine *machine,
    x86_core_cpu_debug_instruction_observation *out_observation);
lib_status x86_core_machine_debug_capture_cpu_snapshot(const x86_core_machine *machine,
    x86_core_cpu_snapshot_point point,
    x86_core_cpu_debug_cpu_snapshot *out_snapshot);
lib_status x86_core_machine_debug_read_register(
    const x86_core_machine *machine, x86_core_cpu_debug_register register_id,
    lib_u32 *out_value);
lib_status x86_core_machine_debug_write_register(
    x86_core_machine *machine, x86_core_cpu_debug_register register_id,
    lib_u32 value);
lib_status x86_core_machine_debug_patch_registers(x86_core_machine *machine,
    const x86_core_cpu_debug_register_patch *patch);
lib_status x86_core_machine_debug_get_code_default_size(
    const x86_core_machine *machine, lib_i32 *out_default_size);
lib_status x86_core_machine_debug_get_code_base(
    const x86_core_machine *machine, lib_u32 *out_base);
lib_status x86_core_machine_debug_read_linear(x86_core_machine *machine,
    lib_u32 address, void *out_data, lib_u8 size);
lib_status x86_core_machine_debug_write_linear(x86_core_machine *machine,
    lib_u32 address, const void *data, lib_u8 size);
lib_status x86_core_machine_debug_read_real(x86_core_machine *machine, lib_u16 segment,
    lib_u16 offset, void *out_data, lib_size size);
lib_status x86_core_machine_debug_write_real(x86_core_machine *machine, lib_u16 segment,
    lib_u16 offset, const void *data, lib_size size);
lib_status x86_core_machine_debug_read_port(x86_core_machine *machine, lib_u16 port,
    lib_u32 *out_value);
lib_status x86_core_machine_debug_write_port(x86_core_machine *machine, lib_u16 port,
    lib_u32 value);
lib_status x86_core_machine_debug_set_watchpoint(x86_core_machine *machine,
    x86_core_cpu_debug_watch_kind kind, lib_u32 address);
lib_status x86_core_machine_debug_clear_watchpoint(x86_core_machine *machine,
    x86_core_cpu_debug_watch_kind kind);
lib_status x86_core_machine_debug_get_watchpoint(x86_core_machine *machine,
    x86_core_cpu_debug_watch_kind kind, lib_u8 *out_enabled,
    lib_u32 *out_address);

#ifdef __cplusplus
}
#endif

#endif
