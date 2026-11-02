#ifndef MYPC110_PRODUCT_CARD_H
#define MYPC110_PRODUCT_CARD_H

#include "lib/storage/medium_interface.h"
#include "lib/types/atomic.h"
#include "x86-ibmpc/product/command_interface.h"

#define MYPC110_CARD_SLOT_COUNT 2u

typedef struct mypc110_card_slot {
    lib_atomic_i32 pending;
    lib_storage_medium *pending_common;
    lib_storage_medium *pending_attribute;
    lib_storage_medium *active_common;
    lib_storage_medium *active_attribute;
    lib_bool pending_readonly;
} mypc110_card_slot;

typedef struct mypc110_product_cards {
    mypc110_card_slot slot[MYPC110_CARD_SLOT_COUNT];
} mypc110_product_cards;

static inline lib_status mypc110_product_card_close_pair(lib_storage_medium **common,
    lib_storage_medium **attribute)
{
    const lib_status first = lib_storage_medium_destroy(common);
    const lib_status second = lib_storage_medium_destroy(attribute);
    return first != LIB_STATUS_OK ? first : second;
}

void mypc110_product_cards_initialize(mypc110_product_cards *cards);
lib_bool mypc110_product_card_submit(mypc110_product_cards *cards,
    emulator_machine *machine, emulator_session_machine_state state,
    const char *line, emulator_session_command_result *out);
/* Called only after the machine is stopped or before it was published. */
lib_status mypc110_product_cards_close_pending(mypc110_product_cards *cards);

#endif
