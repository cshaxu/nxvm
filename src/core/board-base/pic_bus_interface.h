#ifndef CORE_MACHINE_PIC_INTERFACE_H
#define CORE_MACHINE_PIC_INTERFACE_H
#include "lib/types/types_interface.h"
#include "core/x86/port_interface.h"
#include "core/chips/pic8259/pic8259_interface.h"

typedef struct core_machine_pic_bus core_machine_pic_bus;
typedef struct core_machine_pic_irq_source core_machine_pic_irq_source;

/* Zero preserves the PC/AT cascaded pair; a selected single-PIC board omits
 * the slave's guest-visible port decode while retaining one private Core
 * owner. */
typedef enum core_machine_pic_topology {
    CORE_MACHINE_PIC_TOPOLOGY_CASCADED = 0,
    CORE_MACHINE_PIC_TOPOLOGY_SINGLE = 1
} core_machine_pic_topology;

/* Immutable board timing for already-pending IRQs released by an IMR write.
 * The 8259A remains the sole owner of IRR, IMR and interrupt selection. */
typedef struct core_machine_pic_irq_timing {
    lib_u32 unmask_delivery_ticks[16];
} core_machine_pic_irq_timing;

/* Serialized board construction/runtime access. The pair owns its endpoints
 * and every IRQ source lease; callers borrow handles until finalize. Core
 * copies the routes atomically. Stop dispatch before finalizing the pair;
 * Core attachment teardown may finalize immediately before discarding routes.
 * Failed construction publishes neither endpoint. Rebinding an existing source
 * reuses its lease, and requires the same pair and a quiescent/reset source. */
lib_status core_machine_pic_initialize(core_machine_pic_bus **out_master,
    core_machine_pic_bus **out_slave, core_machine *machine,
    core_machine_pic_topology topology);
void core_machine_pic_finalize(core_machine_pic_bus *master, core_machine_pic_bus *slave);
void core_machine_pic_reset(core_machine_pic_bus *master, core_machine_pic_bus *slave);
void core_machine_pic_refresh(core_machine_pic_bus *master, core_machine_pic_bus *slave);
void core_machine_pic_set_irq_timing(core_machine_pic_bus *master,
    core_machine_pic_bus *slave, const core_machine_pic_irq_timing *timing);
void core_machine_pic_advance(core_machine_pic_bus *master,
    core_machine_pic_bus *slave, lib_u64 elapsed_ticks);
lib_status core_machine_pic_ticks_until_event(const core_machine_pic_bus *master,
    const core_machine_pic_bus *slave, lib_u64 *out_ticks);
lib_status core_machine_pic_irq_source_bind(core_machine_pic_irq_source **out_source,
    core_machine_pic_bus *master, core_machine_pic_bus *slave, lib_u8 irq);
void core_machine_pic_irq_source_assert(core_machine_pic_irq_source *source);
void core_machine_pic_irq_source_deassert(core_machine_pic_irq_source *source);
lib_bool core_machine_pic_irq_source_is_asserted(const core_machine_pic_irq_source *source);
void core_machine_pic_timer_output(void *owner, lib_u8 asserted);
lib_u8 core_machine_pic_scan_interrupt(core_machine_pic_bus *master, core_machine_pic_bus *slave);
lib_u8 core_machine_pic_peek_interrupt(core_machine_pic_bus *master, core_machine_pic_bus *slave);
lib_u8 core_machine_pic_get_interrupt(core_machine_pic_bus *master, core_machine_pic_bus *slave);

/* Register cycles share the port adapter's programming/refresh path. These
 * are guest-visible operations, not a chip/layout getter. Reads may poll/ack. */
lib_status core_machine_pic_read_register(core_machine_pic_bus *bus,
    lib_u8 selector, lib_u8 *out_value);
lib_status core_machine_pic_capture_registers(const core_machine_pic_bus *bus,
    x86_pic_register_state *out_state);
lib_status core_machine_pic_write_register(core_machine_pic_bus *bus,
    lib_u8 selector, lib_u8 value);

#endif
