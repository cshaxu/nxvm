# M5 T540 S43 Board Constructor and Callback Owner

## Actual diff and ownership

`machine.c` lost 245 lines: the existing XT keyboard/PPI/KBC callbacks,
board-configuration validation, and the sole board constructor. The same
functions now live in `machine_board.c` (248 added lines); `machine.h` adds
only two private declarations needed by the unchanged `create_internal`
call. The board constructor still installs the same callback slots, clocks,
high-reset RAM alias, A20 route, VADP, XT keyboard or 8042, DMA, PIC and PIT
in the original order. The single Core candidate allocation, port
registration checkpoint, success publication and common destruction/rollback
path were not duplicated. The one adjusted expression reads the already
resolved `executor_memory.connect.installed_bytes` instead of resolving the
same configuration bytes again when deciding the firmware-less high RAM
alias; at this point the memory allocation has succeeded, so the value is
identical.

Eight source-inventory CMake gates were pointed at the new board owner.
Their required tokens and forbidden Core effects remain checked; in the A20
gate the KBC-to-Core A20 callback moved from the Core input set to the board
input set, while the Core memory-route requirements remain in place. Source
and gate changes add **260**, remove **253** lines (net +7). No new generic
framework, public ABI, chip/timing model, INI, firmware or media input was
introduced. Shared and MyNES files were not edited. The mixed private
`core_machine` structure still awaits S44-S47; no premature Shared move is
claimed.

## Final-source verification

- Complete repository-only unit suite: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates`: x64 and x86 pass, including the eight
  updated board-source inventories, strict CPU matrix, manifest and
  documentation gates.
- External fixed-profile boot probes, **once per profile and width** using
  the matching fixed-profile Release build: Default reaches `dos-prompt`
  on x64/x86; XT, IBM 5170 and Model 40 reach `installer-running` on
  x64/x86. **8/8** accepted terminals. An initial non-matrix attempt used
  the Default-fixed debug probe against XT/Model 40 configuration and was
  rejected before session creation; the matching profile probes were then
  built and used. No guest result was inferred from that mismatched probe.
- Eight optimized 0540 EXEs were rebuilt in the four existing
  `assets/nxvm/<profile>/` directories. `objdump -f` confirms four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds zero `.debug`
  sections. SHA-256 by profile, x64 then x86:
  - Model 40: `7B45FFBA1FB4CABDDC16ADC45CC5F1B09447D6709E89486EF7956AD4C68B5DF8`, `471C965EAC8E8CED9DFE8A266F759FB741959A232DCC48A6C45E67BF0EAB0AC6`.
  - Default: `99B21740DCB8A3FCA1BBA8FA30A4A830FDAC0FB3B6D0A3484B9222CA6D3908F0`, `EAA8E004E244CA05389E120621A4B4F8BF3FC8A4C271AFA7CE398699436EBC5E`.
  - XT: `9ED2ACDA2185BAD3724A0878A78648BEC54FDE17455093F9FFDFBFC3F773CDA5`, `B68EF8CDBCE2B5EAE9FF553EAF541B25E12C2DB27011B6C6D1A8930A15336DE9`.
  - IBM 5170: `EE7DC107FD6510CF340E95E48E2969B3C20C3D007F591EDA1B69CD781CE54E62`, `8BF218FE1FAC385BA44C36A6D0912C6B7287A41B88D84533D0EE5688F02BD97F`.

S43 closes only constructor/callback placement. S44 receives named board
device-clock/topology state; T540 remains open for the private-state split,
neutral Core move and later IBM-PC board extraction.
