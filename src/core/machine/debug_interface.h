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
    type_unsigned_32 physical,
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
    type_unsigned_32 linear;
    type_unsigned_8 bytes;
    type_unsigned_64 data;
} core_machine_debug_memory_access;

/* A copied debugger record names only the fields consumed by the retained
 * debugger. It is not a CPU, decoder, or executor layout. */
typedef struct core_machine_debug_instruction_observation {
    type_unsigned_16 cs;
    type_unsigned_16 ss;
    type_unsigned_16 ds;
    type_unsigned_16 es;
    type_unsigned_16 fs;
    type_unsigned_16 gs;
    type_unsigned_32 cs_base;
    type_unsigned_32 ss_base;
    type_unsigned_32 eip;
    type_unsigned_32 esp;
    type_unsigned_32 eax;
    type_unsigned_32 ecx;
    type_unsigned_32 edx;
    type_unsigned_32 ebx;
    type_unsigned_32 ebp;
    type_unsigned_32 esi;
    type_unsigned_32 edi;
    type_unsigned_32 eflags;
    C_INT code_default_size;
    type_unsigned_16 instruction_cs;
    type_unsigned_32 instruction_eip;
    type_unsigned_32 instruction_linear;
    type_unsigned_8 instruction_bytes[CORE_MACHINE_DEBUG_INSTRUCTION_BYTES];
    type_unsigned_8 instruction_byte_count;
    core_machine_debug_memory_access
        memory_accesses[CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY];
    type_unsigned_8 memory_access_count;
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
    type_unsigned_32 mask;
    type_unsigned_32 values[CORE_MACHINE_DEBUG_REGISTER_COUNT];
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
    type_unsigned_32 *out_value);
type_status core_machine_debug_write_register(
    core_machine *machine, core_machine_debug_register register_id,
    type_unsigned_32 value);
type_status core_machine_debug_patch_registers(core_machine *machine,
    const core_machine_debug_register_patch *patch);
type_status core_machine_debug_get_code_default_size(
    const core_machine *machine, C_INT *out_default_size);
type_status core_machine_debug_get_code_base(
    const core_machine *machine, type_unsigned_32 *out_base);
type_status core_machine_debug_read_linear(core_machine *machine,
    type_unsigned_32 address, C_VOID *out_data, type_unsigned_8 size);
type_status core_machine_debug_write_linear(core_machine *machine,
    type_unsigned_32 address, const C_VOID *data, type_unsigned_8 size);
type_status core_machine_debug_read_real(core_machine *machine, type_unsigned_16 segment,
    type_unsigned_16 offset, C_VOID *out_data, STD_SIZE_T size);
type_status core_machine_debug_write_real(core_machine *machine, type_unsigned_16 segment,
    type_unsigned_16 offset, const C_VOID *data, STD_SIZE_T size);
type_status core_machine_debug_read_port(core_machine *machine, type_unsigned_16 port,
    type_unsigned_32 *out_value);
type_status core_machine_debug_write_port(core_machine *machine, type_unsigned_16 port,
    type_unsigned_32 value);
type_status core_machine_debug_set_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind, type_unsigned_32 address);
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
