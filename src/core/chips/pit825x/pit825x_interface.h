/* Copyright 2012-2014 Neko. */
#ifndef X86_PIT825X_INTERFACE_H
#define X86_PIT825X_INTERFACE_H
#include "lib/types/types_interface.h"

typedef struct x86_pit x86_pit;
typedef enum x86_pit_personality {
    X86_PIT_PERSONALITY_8254 = 0,
    X86_PIT_PERSONALITY_8253
} x86_pit_personality;
typedef void (*x86_pit_output_provider)(void *context, lib_bool asserted);

/* One execution owner; no concurrent calls. Output contexts are borrowed and
 * must outlive reset/destroy, which release asserted outputs. Callbacks may
 * deliver signals but must not run or destroy this timer recursively. */
lib_status x86_pit_create(x86_pit_personality personality, x86_pit **out_pit);
void x86_pit_destroy(x86_pit *pit);
void x86_pit_reset(x86_pit *pit);
/* Counter read consumes its latch. An unprogrammed counter leaves the input
 * bus byte unchanged. Counter selectors are 0..2; write selectors are 0..3. */
lib_status x86_pit_read_counter(x86_pit *pit, lib_u8 counter,
    lib_u8 *inout_value);
lib_status x86_pit_write_register(x86_pit *pit, lib_u8 selector, lib_u8 value);
/* Binding replaces the sink without changing GATE or emitting a synthetic edge. */
void x86_pit_set_output(x86_pit *pit, lib_u8 counter,
    x86_pit_output_provider provider, void *context);
void x86_pit_set_gate(x86_pit *pit, lib_u8 counter, lib_bool asserted);
lib_bool x86_pit_get_output(const x86_pit *pit, lib_u8 counter);
/* Input-clock cycles, not host time. All intermediate edges are delivered.
 * INVALID_STATE from the observation means no scheduled output change;
 * OK returns a positive cycle distance. Invalid arguments return separately. */
void x86_pit_advance(x86_pit *pit, lib_u64 cycles);
lib_status x86_pit_ticks_until_output(const x86_pit *pit, lib_u8 counter,
    lib_u64 *out_cycles);
#endif
