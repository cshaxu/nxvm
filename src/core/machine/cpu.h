#ifndef NXVM_CORE_CPU_H
#define NXVM_CORE_CPU_H

#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_core_machine nxvm_core_machine;

typedef struct nxvm_core_cpu_state {
    uint16_t cs;
    uint32_t cs_base;
    uint32_t eip;
    uint32_t eflags;
    uint8_t halted;
} nxvm_core_cpu_state;

typedef enum core_machine_cpu_segment {
    CORE_MACHINE_CPU_SEGMENT_ES,
    CORE_MACHINE_CPU_SEGMENT_CS,
    CORE_MACHINE_CPU_SEGMENT_SS,
    CORE_MACHINE_CPU_SEGMENT_DS,
    CORE_MACHINE_CPU_SEGMENT_FS,
    CORE_MACHINE_CPU_SEGMENT_GS
} core_machine_cpu_segment;

typedef enum core_machine_cpu_watchpoint {
    CORE_MACHINE_CPU_WATCH_READ,
    CORE_MACHINE_CPU_WATCH_WRITE,
    CORE_MACHINE_CPU_WATCH_EXECUTE
} core_machine_cpu_watchpoint;

nxvm_core_status nxvm_core_machine_get_cpu_state(
    const nxvm_core_machine *machine,
    nxvm_core_cpu_state *out_state);

int core_machine_cpu_read_linear(uint32_t linear, void *out_data, uint8_t size);
int core_machine_cpu_write_linear(uint32_t linear, const void *in_data,
    uint8_t size);
int core_machine_cpu_load_segment(core_machine_cpu_segment segment,
    uint16_t selector);
int core_machine_cpu_get_code_default_size(void);
uint32_t core_machine_cpu_get_code_base(void);
void core_machine_cpu_set_watchpoint(core_machine_cpu_watchpoint kind,
    uint32_t linear);
void core_machine_cpu_clear_watchpoint(core_machine_cpu_watchpoint kind);
void core_machine_cpu_print_registers(void);
void core_machine_cpu_print_segment_registers(void);
void core_machine_cpu_print_control_registers(void);
void core_machine_cpu_print_memory_accesses(void);
void core_machine_cpu_print_watchpoints(void);

#ifdef __cplusplus
}
#endif

#endif
