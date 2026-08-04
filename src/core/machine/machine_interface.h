#ifndef CORE_MACHINE_INTERFACE_H
#define CORE_MACHINE_INTERFACE_H


#include "core/machine/cpu_interface.h"
#include "core/machine/fpu_interface.h"
#include "core/machine/execution_provider.h"
#include "core/machine/firmware_interrupt_interface.h"
#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/dma.h"
#include "core/machine/kbc.h"
#include "core/machine/lifecycle_interface.h"
#include "core/machine/memory.h"
#include "core/machine/memory_interface.h"
#include "core/machine/pic.h"
#include "core/machine/pit.h"
#include "core/machine/port.h"
#include "core/machine/port_interface.h"
#include "type.h"
#include "core/machine/trace_interface.h"
#include "core/machine/vadp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef struct core_machine_profile_binding {
    core_machine *machine;
} core_machine_profile_binding;

#define CORE_MACHINE_DEFAULT_MEMORY_BYTES (16u * 1024u * 1024u)
#define CORE_MACHINE_MINIMUM_MEMORY_BYTES (2u * 1024u * 1024u)
#define CORE_MACHINE_MAXIMUM_MEMORY_BYTES (64u * 1024u * 1024u)

typedef struct core_machine_config {
    STD_SIZE_T memory_bytes;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
} core_machine_config;

typedef enum core_machine_stop_reason {
    CORE_MACHINE_STOP_NONE = 0,
    CORE_MACHINE_STOP_BUDGET,
    CORE_MACHINE_STOP_PAUSED,
    CORE_MACHINE_STOP_GUEST_EXIT,
    CORE_MACHINE_STOP_REQUESTED,
    CORE_MACHINE_STOP_RESET_REQUESTED,
    CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT,
    CORE_MACHINE_STOP_FAULT
} core_machine_stop_reason;

typedef struct core_machine_run_budget {
    uint64_t instructions;
    uint32_t ticks;
} core_machine_run_budget;

typedef struct core_machine_run_result {
    core_machine_stop_reason reason;
    uint64_t executed;
    uint32_t linear_pc;
    uint32_t detail;
} core_machine_run_result;

typedef struct core_machine_observation {
    core_machine_lifecycle lifecycle;
    core_machine_cpu_state cpu;
    core_machine_cpu_diagnostic diagnostic;
} core_machine_observation;

type_status core_machine_create(
    const core_machine_config *config,
    core_machine **out_machine);

type_status core_machine_reset(core_machine *machine);

type_status core_machine_reconfigure_memory(core_machine *machine,
    STD_SIZE_T memory_bytes);

type_status core_machine_get_lifecycle(
    const core_machine *machine,
    core_machine_lifecycle *out_lifecycle);

type_status core_machine_get_cpu_state(
    const core_machine *machine,
    core_machine_cpu_state *out_state);

type_status core_machine_get_cpu_profile(
    const core_machine *machine, core_machine_cpu_profile *out_profile);
type_status core_machine_get_fpu_profile(
    const core_machine *machine, core_machine_fpu_profile *out_profile);
type_status core_machine_get_memory_bytes(
    const core_machine *machine, STD_SIZE_T *out_memory_bytes);

type_status core_machine_get_cpu_diagnostic(
    const core_machine *machine,
    core_machine_cpu_diagnostic *out_diagnostic);

type_status core_machine_run(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *result);

type_status core_machine_request_stop(core_machine *machine);

type_status core_machine_keyboard_submit_scan_code(core_machine *machine,
    uint8_t scan_code);

type_status core_machine_capture_display_snapshot(const core_machine *machine,
    core_machine_display_snapshot *out_snapshot);

type_status core_machine_report_fault(
    core_machine *machine,
    uint32_t detail);

/* Composition may borrow these core-owned objects only while INITIALIZED to
 * bind providers/profile firmware. A registered provider may retain its child
 * reference through core-controlled teardown; no product command may borrow. */
t_cpu *core_machine_configuration_cpu_borrow(core_machine *machine);
t_cpuins *core_machine_configuration_cpu_instructions_borrow(core_machine *machine);
core_machine_cpu_execution_context *core_machine_configuration_cpu_execution_borrow(
    core_machine *machine);
t_ram *core_machine_configuration_memory_borrow(core_machine *machine);
t_port *core_machine_configuration_port_borrow(core_machine *machine);
t_pic *core_machine_configuration_shared_pic_master_borrow(core_machine *machine);
t_pic *core_machine_configuration_shared_pic_slave_borrow(core_machine *machine);
t_pit *core_machine_configuration_shared_pit_borrow(core_machine *machine);
t_latch *core_machine_configuration_shared_dma_latch_borrow(core_machine *machine);
t_dma *core_machine_configuration_shared_dma_primary_borrow(core_machine *machine);
t_dma *core_machine_configuration_shared_dma_secondary_borrow(core_machine *machine);
t_kbc *core_machine_configuration_shared_kbc_borrow(core_machine *machine);

type_status core_machine_profile_binding_initialize(
    core_machine *machine, core_machine_profile_binding *binding);
t_ram *core_machine_profile_binding_memory(
    const core_machine_profile_binding *binding);
core_machine_cpu_execution_context *core_machine_profile_binding_execution(
    const core_machine_profile_binding *binding);
type_status core_machine_profile_binding_configure_text_video(
    const core_machine_profile_binding *binding, uint8_t mode, uint16_t columns,
    uint16_t rows, C_INT color_enabled);
C_VOID core_machine_profile_binding_set_video_cursor_shape(
    const core_machine_profile_binding *binding, uint8_t top, uint8_t bottom);
C_VOID core_machine_profile_binding_set_video_cursor_address(
    const core_machine_profile_binding *binding, uint16_t address);
C_VOID core_machine_profile_binding_set_video_display_start(
    const core_machine_profile_binding *binding, uint16_t address);

type_status core_machine_capture_observation(
    const core_machine *machine, core_machine_observation *out_observation);

type_status core_machine_bind_execution_provider(core_machine *machine,
    const core_machine_execution_provider *provider, C_VOID *context);
type_status core_machine_install_firmware_interrupt_portal(core_machine *machine,
    const core_machine_firmware_interrupt_portal *portal);
type_status core_machine_freeze_execution_providers(core_machine *machine);

C_VOID core_machine_destroy(core_machine *machine);

#ifdef __cplusplus
}
#endif

#endif
