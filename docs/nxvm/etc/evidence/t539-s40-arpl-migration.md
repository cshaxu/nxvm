# T539 S40: ARPL Test Owner Migration

## Original-case receiving map

| Retired source / original family | Receiver and preserved observation |
| --- | --- |
| Base ARPL: 80286 adjust/retain register forms (2), ES memory-prefix form (1), legacy #UD profiles (3), opcode metadata (1) | `cpu_arpl_smoke.c` retains the register/RPL/ZF, memory-prefix, terminal #UD and minimum-profile assertions. `machine_arpl_board_smoke.c` separately retains the guest-loaded 80286 GDT/ES physical-memory case. |
| S53 direct matrix: eight destination × eight source encodings (64) | `cpu_arpl_smoke.c` retains low/high GPR halves, ZF/other flags, IP, all other GPRs and unchanged segment caches. |
| S53 RPL/flags change and retain (2) | `cpu_arpl_smoke.c` retains destination and unaffected register/flag assertions. |
| S53 DS/SS/ES/FS/GS, address-size, combined-size and operand-size memory forms (8) | `cpu_arpl_smoke.c` retains segment-base and effective-address selection, destination/adjacent memory, flags and unchanged registers/caches. |
| S53 legacy attribute rejects (3 profiles × 3 encodings = 9) and invalid LOCK forms (4) | `cpu_arpl_smoke.c` retains terminal #UD, full CPU rollback and unchanged memory. The base's three bare legacy #UD forms are retained separately. |
| S53 protected read-limit fault (1) | `cpu_arpl_smoke.c` retains hidden-cache limit and rollback; `machine_arpl_board_smoke.c` retains guest descriptor loading, delivered #GP, error-code/IP/CS/FLAGS frame and unchanged memory/caches. |
| S53 pending PIC IRQ after ARPL, ZF clear/set (2) | `machine_arpl_board_smoke.c` retains real PIC ISR/IRR, delivered stack frame, register/segment preservation and RPL/ZF outcome. |
| S53 legacy/mode wrapper | Its repeated base assertions have the same CPU/board receivers above, not a second implementation. |

The retired sources had no instruction-timing assertions. The original direct
`#include "core_machine_arpl_smoke.c"` dependency and both mixed-owner tests are
deleted. CPU tests link only `x86-cpu`; board tests use guest bootstrap,
public machine debug/read operations and the real board PIC. No production
source, public ABI, firmware, profile, INI, Shared component, MyNES source or
EXE input changes. The direct-private `.c` consumer inventory falls 58 to 56;
the common fixture header and S41-S51 groups remain pending.

## Verification and change audit

The eight code/test/build/gate paths add 689 and remove 941 tracked lines,
net minus 252 by `git diff --numstat`; this evidence is excluded. The old
910-line pair is replaced by 369 CPU and 288 board lines; the remaining count
is exact build/gate registration. One CPU path and one board path replace two
registered old tests, so the complete unit count remains 413 per width.
T317/T332 strict historical owner counts fall 35 to 34. T344 classifies one
new direct public-board constructor rather than hiding it under the old
fixture rule. The board receiver does not access a private CPU field.

Complete x64 and x86 builds pass. Both complete unit suites pass 413/413.
The specialized-gate aggregate passes 66/66 on both widths; T344 direct
compilation passes 412 rows on each width. Six unchanged Shared manifests pass 6/6 on both
widths; documentation governance and `git diff --check` pass. No tracked EXE
was modified. Implementation P1 and actual-commit coordinator review are
recorded after push; CPU extraction as a whole remains open.
