/* Copyright 2012-2026 Neko. */
/* Offline construction only: no Core or machine instance is linked here. */
#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "lib/storage/file_interface.h"
#include "product/xasm32/xasm32_interface.h"

#define BIOS_BYTES 0x10000u
#define BIOS_IVT 0xf800u
#define BIOS_BDA 0xfc00u

static void bios_word(lib_u8 *image, lib_size offset, lib_u16 value)
{
    image[offset] = (lib_u8)value;
    image[offset + 1u] = (lib_u8)(value >> 8u);
}

static lib_status bios_assemble(lib_u8 *image, lib_size *offset, lib_size limit,
    const char *directory, const char *name)
{
    char path[1024];
    void *text = LIB_NULL;
    lib_size text_bytes = 0u;
    lib_size code_bytes = 0u;
    lib_i32 count;
    lib_status status;

    count = lib_c_snprintf(path, sizeof(path), "%s/%s.asm", directory, name);
    if (count < 0 || (lib_size)count >= sizeof(path) || *offset >= limit ||
        limit > BIOS_BYTES) return LIB_STATUS_LIMIT_EXCEEDED;
    status = lib_storage_file_read_owned(path, 1024u * 1024u, &text, &text_bytes);
    if (status == LIB_STATUS_OK) status = product_xasm32_assemble_paragraph(text,
        text_bytes, image + *offset, limit - *offset, &code_bytes, LIB_FALSE);
    lib_release(text);
    if (status != LIB_STATUS_OK || code_bytes == 0u) {
        lib_c_fprintf(lib_c_stderr, "Cannot assemble %s: status %d\n", name, status);
        return status == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : status;
    }
    lib_c_printf("%04zx %04zx %s\n", *offset, code_bytes, name);
    *offset += code_bytes;
    return LIB_STATUS_OK;
}

static void bios_keyboard_tables(lib_u8 *image)
{
    static const lib_u8 normal[0x59] = {
        [0x01] = 0x1bu, [0x02] = '1', [0x03] = '2', [0x04] = '3',
        [0x05] = '4', [0x06] = '5', [0x07] = '6', [0x08] = '7',
        [0x09] = '8', [0x0a] = '9', [0x0b] = '0', [0x0c] = '-',
        [0x0d] = '=', [0x0e] = 0x08u, [0x0f] = 0x09u,
        [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
        [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
        [0x18] = 'o', [0x19] = 'p', [0x1a] = '[', [0x1b] = ']',
        [0x1c] = 0x0du, [0x1e] = 'a', [0x1f] = 's', [0x20] = 'd',
        [0x21] = 'f', [0x22] = 'g', [0x23] = 'h', [0x24] = 'j',
        [0x25] = 'k', [0x26] = 'l', [0x27] = ';', [0x28] = '\'',
        [0x29] = '`', [0x2b] = '\\', [0x2c] = 'z', [0x2d] = 'x',
        [0x2e] = 'c', [0x2f] = 'v', [0x30] = 'b', [0x31] = 'n',
        [0x32] = 'm', [0x33] = ',', [0x34] = '.', [0x35] = '/',
        [0x37] = '*', [0x39] = ' ', [0x4a] = '-', [0x4e] = '+'
    };
    static const lib_u8 shifted[0x59] = {
        [0x01] = 0x1bu, [0x02] = '!', [0x03] = '@', [0x04] = '#',
        [0x05] = '$', [0x06] = '%', [0x07] = '^', [0x08] = '&',
        [0x09] = '*', [0x0a] = '(', [0x0b] = ')', [0x0c] = '_',
        [0x0d] = '+', [0x0e] = 0x08u, [0x10] = 'Q', [0x11] = 'W',
        [0x12] = 'E', [0x13] = 'R', [0x14] = 'T', [0x15] = 'Y',
        [0x16] = 'U', [0x17] = 'I', [0x18] = 'O', [0x19] = 'P',
        [0x1a] = '{', [0x1b] = '}', [0x1c] = 0x0du, [0x1e] = 'A',
        [0x1f] = 'S', [0x20] = 'D', [0x21] = 'F', [0x22] = 'G',
        [0x23] = 'H', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L',
        [0x27] = ':', [0x28] = '"', [0x29] = '~', [0x2b] = '|',
        [0x2c] = 'Z', [0x2d] = 'X', [0x2e] = 'C', [0x2f] = 'V',
        [0x30] = 'B', [0x31] = 'N', [0x32] = 'M', [0x33] = '<',
        [0x34] = '>', [0x35] = '?', [0x37] = '*', [0x39] = ' ',
        [0x4a] = '-', [0x4e] = '+'
    };

    lib_memory_copy(image + 0xe000u, normal, sizeof(normal));
    lib_memory_copy(image + 0xe080u, shifted, sizeof(shifted));
}

static void bios_tables(lib_u8 *image)
{
    lib_size index;
    lib_u8 *bda = image + BIOS_BDA;

    for (index = 0u; index < 256u; ++index)
        bios_word(image, BIOS_IVT + index * 4u + 2u, 0xf000u);
    bios_keyboard_tables(image);
    /* INT 15h C0 system descriptor, matching the existing default firmware. */
    image[0xe6f5u] = 8u;
    image[0xe6f7u] = 0xfcu;
    image[0xe6f9u] = 1u;
    image[0xe6fau] = 0xb4u;
    image[0xe6fbu] = 0x40u;
    /* Retained fixed disk parameter table: 100 cylinders, 16 heads, 63 sectors. */
    bios_word(image, BIOS_IVT + 0x41u * 4u, 0xf7e0u);
    bios_word(image, 0xf7e0u, 100u);
    image[0xf7e2u] = 16u;
    image[0xf7eeu] = 63u;
    bios_word(image, BIOS_IVT + 0x1eu * 4u, 0xf7c0u);
    lib_memory_copy(image + 0xf7c0u, (const lib_u8[]) {
        0xafu, 0x02u, 0x25u, 0x02u, 18u, 0x1bu, 0xffu, 0x6cu,
        0xf6u, 15u, 8u }, 11u);
    bios_word(bda, 0x00u, 0x03f8u);
    bios_word(bda, 0x08u, 0x0378u);
    bios_word(bda, 0x0eu, 0x9fc0u);
    bios_word(bda, 0x10u, 0x0021u);
    bios_word(bda, 0x13u, 639u); /* Last KiB is the floppy DMA bounce buffer. */
    bda[0x17u] = 0x20u;
    bios_word(bda, 0x1au, 0x041eu);
    bios_word(bda, 0x1cu, 0x041eu);
    bda[0x40u] = 0x25u;
    bda[0x45u] = 1u;
    bda[0x47u] = 1u;
    bda[0x48u] = 2u;
    bda[0x49u] = 3u;
    bios_word(bda, 0x4au, 80u);
    bios_word(bda, 0x4cu, 0x1000u);
    bios_word(bda, 0x50u, 0x0500u);
    bda[0x60u] = 0x0eu;
    bda[0x61u] = 0x0du;
    bios_word(bda, 0x63u, 0x03d4u);
    bda[0x65u] = 0x29u;
    bda[0x66u] = 0x30u;
    bda[0x75u] = 1u;
    bda[0x76u] = 0xc0u;
    bda[0x78u] = 0x14u;
    bda[0x7cu] = 0x0au;
    bios_word(bda, 0x80u, 0x041eu);
    bios_word(bda, 0x82u, 0x043du);
    bda[0x84u] = 24u;
    bios_word(bda, 0x85u, 16u);
    bda[0x87u] = 0x60u;
    bda[0x88u] = 9u;
    bda[0x89u] = 0x11u;
    bda[0x8au] = 0x0bu;
    bda[0x8fu] = 0x77u;
    bda[0x90u] = 0x17u;
    bda[0x96u] = 0x10u;
    bda[0x97u] = 2u;
    bda[0xacu] = 18u;
    bda[0xadu] = 79u;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    static const struct {
        lib_u8 vector;
        lib_u16 fixed_offset;
        const char *name;
    } interrupts[] = {
        {0x08u, 0u, "timer_irq"}, {0x09u, 0u, "keyboard_irq"},
        {0x0eu, 0u, "floppy_irq"}, {0x76u, 0u, "disk_irq"},
        {0x10u, 0x4000u, "video"},
        {0x11u, 0u, "equipment"}, {0x12u, 0u, "memory"},
        {0x13u, 0u, "disk"}, {0x15u, 0u, "system"},
        {0x16u, 0u, "keyboard"}, {0x1au, 0u, "clock"},
        {0x40u, 0x0c00u, "floppy"}
    };
    static const char *const posts[] = {
        "entry", "rtc_post", "dma_post", "pit_post", "pic_post", "floppy_post", "boot"
    };
    lib_u8 *image;
    lib_size cursor = 1u;
    lib_size index;
    lib_status status = LIB_STATUS_OK;
    lib_storage_file_writer *writer = LIB_NULL;

    if (argc != 3) {
        lib_c_fprintf(lib_c_stderr, "Usage: nxvm-firmware-build source-directory output.rom\n");
        return 1;
    }
    image = lib_allocate_zero(BIOS_BYTES, 1u);
    if (image == LIB_NULL) return 1;
    image[0u] = 0xcfu; /* Default interrupt entry is IRET. */
    bios_tables(image);
    for (index = 0u; status == LIB_STATUS_OK &&
        index < sizeof(interrupts) / sizeof(interrupts[0u]); ++index) {
        lib_size offset = interrupts[index].fixed_offset != 0u ?
            interrupts[index].fixed_offset : cursor;
        lib_size limit = offset == 0x4000u ? 0xe000u :
            offset == 0x0c00u ? 0x4000u : 0x0c00u;

        bios_word(image, BIOS_IVT + interrupts[index].vector * 4u, (lib_u16)offset);
        status = bios_assemble(image, &offset, limit, argv[1u], interrupts[index].name);
        if (interrupts[index].fixed_offset == 0u) cursor = offset;
    }
    image[0xfff0u] = 0xeau;
    bios_word(image, 0xfff1u, (lib_u16)cursor);
    bios_word(image, 0xfff3u, 0xf000u);
    for (index = 0u; status == LIB_STATUS_OK &&
        index < sizeof(posts) / sizeof(posts[0u]); ++index)
        status = bios_assemble(image, &cursor, 0x0c00u, argv[1u], posts[index]);
    if (status == LIB_STATUS_OK) status = lib_storage_file_writer_open(argv[2u],
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer);
    if (status == LIB_STATUS_OK) status = lib_storage_file_writer_write(writer,
        image, BIOS_BYTES);
    if (writer != LIB_NULL) {
        lib_status closed = lib_storage_file_writer_close(writer);
        if (status == LIB_STATUS_OK) status = closed;
    }
    lib_release(image);
    return status == LIB_STATUS_OK ? 0 : 1;
}
