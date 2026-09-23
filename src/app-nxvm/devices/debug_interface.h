#ifndef CORE_MACHINE_DEBUG_INTERFACE_H
#define CORE_MACHINE_DEBUG_INTERFACE_H
#include "lib/types/types_interface.h"

#include "type.h"



#include "app-nxvm/devices/machine_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

type_status core_machine_debug_read_cpu(
    const core_machine *machine,
    core_machine_cpu_state *out_state);
type_status core_machine_debug_read_memory(
    const core_machine *machine,
    lib_u32 physical,
    C_VOID *out_data,
    lib_size size);
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

typedef enum core_machine_debug_watch_kind {
    CORE_MACHINE_DEBUG_WATCH_READ,
    CORE_MACHINE_DEBUG_WATCH_WRITE,
    CORE_MACHINE_DEBUG_WATCH_EXECUTE
} core_machine_debug_watch_kind;

typedef struct core_machine_debug_memory_access {
    C_INT write;
    lib_u32 linear;
    lib_u8 bytes;
    lib_u64 data;
} core_machine_debug_memory_access;

typedef struct core_machine_debug_segment_snapshot {
    lib_u16 selector;
    lib_u32 base;
    lib_u32 limit;
    lib_u8 dpl;
    lib_u8 type;
    type_bool accessed;
    type_bool executable;
    type_bool conform;
    type_bool readable;
    type_bool defsize;
    type_bool big;
    type_bool expdown;
    type_bool writable;
} core_machine_debug_segment_snapshot;

typedef struct core_machine_debug_cpu_snapshot {
    core_machine_debug_segment_snapshot es, cs, ss, ds, fs, gs;
    core_machine_debug_segment_snapshot tr, ldtr, gdtr, idtr;
    lib_u32 cr0, cr2, cr3;
} core_machine_debug_cpu_snapshot;

/* A copied debugger record names only the fields consumed by the retained
 * debugger. It is not a CPU, decoder, or executor layout. */
typedef struct core_machine_debug_instruction_observation {
    lib_u16 cs;
    lib_u16 ss;
    lib_u16 ds;
    lib_u16 es;
    lib_u16 fs;
    lib_u16 gs;
    lib_u32 cs_base;
    lib_u32 ss_base;
    lib_u32 eip;
    lib_u32 esp;
    lib_u32 eax;
    lib_u32 ecx;
    lib_u32 edx;
    lib_u32 ebx;
    lib_u32 ebp;
    lib_u32 esi;
    lib_u32 edi;
    lib_u32 eflags;
    C_INT code_default_size;
    lib_u16 instruction_cs;
    lib_u32 instruction_eip;
    lib_u32 instruction_linear;
    lib_u8 instruction_bytes[CORE_MACHINE_DEBUG_INSTRUCTION_BYTES];
    lib_u8 instruction_byte_count;
    core_machine_debug_memory_access
        memory_accesses[CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY];
    lib_u8 memory_access_count;
    type_bool watch_hit;
    core_machine_debug_watch_kind watch_kind;
    lib_u32 watch_address;
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
    lib_u32 mask;
    lib_u32 values[CORE_MACHINE_DEBUG_REGISTER_COUNT];
} core_machine_debug_register_patch;

type_status core_machine_debug_capture_instruction_observation(
    const core_machine *machine,
    core_machine_debug_instruction_observation *out_observation);
type_status core_machine_debug_capture_cpu_snapshot(const core_machine *machine,
    core_machine_debug_cpu_snapshot *out_snapshot);
type_status core_machine_debug_read_register(
    const core_machine *machine, core_machine_debug_register register_id,
    lib_u32 *out_value);
type_status core_machine_debug_write_register(
    core_machine *machine, core_machine_debug_register register_id,
    lib_u32 value);
type_status core_machine_debug_patch_registers(core_machine *machine,
    const core_machine_debug_register_patch *patch);
type_status core_machine_debug_get_code_default_size(
    const core_machine *machine, C_INT *out_default_size);
type_status core_machine_debug_get_code_base(
    const core_machine *machine, lib_u32 *out_base);
type_status core_machine_debug_read_linear(core_machine *machine,
    lib_u32 address, C_VOID *out_data, lib_u8 size);
type_status core_machine_debug_write_linear(core_machine *machine,
    lib_u32 address, const C_VOID *data, lib_u8 size);
type_status core_machine_debug_read_real(core_machine *machine, lib_u16 segment,
    lib_u16 offset, C_VOID *out_data, lib_size size);
type_status core_machine_debug_write_real(core_machine *machine, lib_u16 segment,
    lib_u16 offset, const C_VOID *data, lib_size size);
type_status core_machine_debug_read_port(core_machine *machine, lib_u16 port,
    lib_u32 *out_value);
type_status core_machine_debug_write_port(core_machine *machine, lib_u16 port,
    lib_u32 value);
type_status core_machine_debug_set_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind, lib_u32 address);
type_status core_machine_debug_clear_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind);
type_status core_machine_debug_get_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind, type_bool *out_enabled,
    lib_u32 *out_address);

#ifdef __cplusplus
}
#endif

#endif
