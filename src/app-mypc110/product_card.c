#include "app-mypc110/product_card.h"
#include "lib/types/file.h"

void mypc110_product_cards_initialize(mypc110_product_cards *cards)
{
    if (cards == LIB_NULL) return;
    *cards = (mypc110_product_cards){0};
    for (lib_u8 slot = 0u; slot < MYPC110_CARD_SLOT_COUNT; ++slot)
        lib_atomic_i32_initialize(&cards->slot[slot].pending, 0);
}

lib_status mypc110_product_cards_close_pending(mypc110_product_cards *cards)
{
    lib_status first = LIB_STATUS_OK;
    if (cards == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_u8 slot = 0u; slot < MYPC110_CARD_SLOT_COUNT; ++slot) {
        mypc110_card_slot *entry = &cards->slot[slot];
        const lib_status status = mypc110_product_card_close_pair(&entry->pending_common,
            &entry->pending_attribute);
        lib_atomic_i32_store_explicit(&entry->pending, 0, LIB_MEMORY_ORDER_SEQ_CST);
        if (first == LIB_STATUS_OK) first = status;
    }
    return first;
}

static char *mypc110_product_card_token(char **cursor)
{
    char *token;
    while (**cursor == ' ' || **cursor == '\t') ++*cursor;
    token = *cursor;
    while (**cursor != '\0' && **cursor != ' ' && **cursor != '\t') ++*cursor;
    if (**cursor != '\0') *(*cursor)++ = '\0';
    return token;
}

static void mypc110_product_card_trim(char *text)
{
    lib_size length = lib_text_length(text);
    while (length != 0u && (text[length - 1u] == ' ' || text[length - 1u] == '\t'))
        text[--length] = '\0';
}

lib_bool mypc110_product_card_submit(mypc110_product_cards *cards,
    emulator_machine *machine, emulator_session_machine_state state,
    const char *line, emulator_session_command_result *out)
{
    char buffer[2048], *cursor, *operation, *slot_name, *mode_name, *common_path, *attribute_path;
    lib_storage_medium *common = LIB_NULL, *attribute = LIB_NULL;
    lib_storage_medium_mode mode = LIB_STORAGE_MEDIUM_READONLY;
    lib_size length;
    lib_u8 slot;
    lib_status status;

    if (cards == LIB_NULL || line == LIB_NULL || out == LIB_NULL) return LIB_FALSE;
    while (*line == ' ' || *line == '\t') ++line;
    length = lib_text_length(line);
    if (length < 6u || lib_text_compare_n(line, "pccard", 6u) != 0 ||
        (length > 6u && line[6] != ' ' && line[6] != '\t')) return LIB_FALSE;
    *out = (emulator_session_command_result){0};
    out->arm_prompt = LIB_TRUE;
    if (machine == LIB_NULL || state != EMULATOR_SESSION_MACHINE_STOPPED ||
        emulator_machine_state_get(machine) != EMULATOR_MACHINE_STOPPED) {
        (void)lib_c_snprintf(out->text, sizeof(out->text),
            "PC cards can only be changed while stopped.\r\n");
        return LIB_TRUE;
    }
    if (length >= sizeof(buffer)) goto usage;
    lib_memory_copy(buffer, line, length + 1u);
    cursor = buffer;
    (void)mypc110_product_card_token(&cursor);
    operation = mypc110_product_card_token(&cursor);
    slot_name = mypc110_product_card_token(&cursor);
    if (lib_text_compare(slot_name, "a") == 0) slot = 0u;
    else if (lib_text_compare(slot_name, "b") == 0) slot = 1u;
    else goto usage;
    if (lib_text_compare(operation, "insert") == 0) {
        mode_name = mypc110_product_card_token(&cursor);
        if (lib_text_compare(mode_name, "readonly") == 0) mode = LIB_STORAGE_MEDIUM_READONLY;
        else if (lib_text_compare(mode_name, "overlay") == 0) mode = LIB_STORAGE_MEDIUM_OVERLAY;
        else if (lib_text_compare(mode_name, "direct") == 0) mode = LIB_STORAGE_MEDIUM_DIRECT;
        else goto usage;
        while (*cursor == ' ' || *cursor == '\t') ++cursor;
        common_path = cursor;
        while (*cursor != '\0' && *cursor != '|') ++cursor;
        if (*cursor != '|') goto usage;
        *cursor++ = '\0';
        while (*cursor == ' ' || *cursor == '\t') ++cursor;
        attribute_path = cursor;
        mypc110_product_card_trim(common_path);
        mypc110_product_card_trim(attribute_path);
        if (*common_path == '\0' || *attribute_path == '\0') goto usage;
        status = lib_storage_medium_open(common_path, mode, &common);
        if (status == LIB_STATUS_OK)
            status = lib_storage_medium_open(attribute_path,
                mode == LIB_STORAGE_MEDIUM_READONLY ? LIB_STORAGE_MEDIUM_READONLY :
                    LIB_STORAGE_MEDIUM_OVERLAY, &attribute);
        if (status != LIB_STATUS_OK) {
            (void)mypc110_product_card_close_pair(&common, &attribute);
            (void)lib_c_snprintf(out->text, sizeof(out->text), "Cannot open PC card images.\r\n");
            return LIB_TRUE;
        }
    } else if (lib_text_compare(operation, "eject") != 0 ||
        *mypc110_product_card_token(&cursor) != '\0') goto usage;

    mypc110_card_slot *entry = &cards->slot[slot];
    if (lib_atomic_i32_load_explicit(&entry->pending, LIB_MEMORY_ORDER_SEQ_CST) != 0) {
        status = mypc110_product_card_close_pair(&entry->pending_common, &entry->pending_attribute);
        lib_atomic_i32_store_explicit(&entry->pending, 0, LIB_MEMORY_ORDER_SEQ_CST);
        if (status != LIB_STATUS_OK) {
            (void)mypc110_product_card_close_pair(&common, &attribute);
            (void)lib_c_snprintf(out->text, sizeof(out->text), "Cannot replace pending PC card.\r\n");
            return LIB_TRUE;
        }
    }
    entry->pending_common = common;
    entry->pending_attribute = attribute;
    entry->pending_readonly = mode == LIB_STORAGE_MEDIUM_READONLY;
    /* Session serializes commands; a stopped worker observes this publication
     * before consuming the request at its next reset boundary. */
    lib_atomic_i32_store_explicit(&entry->pending, 1, LIB_MEMORY_ORDER_SEQ_CST);
    (void)lib_c_snprintf(out->text, sizeof(out->text),
        "PC card %c change scheduled for next start/reset.\r\n", slot == 0u ? 'A' : 'B');
    return LIB_TRUE;

usage:
    (void)lib_c_snprintf(out->text, sizeof(out->text),
        "Usage: pccard insert <a|b> <readonly|overlay|direct> <common>|<attribute>\r\n"
        "       pccard eject <a|b>\r\n");
    return LIB_TRUE;
}
