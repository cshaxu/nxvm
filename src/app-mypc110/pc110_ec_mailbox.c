#include "app-mypc110/pc110_ec_mailbox_interface.h"

#define MYPC110_EC_DATA_PORT 0x15e8u
#define MYPC110_EC_COMMAND_STATUS_PORT 0x15ecu
#define MYPC110_EC_RAM_START 0x40u

static void mypc110_ec_mailbox_reply(mypc110_ec_mailbox *mailbox, lib_u8 value)
{
    mailbox->reply = value;
    mailbox->reply_pending = LIB_TRUE;
}

static lib_status mypc110_ec_mailbox_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_ec_mailbox *mailbox = owner;

    (void)tick;
    if (mailbox == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == MYPC110_EC_COMMAND_STATUS_PORT) {
        *out_value = mailbox->reply_pending ? 1u : 0u;
    } else if (port == MYPC110_EC_DATA_PORT) {
        *out_value = mailbox->reply;
        mailbox->reply_pending = LIB_FALSE;
    } else return LIB_STATUS_UNSUPPORTED;
    return LIB_STATUS_OK;
}

static lib_status mypc110_ec_mailbox_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    mypc110_ec_mailbox *mailbox = owner;

    (void)access_bytes;
    if (mailbox == LIB_NULL || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == MYPC110_EC_COMMAND_STATUS_PORT) {
        mailbox->command = (lib_u8)value;
        mailbox->parameter_count = 0u;
        switch (mailbox->command) {
        case 0x05u:
            mypc110_ec_mailbox_reply(mailbox, 0x21u);
            break;
        case 0x21u:
            mypc110_ec_mailbox_reply(mailbox,
                (lib_u8)(mailbox->ram[0xcfu - MYPC110_EC_RAM_START] | 0xf0u));
            break;
        case 0xfcu:
            mypc110_ec_mailbox_reply(mailbox, mailbox->ram[0u]);
            break;
        default: break;
        }
    } else if (port == MYPC110_EC_DATA_PORT) {
        mailbox->data = (lib_u8)value;
        if (mailbox->command >= 0xd4u && mailbox->command <= 0xd6u &&
            mailbox->parameter_count == 1u) {
            if (mailbox->address >= MYPC110_EC_RAM_START &&
                mailbox->address - MYPC110_EC_RAM_START < MYPC110_EC_RAM_BYTES)
                mailbox->ram[mailbox->address - MYPC110_EC_RAM_START] = (lib_u8)value;
            mailbox->parameter_count = 2u;
        } else if (mailbox->parameter_count == 0u) {
            mailbox->parameter_count = 1u;
            switch (mailbox->command) {
            case 0x0au:
                mailbox->ram[0xffu - MYPC110_EC_RAM_START] = (lib_u8)value;
                break;
            case 0x22u:
                mailbox->ram[0xd0u - MYPC110_EC_RAM_START] = (lib_u8)(
                    (mailbox->ram[0xd0u - MYPC110_EC_RAM_START] & 0xf0u) |
                    (~value & 0x0fu));
                break;
            case 0x23u:
                mailbox->ram[0xd1u - MYPC110_EC_RAM_START] = (lib_u8)(
                    (mailbox->ram[0xd1u - MYPC110_EC_RAM_START] & 1u) |
                    ((~value << 1u) & 0xfeu));
                break;
            case 0xd0u:
            case 0xd1u:
            case 0xd2u:
                mailbox->address = (lib_u16)(((mailbox->command & 3u) << 8u) | value);
                if (mailbox->address >= MYPC110_EC_RAM_START &&
                    mailbox->address - MYPC110_EC_RAM_START < MYPC110_EC_RAM_BYTES)
                    mypc110_ec_mailbox_reply(mailbox,
                        mailbox->ram[mailbox->address - MYPC110_EC_RAM_START]);
                break;
            case 0xd4u:
            case 0xd5u:
            case 0xd6u:
                /* The selected image's D4 entry falls through its BRK/RTI
                 * into D5. Do not incorrectly alias page-1 writes onto RAM 0. */
                mailbox->address = (lib_u16)((mailbox->command == 0xd6u ?
                    0x0200u : 0x0100u) | value);
                break;
            default: break;
            }
        }
    } else return LIB_STATUS_UNSUPPORTED;
    return LIB_STATUS_OK;
}

lib_status mypc110_ec_mailbox_install_routes(x86_core_machine *machine,
    mypc110_ec_mailbox *mailbox)
{
    const x86_core_machine_port_route routes[] = {
        {MYPC110_EC_DATA_PORT, mypc110_ec_mailbox_read, mypc110_ec_mailbox_write,
            mailbox, LIB_FALSE, 0u},
        {MYPC110_EC_COMMAND_STATUS_PORT, mypc110_ec_mailbox_read,
            mypc110_ec_mailbox_write, mailbox, LIB_FALSE, 0u}
    };

    if (machine == LIB_NULL || mailbox == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_status status = x86_core_machine_install_port_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]));
    if (status == LIB_STATUS_OK) {
        /* L2 cold RAM facts from the original controller initialization,
         * not a live chipset capture or a second MCU firmware interpreter. */
        *mailbox = (mypc110_ec_mailbox) {.ram = {
            [0x00] = 0x30u, [0x01] = 0x01u, [0x02] = 0x0bu,
            [0x05] = 0x04u, [0x07] = 0xf8u, [0x08] = 0xceu,
            [0x09] = 0x0bu, [0x0a] = 0x10u, [0x0b] = 0x20u,
            [0x0c] = 0x15u, [0x10] = 0x0bu, [0x18] = 0x02u,
            [0x19] = 0x87u, [0x27] = 0x02u, [0x56] = 0x2bu,
            [0x91] = 0x10u, [0xbf] = 0xffu}};
        lib_memory_set(mailbox->ram + 0x100u - MYPC110_EC_RAM_START, 0xff, 19u);
        lib_memory_set(mailbox->ram + 0x13fu - MYPC110_EC_RAM_START, 0xea, 180u);
        mailbox->ram[0x144u - MYPC110_EC_RAM_START] = 0x14u;
    }
    return status;
}
