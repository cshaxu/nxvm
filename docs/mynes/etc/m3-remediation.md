# M3 Architecture Remediation Ledger

Supporting [Architecture](../design/ARCHITECTURE.md). On 2026-09-21 the owner
explicitly required M3 to correct M2 work that violates the accepted architecture
and boundaries. Baseline inspected: `2f6126d`. Historical M2 closure records are
retained; they do not waive these observed differences or establish their repair.
The table records the baseline finding and its designated final owner. T11 S1
has implemented the Vertical foundation for R01-R05/R08-R10; its direct evidence
and any deliberately transferred qualification are recorded in
[T11 history](../history/M3-T11-interactive-dual-kvm-vertical.md). A row is not
milestone-closed until its listed final owner accepts all required proof.

Candidate owners: **Vertical**, **PPU**, **Input**, **Presentation**, corresponding
in order to the four proposals in [Queue](../states/QUEUE.md). The queue alone
owns admission order. Each receiving proposal consumes its entire listed batch.

| ID | Baseline finding / affected mechanism | Required result | Owner and direct acceptance |
| --- | --- | --- | --- |
| R01 | app/command.h holds core_driver; command.c calls core_driver_has_cartridge | Only composition holds the driver; command uses accepted media facts and Common requests, debug uses value-only ABI | Vertical: production load/eject/failure transitions and forbidden include/call checks |
| R02 | app/CMakeLists.txt exports Core driver dependency and broad source includes | Separate composition linkage from command/debug dependencies; private hardware headers cannot become frontend APIs | Vertical: consumer header compilation, dependency graph and forbidden include checks |
| R03 | App embeds a second 256-entry opcode description; public debug value header is absent | One immutable opcode description owned by Core, shared by CPU decode and copied-value disassembly; no direct runtime call to hardware | Vertical: all opcode forms and illegal forms checked against declared profile, no duplicate production table |
| R04 | composition.c uses stack callback context and destroys dependencies after failed shutdown/UI destruction | Heap-owned composition; quiesce producers before sinks; retain full borrowed graph on failed join, report terminal failure | Vertical: every construction stage and machine/UI join failure, live callback lifetime checks; Presentation verifies native teardown |
| R05 | command.c permits oversized paths and lacks complete pending intent; config.c lacks specified help grammar | Implement 1023 printable ASCII path / 4095 line limits, quoting, pending completion facts and startup grammar | Vertical: real accepted/rejected media transactions and retained identity; Input completes all command/state combinations |
| R06 | input reset/wake latches are not consumed; runtime sink does not reset mapped input | Executor consumes signals; composition resets before completion facts; no stale make survives pause/retirement | Vertical: basic pause/resume route; Input: complete source/generation/drain ledger |
| R07 | bus.c computes time from cycles plus bounded trace count; CPU increments totals at instruction end | One slot clock independent of diagnostics; hardware-owned PPU/IRQ/NMI wiring, no public mutable cycle-hook bypass for frontend/test-only alternate execution | Vertical installs production clock; PPU proves CPU/dummy/interrupt/reset/DMA slot coverage and trace saturation |
| R08 | machine_create publishes output before reset success; reset uses prior state and zeros elapsed counters for warm reset | Construct before publish; failed create returns null and releases allocations; explicit deterministic power state and warm reset preserving elapsed time | Vertical: construction/reset failure ownership; PPU: seven reset transfers, MMIO prior-PC cases, NMI and device reset policy |
| R09 | machine_peek writes output before later unsupported address fails | Prevalidate complete range or stage copied result; unsupported MMIO has no output/device side effects | Vertical: RAM-to-MMIO and wrap/boundary failures leave sentinel output and state unchanged |
| R10 | cartridge CHR RAM allocation is not initialized; storage fault evidence does not exercise every actual acquisition boundary | Initialize volatile cartridge memory; transactional allocation/read/close failure preserves accepted machine and frees candidate | Vertical: CHR RAM contents and real fault injection at each acquisition/read/close site, not only mocked return categories |
| R11 | App/Core compressed statements and ownership declarations diverge from adopted NXVM-style target | Adjacent public/private headers, explicit bounded control flow, owner prefixes, no duplicate path; flat src/core and test/core | Each implementation owner fixes its changed mechanism; Presentation reviews entire product-owned source against Coding and records remaining disposition |
| R12 | Product version/build options and old executable counts are insufficient for current delivery proof | Fresh admitted-task artifacts, product strict warnings, required stripping and complete registered CTest/static/shared gates | Every implementation delivery; Presentation supplies final x64/x86 PE identities, hashes, gate results and shared manifest equality |

The coverage universe is all product-owned App/Core source, their tests and root
build integration against the accepted Architecture, Coding, UX and detailed
contracts. The rows above seed that finite review; they are not permission to
ignore another violation in those components. Each implementation admission
records affected files, callers, state/failure variants and owning row. Newly
found variants enter the same mechanism batch; independent findings receive an
explicit row and owner before closure. Existing M2 functional regression remains
required while the mechanism changes. No audit-only task substitutes for fixes.

A row closes only with the production change, direct positive/negative evidence,
regression owner and actual-diff review. Test filenames, old pass counts and
documentation checks alone cannot close it. Final M3 acceptance requires every
applicable row accepted; blocked rows block M3. Scope removal requires explicit
owner approval, never silent transfer to audio/compatibility work.

The four shared roots stay identical to the adopted snapshot. A discovered
shared prerequisite blocks its dependent outcome and requires reviewed upstream
adoption; it is not permission to patch Lib/Common locally. M4/M5 are not planned
here. No source import or third-party rights change occurs in this ledger.
