#ifndef MYPC110_FONT_INTERFACE_H
#define MYPC110_FONT_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"

#define MYPC110_FONT_ROM_BYTES (1024u * 1024u)
#define MYPC110_FONT_BANK_BYTES (8u * 1024u)
#define MYPC110_FONT_BANK_COUNT 128u
#define MYPC110_FONT_BANK_PORT 0x1160u
#define MYPC110_FONT_PHYSICAL_START 0x000de000u

/* The observed PC110 font device exposes one selected immutable 8 KiB bank.
 * It deliberately does not invent behavior for 1161h--1163h beyond the
 * independently observed bank/window path. The caller owns `bytes` for the
 * installed machine lifetime. */
typedef struct mypc110_font_rom {
    const lib_u8 *bytes;
    lib_u8 bank;
} mypc110_font_rom;

lib_status mypc110_font_rom_create(const lib_u8 *bytes, lib_size byte_count,
    mypc110_font_rom *out_font);
lib_status mypc110_font_rom_install(x86_core_machine *machine,
    mypc110_font_rom *font);

#endif
