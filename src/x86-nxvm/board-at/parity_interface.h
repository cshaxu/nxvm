#ifndef X86_NXVM_BOARD_AT_PARITY_INTERFACE_H
#define X86_NXVM_BOARD_AT_PARITY_INTERFACE_H
#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/chips/pit825x/pit825x_interface.h"

typedef struct x86_nxvm_board_at_parity x86_nxvm_board_at_parity;
typedef enum x86_nxvm_board_planar_parity_refresh_status_source {
    /* Port B reflects the directly wired PIT counter 1 output. */
    X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1 = 0,
    /* Frozen board period on Core's sole tick axis; not a second clock. */
    X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE
} x86_nxvm_board_planar_parity_refresh_status_source;

typedef struct x86_nxvm_board_planar_parity_config {
    /* Zero memory_bytes selects port/timer/speaker wiring only. */
    lib_u16 port;
    lib_size memory_bytes;
    x86_nxvm_board_planar_parity_refresh_status_source refresh_status_source;
    /* Required only for ELAPSED_TICK_TOGGLE. */
    lib_u32 refresh_status_toggle_ticks;
} x86_nxvm_board_planar_parity_config;

typedef struct x86_nxvm_board_planar_parity_observation {
    lib_i32 configured;
    lib_i32 enabled;
    lib_i32 latched;
    lib_i32 nmi_signaled;
} x86_nxvm_board_planar_parity_observation;

typedef void (*x86_nxvm_board_at_speaker_lines)(void *context, lib_u8 value);

/* Serialized AT port-B/parity owner. Core, PIT and the electrical speaker
 * sink are borrowed for this lifetime. Creation atomically admits the port
 * and parity producer; failure revokes this candidate's routes. Destroy
 * after dispatch stops, before destroying the borrowed Core/PIT. */
lib_status x86_nxvm_board_at_parity_create(x86_core_machine *machine, x86_pit *pit,
    const x86_nxvm_board_planar_parity_config *config,
    x86_nxvm_board_at_speaker_lines speaker, void *speaker_context,
    x86_nxvm_board_at_parity **out_parity);
void x86_nxvm_board_at_parity_destroy(x86_nxvm_board_at_parity *parity);
void x86_nxvm_board_at_parity_reset(x86_nxvm_board_at_parity *parity);
void x86_nxvm_board_at_parity_refresh_nmi(x86_nxvm_board_at_parity *parity);
lib_status x86_nxvm_board_at_parity_report_fault(x86_nxvm_board_at_parity *parity);
void x86_nxvm_board_at_parity_observe(const x86_nxvm_board_at_parity *parity,
    x86_nxvm_board_planar_parity_observation *out_observation);
#endif
