#ifndef X86_NXVM_BOARD_PIC_INTERFACE_H
#define X86_NXVM_BOARD_PIC_INTERFACE_H
#include "lib/types/types_interface.h"
#include "x86-core/machine/port_interface.h"
#include "x86-nxvm/chips/pic8259/pic8259_interface.h"

typedef struct x86_nxvm_board_pic_bus x86_nxvm_board_pic_bus;
typedef struct x86_nxvm_board_pic_irq_source x86_nxvm_board_pic_irq_source;

/* Zero preserves the PC/AT cascaded pair; a selected single-PIC board omits
 * the slave's guest-visible port decode while retaining one private Core
 * owner. */
typedef enum x86_nxvm_board_pic_topology {
    X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED = 0,
    X86_NXVM_BOARD_PIC_TOPOLOGY_SINGLE = 1
} x86_nxvm_board_pic_topology;

/* Immutable board timing for already-pending IRQs released by an IMR write.
 * The 8259A remains the sole owner of IRR, IMR and interrupt selection. */
typedef struct x86_nxvm_board_pic_irq_timing {
    lib_u32 unmask_delivery_ticks[16];
} x86_nxvm_board_pic_irq_timing;

/* Serialized board construction/runtime access. The pair owns its endpoints
 * and every IRQ source lease; callers borrow handles until finalize. Core
 * copies the routes atomically. Stop dispatch before finalizing the pair;
 * Core attachment teardown may finalize immediately before discarding routes.
 * Failed construction publishes neither endpoint. Rebinding an existing source
 * reuses its lease, and requires the same pair and a quiescent/reset source. */
lib_status x86_nxvm_board_pic_initialize(x86_nxvm_board_pic_bus **out_master,
    x86_nxvm_board_pic_bus **out_slave, x86_core_machine *machine,
    x86_nxvm_board_pic_topology topology);
void x86_nxvm_board_pic_finalize(x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);
void x86_nxvm_board_pic_reset(x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);
void x86_nxvm_board_pic_refresh(x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);
void x86_nxvm_board_pic_set_irq_timing(x86_nxvm_board_pic_bus *master,
    x86_nxvm_board_pic_bus *slave, const x86_nxvm_board_pic_irq_timing *timing);
void x86_nxvm_board_pic_advance(x86_nxvm_board_pic_bus *master,
    x86_nxvm_board_pic_bus *slave, lib_u64 elapsed_ticks);
lib_status x86_nxvm_board_pic_ticks_until_event(const x86_nxvm_board_pic_bus *master,
    const x86_nxvm_board_pic_bus *slave, lib_u64 *out_ticks);
lib_status x86_nxvm_board_pic_irq_source_bind(x86_nxvm_board_pic_irq_source **out_source,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave, lib_u8 irq);
void x86_nxvm_board_pic_irq_source_assert(x86_nxvm_board_pic_irq_source *source);
void x86_nxvm_board_pic_irq_source_deassert(x86_nxvm_board_pic_irq_source *source);
lib_bool x86_nxvm_board_pic_irq_source_is_asserted(const x86_nxvm_board_pic_irq_source *source);
void x86_nxvm_board_pic_timer_output(void *owner, lib_u8 asserted);
lib_u8 x86_nxvm_board_pic_scan_interrupt(x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);
lib_u8 x86_nxvm_board_pic_peek_interrupt(x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);
lib_u8 x86_nxvm_board_pic_get_interrupt(x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);

/* Register cycles share the port adapter's programming/refresh path. These
 * are guest-visible operations, not a chip/layout getter. Reads may poll/ack. */
lib_status x86_nxvm_board_pic_read_register(x86_nxvm_board_pic_bus *bus,
    lib_u8 selector, lib_u8 *out_value);
lib_status x86_nxvm_board_pic_capture_registers(const x86_nxvm_board_pic_bus *bus,
    x86_pic_register_state *out_state);
lib_status x86_nxvm_board_pic_write_register(x86_nxvm_board_pic_bus *bus,
    lib_u8 selector, lib_u8 value);

#endif
