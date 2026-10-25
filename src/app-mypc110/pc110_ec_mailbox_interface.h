#ifndef MYPC110_EC_MAILBOX_INTERFACE_H
#define MYPC110_EC_MAILBOX_INTERFACE_H

#include "x86-core/machine/machine_interface.h"

/* L2: documented PC110 software polls 15ECh until status bits 0--1 are
 * clear, then sends a command at 15ECh and data at 15E8h.  The documented
 * protocol does not establish a required busy duration or command grammar,
 * so this receiver exposes the proven ready state and records writes without
 * inventing an MCU execution model. */
typedef struct mypc110_ec_mailbox {
    lib_u8 command;
    lib_u8 data;
} mypc110_ec_mailbox;

lib_status mypc110_ec_mailbox_install_routes(x86_core_machine *machine,
    mypc110_ec_mailbox *mailbox);

#endif
