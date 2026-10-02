# T540 S70: board-to-Core CPU signals

## Receiving decision

Baseline is accepted S69 P2 `7c0c8c458`. S69's actual-source link proof remains
valid, but it does not authorize board code to import neutral private state.
The eighteen-header exact-path intake finds 306 consumer paths: twelve NXVM
CMake files, 57 NXVM source/header files, 236 NXVM test/support files and one
Shared negative-test file. The negative test is not an implementation consumer.
This is a path inventory, not a count of independent tests or eligible moves.

Source inspection finds four board files with direct neutral fields beyond
the current board attachment: machine_board.c, machine_plan.c,
machine_display.c and board_deadline.c. Board composition owns choices, not
the borrowed CPU/RAM/port state. Therefore the former prospective S70 physical
move is superseded before execution; its required relocation remains in T540.

The finite pre-relocation classes are:

- CPU input signals: XT PPI, planar parity and D4 NMI acceptance, plus KBC
  processor reset. S70 receives all four sites.
- Memory/port construction: high-reset fallback and plan RAM alias mappings,
  parity registration/rollback, port-registration checkpoint/status/rollback
  and board-issued port operations. Subsequent bounded receivers must use one
  neutral route/transaction owner, not expose RAM or route-table storage.
- Firmware publication: provider/context, operation guard and ROM rollback.
  Board keeps F0000h role/alias choice; Core keeps bounded invocation and ROM
  storage. Do not export the firmware context layout or add a parallel loader.
- Execution/composition: lifecycle/time observations, timing qualification,
  bus-ready inputs and provider/attachment binding. Copied observations and
  typed operations replace private reads; no mirrored timing/lifecycle state.
- Direct tests and build receivers: same-owner private Core tests can follow
  Core; board fixtures remain with their actual owner. Do not move all 236
  paths merely because they include a candidate header.

## Complete signal-class implementation

Core exposes two executor-thread operations on its opaque machine handle:
NMI acceptance and processor-reset signaling. NMI still calls the sole CPU
mask/pending-latch mechanism; false means masked or null machine. Processor
reset sets only the existing CPU-owned request, consumed by the existing run
boundary before retirement. Neither operation advances time, resets devices,
creates a host lifecycle request, nor establishes cross-thread synchronization.

All three board NMI sites use this acceptance result. Existing electrical
latches are updated only when CPU acceptance succeeds. KBC's existing reset
callback now uses the neutral signal instead of borrowing the CPU context.
Original board predicates, CPU implementations and reset priority are unchanged.

The existing independent smoke adds null input, masked/unmasked NMI and
processor-reset checks. It proves reset consumes zero instructions, returns
the reset PC, and preserves elapsed time, RAM and the installed port provider.
Existing XT/parity/D4/KBC behavioral tests remain intact. The existing controller
authority gate scans every NXVM production C file and rejects raw CPU NMI/reset
calls outside the neutral machine implementation. CPU implementation and
same-owner CPU tests are intentionally not forbidden.

Five source/test/gate paths add 49/remove seven lines, net +42, by Git numstat.
The increase pays for two required opaque signal operations, their explicit
lifetime/thread contract, runtime regression and whole-product prevention
scan; it creates no new state or production execution route.

## Verification status

Independent Debug execution and complete receiving units pass on x64 and
x86: 470/470 each, in 271.00 and 67.92 seconds respectively. Both-width
specialized gates, exact 37-edge dependency inventory, documentation and diff
checks pass. Receiving logs are `build/s70-{build,unit,gates}-{x64,x86}.log`.
All eight rebuilt Release products pass the independent link/run proof. PE
architecture is checked by their build deployment and no product contains
compiler `.debug` sections. Each matching Release boot probe ran once using
the unchanged owner INI and external overlay media: default x64/x86 reached
`dos-prompt`; XT, AT and Model40 x64/x86 reached `installer-running`. All
eight returned zero. These are scoped headless boot checkpoints, not a claim
of native UI manual verification or completed T540 integration acceptance.
Logs are `build/s70-<profile>-<width>-{build,neutral,boot}.log`; default's
independent proof is included in its build log. The first XT x64 Makefile
invocation regenerated an old target table, then could not dispatch the new
proof target in that same invocation. A fresh invocation built and ran it;
no production change or repeated boot was used to bypass this build issue.
Shared, MyNES, owner INIs and external masters remain outside the change scope.

## Release SHA-256 identities

All files are under `assets/nxvm/<profile>/`, version 0540.

| Product | Width | SHA-256 |
| --- | --- | --- |
| default | x64 | 8B58BFC872EBF0D6FDC9D871556E323A3725E12435068D6C2E90226A084DD0D1 |
| default | x86 | 9B368DCA3816F55043C40232CBA19D09B0A34723052BD640AAAB1D845F55227A |
| XT | x64 | 120F03CAF18E47FE13F1143519961A77BF30EFB6D1B692683B018DBB2E7CC9E8 |
| XT | x86 | 56DFD8FDAD89B91266A8A8E85C52AE3F3211EF83ECAB391F85349EE2861A125B |
| AT | x64 | 878CD091EA4163322E7E20F0C1B3ECDF9797CC37BBFCCAA79A4DB2250015AD14 |
| AT | x86 | 4963EC6BDFBA02BD711385E565F3CF228B429DDE69F4D62E87A95A9AC7FD5C55 |
| Model40 | x64 | 4044F87A888C2B4FDF57B161A5F882D0413D3FA90F5B30DA1C7A70C7B130032F |
| Model40 | x86 | CF74B2DD95556E79EAD2C76D49D2AADD2322302AB8919FAA3D7DBE06E672318C |

## Coordinator review

Actual pushed P1 `ba659381f` was reviewed across all eighteen changed paths,
including the five code/test/gate paths, governing intake changes and eight
artifact identities. The three NMI predicates and latch-on-accept behavior
are preserved; KBC reaches the existing CPU reset request and processor-only
run consumer. CPU sources and run priority have no diff. All requested
S70 outcomes map to the evidence above; the new prevention scan covers all
production NXVM signal callers. Shared six components, MyNES and owner INIs
are unchanged. S70 is accepted; physical relocation and board extraction
remain required open T540 work, not hidden behind this receiving proof.
