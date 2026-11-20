#ifndef MYOB300_CARD_H
#define MYOB300_CARD_H

#include "app-myob300/ob300_board_interface.h"
#include "x86-core/machine/firmware_interface.h"

#define MYOB300_CARD_BOOT_OFFSET 0x7f0000u
#define MYOB300_CARD_RESET_OFFSET 0x7f8000u
#define MYOB300_CARD_RESET_BYTES 0x8000u
#define MYOB300_CARD_WINDOW_COUNT 4u

extern const x86_core_machine_firmware_provider myob300_card_firmware;
#define MYOB300_CARD_WINDOW_REGISTER_COUNT 3u

typedef struct myob300_card {
    myob300_card_image common;
    myob300_card_image attribute;
    lib_u8 windows[MYOB300_CARD_WINDOW_COUNT * MYOB300_CARD_WINDOW_REGISTER_COUNT];
} myob300_card;

lib_status myob300_card_install(myob300_card *card, x86_core_machine *core);
void myob300_card_reset(myob300_card *card);
lib_status myob300_card_register_read(const myob300_card *card, lib_u8 index,
    lib_u32 *out_value);
lib_status myob300_card_register_write(myob300_card *card, lib_u8 index, lib_u8 value);

#endif
