#ifndef LIB_HOST_CONSOLE_INTERFACE_H
#define LIB_HOST_CONSOLE_INTERFACE_H

#include "lib/console/console_interface.h"

typedef struct host_console_broker host_console_broker;

typedef enum host_console_mode {
    HOST_CONSOLE_RAW_EVENTS,
    HOST_CONSOLE_COOKED_LINES
} host_console_mode;

/* The broker owns one process-native Console I/O path. The caller owns every
 * logical Console and remains the product-state owner. */
/* Failure leaves *out_broker NULL.  An activation that cannot retire its
 * native reader is terminal at the application boundary, not a half-broker
 * protocol for ordinary callers. */
lib_status host_console_broker_create(host_console_broker **out_broker,
    lib_console *initial_console, host_console_mode initial_mode);
/* Replaces Current Console only when expected_current is still current. This
 * is a transaction assertion, not a host-side product-state query. */
lib_status host_console_broker_replace(host_console_broker *broker,
    lib_console *expected_current, lib_console *next_console,
    host_console_mode next_mode);
/* Idempotently arms one cooked line only while expected_current remains the
 * Current Console in cooked mode. */
lib_status host_console_broker_request_cooked_line(host_console_broker *broker,
    lib_console *expected_current);
/* Quiesce the current cooked read without replacing its binding. Success
 * discards an unfinished line; *out_completed is true when no unfinished read
 * remains to cancel (including repeated cancellation after completion).
 * Call with an outstanding requested line: a completed event must be consumed.
 * No prompt/output policy is applied. Failure is terminal for this broker. */
lib_status host_console_broker_cancel_cooked_line(host_console_broker *broker,
    lib_console *expected_current, lib_bool *out_completed);
/* OK consumes the object. Failure is a terminal infrastructure failure. */
lib_status host_console_broker_destroy(host_console_broker *broker);

#endif
