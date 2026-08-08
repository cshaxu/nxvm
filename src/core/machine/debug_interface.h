#ifndef CORE_MACHINE_DEBUG_INTERFACE_H
#define CORE_MACHINE_DEBUG_INTERFACE_H

#include "type.h"



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

#define CORE_MACHINE_DEBUG_INSTRUCTION_BYTES 15u
#define CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY 32u
#define CORE_MACHINE_DEBUG_EFLAGS_CF 0x00000001u
#define CORE_MACHINE_DEBUG_EFLAGS_PF 0x00000004u
#define CORE_MACHINE_DEBUG_EFLAGS_AF 0x00000010u
#define CORE_MACHINE_DEBUG_EFLAGS_ZF 0x00000040u
#define CORE_MACHINE_DEBUG_EFLAGS_SF 0x00000080u
#define CORE_MACHINE_DEBUG_EFLAGS_TF 0x00000100u
#define CORE_MACHINE_DEBUG_EFLAGS_IF 0x00000200u
#define CORE_MACHINE_DEBUG_EFLAGS_DF 0x00000400u
#define CORE_MACHINE_DEBUG_EFLAGS_OF 0x00000800u
#define CORE_MACHINE_DEBUG_EFLAGS_NT 0x00004000u
#define CORE_MACHINE_DEBUG_EFLAGS_RF 0x00010000u
#define CORE_MACHINE_DEBUG_EFLAGS_VM 0x00020000u

typedef struct core_machine_debug_memory_access {
    C_INT write;
    uint32_t linear;
    uint8_t bytes;
    uint64_t data;
} core_machine_debug_memory_access;

/* A copied debugger record names only the fields consumed by the retained
 * debugger. It is not a CPU, decoder, or executor layout. */
typedef struct core_machine_debug_instruction_observation {
    uint16_t cs;
    uint16_t ss;
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
    uint32_t cs_base;
    uint32_t ss_base;
    uint32_t eip;
    uint32_t esp;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
    C_INT code_default_size;
    uint16_t instruction_cs;
    uint32_t instruction_eip;
    uint32_t instruction_linear;
    uint8_t instruction_bytes[CORE_MACHINE_DEBUG_INSTRUCTION_BYTES];
    uint8_t instruction_byte_count;
    core_machine_debug_memory_access
        memory_accesses[CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY];
    uint8_t memory_access_count;
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

#ifdef __cplusplus
}
#endif

#endif
