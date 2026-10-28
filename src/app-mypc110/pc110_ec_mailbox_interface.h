#ifndef MYPC110_EC_MAILBOX_INTERFACE_H
#define MYPC110_EC_MAILBOX_INTERFACE_H

#include "x86-core/machine/machine_interface.h"

#define MYPC110_EC_RAM_BYTES 512u

/* L2 functional host-mailbox transactions, not an executing MCU. Commands
 * 05/10/21/FC return defined controller data; D0--D2 read its 40h--23Fh RAM.
 * D4/D5 write page 1 and D6 writes page 2 in the selected controller image.
 * One reply latch owns OBF; writes complete immediately (IBF clear). GPIO,
 * serial charging traffic and unsolicited input remain separate receivers.
 * Construction initializes retained EC RAM; a CPU reset does not clear it. */
typedef struct mypc110_ec_mailbox {
    lib_u8 command;
    lib_u8 data;
    lib_u16 address;
    lib_u8 parameter_count;
    lib_u8 reply;
    lib_bool reply_pending;
    lib_bool control_pending;
    lib_bool control_timing_started;
    lib_u64 control_start_tick;
    lib_u8 ram[MYPC110_EC_RAM_BYTES];
} mypc110_ec_mailbox;

lib_status mypc110_ec_mailbox_install_routes(x86_core_machine *machine,
    mypc110_ec_mailbox *mailbox);

#endif
