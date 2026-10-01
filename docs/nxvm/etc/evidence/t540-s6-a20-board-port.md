# M5 T540 S6 Board-Owned Port 92h

S6 moves the fixed 92h A20 route from generic memory storage to the existing
IBM-PC board owner. `core_machine_create_internal()` registers the same read
and write callbacks at the same construction point, now through
`core_machine_board_register_a20_port()`. The old
`core_machine_memory_register_ports()` function and memory-local 92h constant
are deleted; there is no forwarding route or second A20 state.

This is a boundary cut, not a claim that the complete neutral Core is already
in `src/x86/core`. The memory owner still holds the one A20 state. The 8042
path and profile applicability are unchanged. S7 intake must handle the
remaining private machine layout before physically moving neutral memory.

## Code and test proof

- Five NXVM production files add 34 and remove 33 lines, net +1. One
  NXVM owner-local test adds 21 lines, net +21. Combined source/test net is
  +22; no Shared or MyNES source is changed.
- `machine_instance_smoke` checks initial 92h read, A20 enable and 1 MiB
  memory selection, isolation from a second machine, and A20 disable/wrap.
  The focused test passes on x64 and x86.
- Complete repository-only unit suites pass 467/467 on x64 and 467/467 on
  x86. No fixed I/O address remains in `memory.c`, `port.c`, `clock.c`,
  `timeline.c` or `transaction.c`.
- All four optimized Release products rebuilt on each width; the product
  artifact gate accepted x64 and x86 PE architecture for every profile. The
  x86 Ninja runner stalled before launching a compile command, so x86 Release
  was completed with the same WinLibs UCRT i686 compiler through a fresh
  MinGW Make build tree. No MyNES build or file was changed.
- `git diff --check` and NXVM documentation governance pass.

## Product boundary

Every product remains the existing compiled XT, AT, Model 40 or default
PC/AT profile. This S changes neither ROM/CMOS/media input nor the adjacent
owner INIs. The external integration gate remains T540's final acceptance
gate, not a substitute for the complete unit results above.

| Profile | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| XT 5160 | `56619D7CFDD9CCB773EC8BA2F5A4FAC23B226A87B0064D6489FA8386C916D9EC` | `5885699152F72E25C872D9B042302FD3F28FEDB78B32FBC9A43380549E090B2D` |
| AT 5170 | `68D9657F8C6FAFFBBE05F2F548911100F6DCC0CEFAABFCDFEBEFA79FE3AD4C26` | `D7E90C2A910CE6A644794CED6856297B79E2C4F5BA04739619EB08D6F7D7FAC7` |
| Model 40 | `7E30F5D5F1790D883DAB85920EE5356215E626AFD268E8279BA80583FF95713D` | `443CF8C1EA150C63BA3F30AE6112B06F0C6BD86113B8B9B9570DA9345E706C64` |
| Default PC/AT | `9D7FC303730151977ECF93D294BE23C105304241547F67F1AFC8D54F1CEE7D28` | `D7E97D47E878599678D23ABE5E76439A1E545AB8F877D282CB389B1B5F54F535` |

Each profile directory contains only its current 0540 pair and unchanged
`NXVM.ini`. Earlier binaries remain recoverable in Git history.
