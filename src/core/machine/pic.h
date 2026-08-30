/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_PIC_H
#define CORE_MACHINE_PIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/pic_interface.h"
#include "core/machine/port.h"

#define CORE_MACHINE_DEVICE_PIC "Intel 8259A"

typedef enum {ICW1, ICW2, ICW3, ICW4, OCW1} t_pic_init_status;

#define VPIC_MAX_IRQ_COUNT 8
typedef struct {
    type_unsigned_8 irr;  /* Interrupt Request Register */
    type_unsigned_8 imr;  /* Interrupt Mask Register */
    type_unsigned_8 isr;  /* In Service Register */
    type_unsigned_8 icw1, icw2, icw3, icw4, ocw2, ocw3; /* command words */
    t_pic_init_status status; /* initialization status */
    type_unsigned_8 irx; /* id of current top potential ir */
    type_unsigned_8 asserted[VPIC_MAX_IRQ_COUNT]; /* source levels */
    type_unsigned_8 cascade_irr; /* paired slave's synthesized request */
} t_pic_data;

typedef struct t_pic {
    t_pic_data data;
    /* Construction-fixed pair for immediate refresh of the one cascade state. */
    struct t_pic *cascade_master;
    struct t_pic *cascade_slave;
} t_pic;

typedef struct core_machine_pic_irq_source {
    t_pic *master;
    t_pic *slave;
    type_unsigned_8 irq;
    type_bool asserted;
} core_machine_pic_irq_source;

/*
 * IRR: IRQ7 | IRQ6 | IRQ5 | IRQ4 | IRQ3 | IRQ2 | IRQ1 | IRQ0
 * IMR: IRQ7 | IRQ6 | IRQ5 | IRQ4 | IRQ3 | IRQ2 | IRQ1 | IRQ0
 * ISR: IRQ7 | IRQ6 | IRQ5 | IRQ4 | IRQ3 | IRQ2 | IRQ1 | IRQ0
 */

/* interrupt request register bits */
#define VPIC_IRR_IRQ(id) (1 << (id))

/* interrupt mask register bits */
#define VPIC_IMR_IRQ(id) (1 << (id))

/* in-service request register bits */
#define VPIC_ISR_IRQ(id) (1 << (id))

/*
 * ICW1: 0  | 0    | 0   | I    | LTIM | x   | SNGL | IC4
 * ICW2: T7 | T6   | T5  | T4   | T3   | x   | x    | x
 * ICW3: S7 | S6   | S5  | S4   | S3   | S2  | S1   | S0   (master)
 * ICW3: x  | x    | x   | x    | x    | ID2 | ID1  | ID0  (slave)
 * ICW4: 0  | 0    | 0   | SFNM | BUF  | M/S | AEOI | uPM
 * OCW1: M7 | M6   | M5  | M4   | M3   | M2  | M1   | M0
 * OCW2: R  | SL   | EOI | 0    | 0    | L2  | L1   | L0
 * OCW3: 0  | ESMM | SMM | 0    | I    | P   | RR   | RIS
 * POLL: I  | x    | x   | x    | x    | W2  | W1   | W0
 */

/* ICW1 bits */
#define VPIC_ICW1_IC4  0x01 /* ICW4 is needed */
#define VPIC_ICW1_SNGL 0x02 /* single(1) 8259 or cascading(0) 8259's */
#define VPIC_ICW1_LTIM 0x08 /* level(1) or edge(0) triggered mode */
#define VPIC_ICW1_I    0x10 /* must be 1 for icw1 */

/* ICW2 bits */
#define VPIC_ICW2_VALID 0xf8 /* A7-A3 of x86 interrupt vector */

/* ICW3 master bits */
#define VPIC_ICW3_S(id) (1 << (id)) /* C_INT req id has slave (1) or not(0) */

/* ICW4 bits */
#define VPIC_ICW4_uPM   0x01 /* uPM */
#define VPIC_ICW4_AEOI  0x02 /* auto end of interrupt */
#define VPIC_ICW4_MS    0x04 /* master(1) or slave(0) */
#define VPIC_ICW4_BUF   0x08 /* buffered(1) or not(0) */
#define VPIC_ICW4_SFNM  0x10 /* special fully nested mode (1) or sequential (0) */
#define VPIC_ICW4_VALID 0x1f

/* OCW1 bits */
#define VPIC_OCW1_IMR(id) (1 << (id)) /* irq id is masked */

/* OCW2 bits */
#define VPIC_OCW2_L   0x07 /* interrupt request level to act upon */
#define VPIC_OCW2_EOI 0x20 /* eoi type */
#define VPIC_OCW2_SL  0x40 /* specific eoi command */
#define VPIC_OCW2_R   0x80 /* rotate priority */
#define VPIC_GetOCW2_L(cocw2) ((cocw2) & VPIC_OCW2_L)

/* OCW3 bits */
#define VPIC_OCW3_RIS  0x01 /* read irr(1) or isr(0) on next read */
#define VPIC_OCW3_RR   0x02 /* act(1) on value of bit 0 or no(0) action if bit 0 set */
#define VPIC_OCW3_P    0x04 /* poll command issued(1) or not(0) */
#define VPIC_OCW3_I    0x08 /* must be 1 for ocw3 */
#define VPIC_OCW3_SMM  0x20 /* set(1) or reset(0) special mask */
#define VPIC_OCW3_ESMM 0x40 /* act(1) on value of bit 5 or no(0) action if bit 5 set */

/* POLL bits */
#define VPIC_POLL_I 0x80 /* must be 1 for poll command */

C_VOID core_machine_pic_initialize(t_pic *master, t_pic *slave, t_port *port,
    core_machine_pic_topology topology);
C_VOID core_machine_pic_reset(t_pic *master, t_pic *slave);
C_VOID core_machine_pic_refresh(t_pic *master, t_pic *slave);
C_VOID core_machine_pic_finalize(t_pic *master, t_pic *slave);
C_VOID core_machine_pic_irq_source_bind(core_machine_pic_irq_source *source,
    t_pic *master, t_pic *slave, type_unsigned_8 irq_id);
C_VOID core_machine_pic_irq_source_assert(core_machine_pic_irq_source *source);
C_VOID core_machine_pic_irq_source_deassert(core_machine_pic_irq_source *source);
C_VOID core_machine_pic_timer_output(C_VOID *owner, type_bool asserted);
type_bool core_machine_pic_scan_interrupt(t_pic *master, t_pic *slave);
type_unsigned_8 core_machine_pic_peek_interrupt(t_pic *master, t_pic *slave);
/* First logical INTA: select the request, transfer it from IRR to ISR, and
 * return the vector reserved for the CPU's following interrupt entry. */
type_unsigned_8 core_machine_pic_get_interrupt(t_pic *master, t_pic *slave);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
