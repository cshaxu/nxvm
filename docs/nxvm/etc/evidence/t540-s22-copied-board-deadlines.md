# M5 T540 S22 Copied Board Deadlines

S22 moves IBM-PC device deadline discovery out of the Core time-decision
routine. The board returns a copied observation containing source ticks,
immediate-due, unsourced-DMA L1 compatibility and fast-advance blocking.
Core still owns `elapsed_ticks`, the timeline and FPU completion, chooses the
minimum and performs the only guest-time advancement. There is no second
clock, scheduler, chip state copy or new timing formula.

## Owner and difference review

- The old `machine_scheduler.c` queried PIT and auxiliary PIT, RTC, DMA,
  FDC, HDC, KBC, PIC, XT keyboard and D4 refresh directly. These exact
  queries now live in `board_deadline.c`; the Core observation reads only
  the copied result. FPU completion and generic timeline next-due remain
  in Core. Device advancement is unchanged and belongs to S23.
- The board receives Core `now` as a value, not authority to advance time.
  It converts the same frozen device-clock ratios and compares the same
  absolute FDC/HDC, PIC and refresh due ticks. Immediate due beats L1;
  unsourced DMA remains L1 and blocks fast advance; an FDC due at/before
  `now` continues to block fast advance. Unknown conversions remain absent,
  not fabricated deadlines. Existing overflow and query-failure branches
  retain their previous behavior.
- One provider is bound at machine creation. Both public observation and
  scheduler advancement use the same `capture_time_with_board` helper, so
  the board is queried once per Core decision. A synthetic provider unit
  checks minimum deadline, immediate, L1 and blocked disposition. The new
  gate rejects direct board queries in the Core observation prefix and
  requires the copied board seam and sole Core publication path.
- Two older static gates were updated at their source-owner checks: the
  FDC contract is now looked up in `board_deadline.c`, and the T499
  scheduler check recognizes the copied `fast_advance_blocked` fact. The
  existing FDC negative fixture now includes the new file. No gate was
  removed or weakened; its negative controls still pass.

Production changes add 160 and remove 136 lines (net +24): most of the 121
new board-file lines are moved deadline logic. The Core scheduler itself
removes 136 and adds 24 lines. No Shared, MyNES, external asset, firmware,
INI or profile source was changed.

## Verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
  The first runs were 468/469 solely because the old FDC negative fixture
  did not copy the new owner file. After repairing the fixture, its focused
  negative test and both complete suites passed.
- `verify-current-specialized-gates` passed **75/75** steps, including the
  new S22 board-deadline gate and both updated older owner gates.
- One external boot checkpoint per four existing profiles and width:
  **8/8 passed**, with no repeated group. The optimized 0540 products were
  rebuilt in both widths and installed under the unchanged profile asset
  directories. `objdump` reports `pei-x86-64` for x64 and `pei-i386` for x86;
  all eight have zero `.debug` sections. Owner INIs were not edited.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `6CC7D06BBF4C46AB0F6E9EDE1EE367FB66AE9D1D1B9E97E347BBF6716D073AA8` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `ED03063F41807548F6F4CDF0805BAB6108CC5F89E9DAA50017EFCB3038416C51` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `0625D6E29C7C5EDF1F599DD8EF249A79BC655D7284F70E9F1FCCDF3CC7E902C2` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `B1EB5080F69A9F19F07B99C4F8B11D188463E6BD1AD2B806BFD7942A1AED3504` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `5013AC32804E3DFDA41A1B275D176335D404D00E5A58A5A7D9A1AD2E3AE1601A` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `1BDA2F8533EAF0D22665F0EB0A6314396077F5E56AD3E3440C4FE06D8C9BBFA6` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `6CE5F129794725B3B1F99C9B7475D661EB6E6C61D7160B20AF69E1F0FF50307C` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `BB05C54092A2051915A6EAB6704848FACD98C4FED96D87CE75F2B750E1E2D9A6` |

S22 closes only copied board deadline publication. S23 receives the still
mixed, ordered board-advance effects; PIC CPU INTA and HOLD locality remain
S24. T540 remains open.
