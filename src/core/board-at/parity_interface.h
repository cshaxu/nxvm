#ifndef CORE_MACHINE_AT_PARITY_INTERFACE_H
#define CORE_MACHINE_AT_PARITY_INTERFACE_H
#include "core/x86/machine_interface.h"
#include "core/chips/pit825x/pit825x_interface.h"

typedef struct core_machine_at_parity core_machine_at_parity;
typedef enum core_machine_planar_parity_refresh_status_source {
    /* Port B reflects the directly wired PIT counter 1 output. */
    CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1 = 0,
    /* Frozen board period on Core's sole tick axis; not a second clock. */
    CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE
} core_machine_planar_parity_refresh_status_source;

typedef struct core_machine_planar_parity_config {
    /* Zero memory_bytes selects port/timer/speaker wiring only. */
    lib_u16 port;
    lib_size memory_bytes;
    core_machine_planar_parity_refresh_status_source refresh_status_source;
    /* Required only for ELAPSED_TICK_TOGGLE. */
    lib_u32 refresh_status_toggle_ticks;
} core_machine_planar_parity_config;

typedef struct core_machine_planar_parity_observation {
    lib_i32 configured;
    lib_i32 enabled;
    lib_i32 latched;
    lib_i32 nmi_signaled;
} core_machine_planar_parity_observation;

typedef void (*core_machine_at_speaker_lines)(void *context, lib_u8 value);

/* Serialized AT port-B/parity owner. Core, PIT and the electrical speaker
 * sink are borrowed for this lifetime. Creation atomically admits the port
 * and parity producer; failure revokes this candidate's routes. Destroy
 * after dispatch stops, before destroying the borrowed Core/PIT. */
lib_status core_machine_at_parity_create(core_machine *machine, x86_pit *pit,
    const core_machine_planar_parity_config *config,
    core_machine_at_speaker_lines speaker, void *speaker_context,
    core_machine_at_parity **out_parity);
void core_machine_at_parity_destroy(core_machine_at_parity *parity);
void core_machine_at_parity_reset(core_machine_at_parity *parity);
void core_machine_at_parity_refresh_nmi(core_machine_at_parity *parity);
lib_status core_machine_at_parity_report_fault(core_machine_at_parity *parity);
void core_machine_at_parity_observe(const core_machine_at_parity *parity,
    core_machine_planar_parity_observation *out_observation);
#endif
