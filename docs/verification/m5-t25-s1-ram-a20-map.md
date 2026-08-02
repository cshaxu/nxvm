# M5 T25 S1 RAM/A20 Authority Map

The retained full-PC RAM authority is `t_ram vram` in
`core/machine/vram.c`. It owns one backing allocation, A20 state, port `0x92`
callbacks, real-address helpers, physical access, and the historic high-ROM
read mirror. The live-machine carrier currently references that object.

`nxvm_core_machine.memory` is deliberately separate M3 minimal-model storage.
It remains outside full-PC composition and is not a second RAM authority for
NXVM. The next subtask moves the retained source into `memory.*`, embeds that
same `t_ram` object in the carrier, and leaves compatibility spellings as
direct aliases only.

This is a mapping-only subtask: no source behavior changed and no task
executable is required.
