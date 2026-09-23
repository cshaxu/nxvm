# M3 T14 S1: M2 Source, Build And Test Quality Inventory

This audits the current repository at T14 admission. It does not replace M3's
required repairs. It maps M2's closed 13-row universe to current production
receivers and retains the M3 owner for unclosed architecture/build work.

## Inspected Surface And Method

The audit inspected product-owned `src/app`, `src/core`, `test/app`,
`test/core`, `test/integration`, their CMake registrations, the root build graph
and the [M2 final ledger](m2-final-ledger.md). The declared direction is intact:
only App composition binds the Core driver; command/debug use copied protocol
values; Common and Lib cannot include product owners. The sole App Core-driver
interface include is in `src/app/composition.c`; App includes no Core
private/runtime header, and Common/Lib include neither App nor Core.

The product-owned source/test surface contains no tracked `TODO`, `FIXME`,
`HACK` or `XXX` marker. This is a review signal, not proof that unmarked defects
do not exist. The source layout remains flat under `src/core` and `test/core`.

Both `ctest --preset mynes-x64 --output-on-failure` and the x86 counterpart
passed their 101 registered tests. Registrations match: Lib 1--44, Common
45--62, Core 63--90, App 91--93 and product integration 94--101. A test count
does not discharge a row alone; the table identifies its receiver family.

## M2 Acceptance-Row Disposition

| Row | Current direct receiver family | Disposition |
| --- | --- | --- |
| BUILD | Root CMake graph; x64/x86 CTest registrations and runs | Direct current build/test evidence; fresh PE/hash proof remains R12 Presentation work. |
| ROM | cartridge, media-failure and lifecycle smoke tests | Direct parser, replacement and failure-path coverage retained. |
| MAP | bus, addressing, indirect-wrap and store-addressing tests | Direct production-bus receiver family retained. |
| CPU-ALL | opcode-profile, decode-ledger and cycle-ledger tests | Immutable protocol/decode and cycle-ledger receivers retained. |
| CPU-ADDR | addressing, branch, indirect-wrap and stack/control tests | Direct address/control boundary receivers retained. |
| CPU-ALU | exhaustive ALU, ALU, shift and logic/compare tests | Direct arithmetic/flag receivers retained. |
| CPU-CTRL | control, reset, interrupt and stack/control tests | Direct reset/interrupt/control receivers retained. |
| RUN | machine, run/step-equivalence and lifecycle tests | Direct deterministic-slice/lifecycle receivers retained. |
| DBG | debug-workflow and App debug-command integration tests | Copied paused-request boundary retained. |
| APP | command, composition and App-media integration tests | Composition remains the sole driver-binding location. |
| LIFE | lifecycle and App-media integration tests | Media and state-transition receivers retained. |
| HOST | media-failure plus Common composition/machine/wait/session tests | Storage/executor failure receivers retained. |
| PROMPT | Common monitor, queue-failure and frame tests | Cooked management/prompt receivers retained. |

## Findings And Transfer

No new product source, dependency or test-registration defect was found in S1.
T14 admission repaired two governance defects: T13 lacked its numbered history
main record, and closed T11--T13 proposals had not been moved to `history/`.
R01--R12 remain M3 obligations. R11/R12 in particular require Presentation's
complete source-style disposition, fresh release artifacts, hashes and final
native proof; this inventory does not mark them fixed.
