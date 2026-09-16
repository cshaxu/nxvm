#ifndef LIB_CONSOLE_BROKER_INTERFACE_H
#define LIB_CONSOLE_BROKER_INTERFACE_H

#include "lib/console/console_interface.h"

typedef struct console_broker console_broker;

typedef enum console_broker_mode {
    CONSOLE_BROKER_RAW_EVENTS,
    CONSOLE_BROKER_COOKED_LINES
} console_broker_mode;

/* The broker owns one process-native Console I/O path. The caller owns every
 * logical Console and remains the product-state owner. */
/* Failure leaves *out_broker NULL.  An activation that cannot retire its
 * native reader is terminal at the application boundary, not a half-broker
 * protocol for ordinary callers. */
lib_status console_broker_create(console_broker **out_broker,
    lib_console *initial_console, console_broker_mode initial_mode);
/* Replaces Current Console only when expected_current is still current. This
 * is a transaction assertion, not a host-side product-state query. */
lib_status console_broker_replace(console_broker *broker,
    lib_console *expected_current, lib_console *next_console,
    console_broker_mode next_mode);
/* Idempotently arms one cooked line only while expected_current remains the
 * Current Console in cooked mode. */
lib_status console_broker_request_cooked_line(console_broker *broker,
    lib_console *expected_current);
/* Quiesce the current cooked read without replacing its binding. Success
 * discards an unfinished line; *out_completed is true when no unfinished read
 * remains to cancel (including repeated cancellation after completion).
 * Call with an outstanding requested line: a completed event must be consumed.
 * No prompt/output policy is applied. Failure is terminal for this broker. */
lib_status console_broker_cancel_cooked_line(console_broker *broker,
    lib_console *expected_current, lib_bool *out_completed);
/* OK consumes the object. Failure is a terminal infrastructure failure. */
lib_status console_broker_destroy(console_broker *broker);

#endif
