#include "app-myob300/ob300_card.h"
#include "x86-core/machine/memory_interface.h"

#define MYOB300_CARD_INDEX_BASE 0x26u
#define MYOB300_CARD_PAGE_SHIFT 14u
#define MYOB300_CARD_PAGE_BYTES (1u << MYOB300_CARD_PAGE_SHIFT)
#define MYOB300_CARD_LEGACY_START 0xd0000u
#define MYOB300_CARD_BOOT_START 0xf0000u
#define MYOB300_CARD_LEGACY_END 0xf8000u
#define MYOB300_CARD_PAGE_MASK 0x3fu
#define MYOB300_CARD_WINDOW_ENABLED 0x80u
#define MYOB300_CARD_COMMON_SELECT 0x40u
#define MYOB300_CARD_VIDEO_ROM_BASE 0xe0000u
#define MYOB300_CARD_VIDEO_ROM_OFFSET 0x7e0000u
#define MYOB300_CARD_OPTION_BASE 0xe8000u
#define MYOB300_CARD_OPTION_OFFSET 0x7e8000u
#define MYOB300_CARD_OPTION_BYTES 0x8000u
/* L1: the high half of the 24-bit bus exposes common card offsets.
 * ROM Windows reads this aperture beyond the first two MiB. */
#define MYOB300_CARD_HIGH_BASE 0x00800000u
#define MYOB300_CARD_HIGH_END 0x01000000u

static lib_status myob300_card_firmware_configure(void *owner,
    x86_core_machine_firmware_context *firmware)
{
    const myob300_card_image *image = owner;
    lib_status status;

    status = x86_core_machine_firmware_register_immutable_rom(firmware,
        MYOB300_CARD_VIDEO_ROM_BASE, image->data + MYOB300_CARD_VIDEO_ROM_OFFSET,
        MYOB300_CARD_OPTION_BYTES);
    if (status != LIB_STATUS_OK) return status;
    return x86_core_machine_firmware_register_immutable_rom(firmware,
        MYOB300_CARD_OPTION_BASE, image->data + MYOB300_CARD_OPTION_OFFSET,
        MYOB300_CARD_OPTION_BYTES);
}

static lib_status myob300_card_firmware_reset(void *owner,
    x86_core_machine_firmware_context *firmware)
{
    /* Board reset owns the card's bank registers. Immutable ROM needs no reload. */
    (void)owner;
    (void)firmware;
    return LIB_STATUS_OK;
}

const x86_core_machine_firmware_provider myob300_card_firmware = {
    .configure = myob300_card_firmware_configure,
    .reset = myob300_card_firmware_reset
};

void myob300_card_reset(myob300_card *card)
{
    for (lib_size i = 0u; i < MYOB300_CARD_WINDOW_COUNT; ++i) {
        lib_u8 *window = card->windows + i * MYOB300_CARD_WINDOW_REGISTER_COUNT;
        window[0] = (lib_u8)(0xf4u + i);
        window[1] = 0x80u;
        window[2] = 0u;
    }
}

lib_status myob300_card_register_read(const myob300_card *card, lib_u8 index,
    lib_u32 *out_value)
{
    if (index < MYOB300_CARD_INDEX_BASE ||
        index - MYOB300_CARD_INDEX_BASE >= sizeof(card->windows)) return LIB_STATUS_UNSUPPORTED;
    *out_value = card->windows[index - MYOB300_CARD_INDEX_BASE];
    return LIB_STATUS_OK;
}

lib_status myob300_card_register_write(myob300_card *card, lib_u8 index, lib_u8 value)
{
    if (index < MYOB300_CARD_INDEX_BASE ||
        index - MYOB300_CARD_INDEX_BASE >= sizeof(card->windows)) return LIB_STATUS_UNSUPPORTED;
    card->windows[index - MYOB300_CARD_INDEX_BASE] = value;
    return LIB_STATUS_OK;
}

/* ROM 1.0 ABA reads common space through both F4/81/FD and BC/C2/00.
 * Selection priority and the complementary attribute mode are modeled. */
static lib_bool myob300_card_resolve(const myob300_card *card, lib_u32 address,
    const lib_u8 **out_byte)
{
    const lib_u32 guest_page = address >> MYOB300_CARD_PAGE_SHIFT;
    lib_size offset;

    *out_byte = LIB_NULL;
    for (lib_size i = 0u; i < MYOB300_CARD_WINDOW_COUNT; ++i) {
        const lib_u8 *window = card->windows + i * MYOB300_CARD_WINDOW_REGISTER_COUNT;
        const myob300_card_image *image;
        const lib_bool common = ((window[0] | window[1]) & MYOB300_CARD_COMMON_SELECT) != 0u;

        if ((window[0] & MYOB300_CARD_WINDOW_ENABLED) == 0u ||
            (window[0] & MYOB300_CARD_PAGE_MASK) != guest_page) continue;
        offset = (((lib_u32)(window[1] & MYOB300_CARD_PAGE_MASK) << 8u) | window[2]) *
            MYOB300_CARD_PAGE_BYTES + (address & (MYOB300_CARD_PAGE_BYTES - 1u));
        image = common ? &card->common : &card->attribute;
        if (!common && (offset & 1u) != 0u) return LIB_TRUE;
        if (!common) offset >>= 1u;
        if (offset < image->bytes) *out_byte = image->data + offset;
        return LIB_TRUE;
    }
    if (address >= MYOB300_CARD_BOOT_START && card->common.data != LIB_NULL) {
        offset = MYOB300_CARD_BOOT_OFFSET + address - MYOB300_CARD_BOOT_START;
        *out_byte = card->common.data + offset;
        return LIB_TRUE;
    }
    return LIB_FALSE;
}

static lib_status myob300_card_transfer(const myob300_card *card, lib_u32 physical,
    lib_uptr bytes, lib_u8 *destination)
{
    const lib_u8 *source;

    /* The high common window starts at card offset zero. */
    if (physical >= MYOB300_CARD_HIGH_BASE && physical < MYOB300_CARD_HIGH_END) {
        const lib_u32 offset = physical - MYOB300_CARD_HIGH_BASE;

        if (bytes == 0u || bytes > MYOB300_CARD_HIGH_END - physical ||
            card->common.data == LIB_NULL || offset + bytes > card->common.bytes)
            return LIB_STATUS_UNSUPPORTED;
        if (destination != LIB_NULL)
            lib_memory_copy(destination, card->common.data + offset, bytes);
        return LIB_STATUS_OK;
    }
    if (physical < MYOB300_CARD_LEGACY_START || physical >= MYOB300_CARD_LEGACY_END ||
        bytes == 0u || bytes > MYOB300_CARD_LEGACY_END - physical)
        return LIB_STATUS_UNSUPPORTED;
    for (lib_uptr i = 0u; i < bytes; ++i) {
        if (!myob300_card_resolve(card, physical + (lib_u32)i, &source))
            return LIB_STATUS_UNSUPPORTED;
        if (destination != LIB_NULL) destination[i] = source == LIB_NULL ? 0xffu : *source;
    }
    return LIB_STATUS_OK;
}

static lib_status myob300_card_query(void *owner, lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access)
{
    (void)access;
    return myob300_card_transfer(owner, physical, bytes, LIB_NULL);
}

static lib_status myob300_card_read(void *owner, lib_u32 physical, lib_uptr destination,
    lib_uptr bytes, lib_bool observe_only)
{
    (void)observe_only;
    return myob300_card_transfer(owner, physical, bytes, (lib_u8 *)destination);
}

static lib_status myob300_card_write(void *owner, lib_u32 physical, lib_uptr source,
    lib_uptr bytes)
{
    (void)source;
    return myob300_card_transfer(owner, physical, bytes, LIB_NULL);
}

lib_status myob300_card_install(myob300_card *card, x86_core_machine *core)
{
    const x86_core_machine_memory_device_route routes[] = {
        {MYOB300_CARD_LEGACY_START, MYOB300_CARD_LEGACY_END - MYOB300_CARD_LEGACY_START,
            {myob300_card_read, myob300_card_write, myob300_card_query},
            X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT_PARTIAL, LIB_NULL},
        {MYOB300_CARD_HIGH_BASE, MYOB300_CARD_HIGH_END - MYOB300_CARD_HIGH_BASE,
            {myob300_card_read, myob300_card_write, myob300_card_query},
            X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD, LIB_NULL}
    };

    myob300_card_reset(card);
    return x86_core_machine_install_memory_device_routes(core, routes,
        sizeof(routes) / sizeof(routes[0]), LIB_NULL, LIB_NULL, card);
}
