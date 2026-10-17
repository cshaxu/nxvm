# Research Register

## Fixed Reference

Read-only research on 2026-10-17 used the official
[MAME RAIDEN II driver](https://github.com/mamedev/mame/blob/c1425a1af00b88aa07477b1b23a30ed5e5f856ff/src/mame/seibu/raiden2.cpp).
The driver file's latest-path commit returned by GitHub was
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff`; this is not a claim about MAME's
whole-tree HEAD. The retrieved UTF-8 source SHA-256 was
`fdde57f6e9024657f2a9830864523305139562229750b87c967e597324cdb26a`.
No source file, ROM or table was imported, saved into production or compiled.

The file declares LGPL-2.1+ and holders Olivier Galibert, Angelo Salese,
David Haywood and Tomasz Slanina. It is a research reference, not authorization
to copy code/tables into this MIT repository. Follow the source policy first.

## Observations, Not Yet Hardware Certification

At the pinned revision, `raiden2` configuration near line 1121 selects V30 at
32 MHz / 2, a Z80/Seibu sound subsystem, YM2151 and two OKI sample devices.
The sound base near line 1089 uses 28.636363 MHz / 8 for Z80. Video is nominally
320 by 240 native pixels with ROT270 in the game declaration near line 3216;
host portrait orientation, physical aspect and native timing are distinct.
The raster configuration is expressly hand-tuned around 55.47 Hz, not proof of
exact board timing and not permission to assume a generic 60 Hz guest clock.

`ROM_START(raiden2)` near line 1332 is an original parent-set research candidate,
not an owner-set selection. Its two program files are 0x80000 bytes each,
interleaved into a 0x100000 region. Audio address-region allocation likewise
must not be mistaken for dump-file size. There are separate program, sound,
character/background/sprite/sample and protection-related requirements.
Later V33/DX configurations in the same driver are outside the selected goal.

M1 must verify actual board/revision, legal owner files, role/size/hash inventory,
manuals and model grades. No exact ROM inventory, protection completeness,
silicon timing or game compatibility is established by this M0 research.

## Local Capability Audit

The clean local baseline was `da9e84b88`:

- `src/x86-core/cpu/cpu_interface.h` offers Intel 8086/8088/80186/80286/80386
  and selected 80486SX profiles, not NEC V30. Reuse requires a real V30 audit.
- Lib PCM streams provide host delivery; `src/lib/audio/fm/output_interface.h`
  explicitly describes native raw FM output, not software synthesis. It cannot
  supply a YM2151 emulator. No Z80/YM2151/OKI/Seibu implementation was found in
  the local production inventory.
- `src/emulator/product/composition_interface.h`, Product Machine adapter and
  Emulator Machine's driver already provide the host composition and safe
  execution route. Reuse those, not PC board or MyNES private host code.

NEC, Zilog, Yamaha, OKI and Seibu primary hardware documentation remains a
receiving requirement. The bounded external filename search did not inventory
the owner's game set; do not infer possession or absence of lawful ROMs.
