#ifndef CORE_MACHINE_DEBUG_INTERFACE_H
#define CORE_MACHINE_DEBUG_INTERFACE_H

#include "type.h"



#include "core/machine/cpu_interface.h"

#include "core/machine/machine_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

type_status core_machine_debug_read_cpu(
    const core_machine *machine,
    core_machine_cpu_state *out_state);
type_status core_machine_debug_read_memory(
    const core_machine *machine,
    uint32_t physical,
    C_VOID *out_data,
    STD_SIZE_T size);
type_status core_machine_debug_step(
    core_machine *machine,
    core_machine_run_result *out_result);
type_status core_machine_debug_continue(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *out_result);

/* These operations are deliberately debugger-shaped: each is available only
 * at a stopped or paused command boundary and never exposes core storage. */
typedef struct core_machine_debug_instruction_observation {
    t_cpu cpu;
    t_cpuins instructions;
} core_machine_debug_instruction_observation;

typedef enum core_machine_debug_register {
    CORE_MACHINE_DEBUG_EAX, CORE_MACHINE_DEBUG_ECX, CORE_MACHINE_DEBUG_EDX,
    CORE_MACHINE_DEBUG_EBX, CORE_MACHINE_DEBUG_ESP, CORE_MACHINE_DEBUG_EBP,
    CORE_MACHINE_DEBUG_ESI, CORE_MACHINE_DEBUG_EDI, CORE_MACHINE_DEBUG_EIP,
    CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_ES, CORE_MACHINE_DEBUG_CS,
    CORE_MACHINE_DEBUG_SS, CORE_MACHINE_DEBUG_DS, CORE_MACHINE_DEBUG_FS,
    CORE_MACHINE_DEBUG_GS, CORE_MACHINE_DEBUG_CR0, CORE_MACHINE_DEBUG_CR1,
    CORE_MACHINE_DEBUG_CR2, CORE_MACHINE_DEBUG_CR3, CORE_MACHINE_DEBUG_CR4,
    CORE_MACHINE_DEBUG_REGISTER_COUNT
} core_machine_debug_register;

#define CORE_MACHINE_DEBUG_REGISTER_MASK(register_id) \
    (1u << (register_id))

/* A patch names precisely the fields it may alter. Core validates the complete
 * requested set against a candidate CPU and commits it only on success. */
typedef struct core_machine_debug_register_patch {
    uint32_t mask;
    uint32_t values[CORE_MACHINE_DEBUG_REGISTER_COUNT];
} core_machine_debug_register_patch;

typedef enum core_machine_debug_watch_kind {
    CORE_MACHINE_DEBUG_WATCH_READ,
    CORE_MACHINE_DEBUG_WATCH_WRITE,
    CORE_MACHINE_DEBUG_WATCH_EXECUTE
} core_machine_debug_watch_kind;

type_status core_machine_debug_capture_instruction_observation(
    const core_machine *machine,
    core_machine_debug_instruction_observation *out_observation);
type_status core_machine_debug_read_register(
    const core_machine *machine, core_machine_debug_register register_id,
    uint32_t *out_value);
type_status core_machine_debug_write_register(
    core_machine *machine, core_machine_debug_register register_id,
    uint32_t value);
type_status core_machine_debug_patch_registers(core_machine *machine,
    const core_machine_debug_register_patch *patch);
type_status core_machine_debug_get_code_default_size(
    const core_machine *machine, C_INT *out_default_size);
type_status core_machine_debug_get_code_base(
    const core_machine *machine, uint32_t *out_base);
type_status core_machine_debug_read_linear(core_machine *machine,
    uint32_t address, C_VOID *out_data, uint8_t size);
type_status core_machine_debug_write_linear(core_machine *machine,
    uint32_t address, const C_VOID *data, uint8_t size);
type_status core_machine_debug_read_real(core_machine *machine, uint16_t segment,
    uint16_t offset, C_VOID *out_data, STD_SIZE_T size);
type_status core_machine_debug_write_real(core_machine *machine, uint16_t segment,
    uint16_t offset, const C_VOID *data, STD_SIZE_T size);
type_status core_machine_debug_read_port(core_machine *machine, uint16_t port,
    uint32_t *out_value);
type_status core_machine_debug_write_port(core_machine *machine, uint16_t port,
    uint32_t value);
type_status core_machine_debug_set_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind, uint32_t address);
type_status core_machine_debug_clear_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind);
type_status core_machine_debug_print_registers(core_machine *machine);
type_status core_machine_debug_print_segment_registers(core_machine *machine);
type_status core_machine_debug_print_control_registers(core_machine *machine);
type_status core_machine_debug_print_memory_accesses(core_machine *machine);
type_status core_machine_debug_print_watchpoints(core_machine *machine);

/* Transitional composition-debug adapter access. These borrows are valid
 * only after the execution thread has returned a paused boundary. */
t_cpu *core_machine_debug_cpu_borrow(core_machine *machine);
t_cpuins *core_machine_debug_cpu_instructions_borrow(core_machine *machine);
core_machine_cpu_execution_context *core_machine_debug_cpu_execution_borrow(
    core_machine *machine);
t_ram *core_machine_debug_memory_borrow(core_machine *machine);
t_port *core_machine_debug_port_borrow(core_machine *machine);

#ifdef __cplusplus
}
#endif

#endif
