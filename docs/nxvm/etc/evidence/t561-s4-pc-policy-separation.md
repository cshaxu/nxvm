# T561 S4 PC Policy Separation

## Scope And Ownership

Baseline is accepted S3 `1df04f40d`. This consumes I3's P2/P3/P5 prerequisite,
not neutral Machine extraction or complete T561 acceptance. Architecture
governance fixes the sole table/default/trace owners before coding governance
applies names and repairs receivers. Lib, Emulator, all imported packages,
the new CPU/FPU package and MyNES implementation/assets are unchanged.
Original CPU source/timing grades, closed production 486SX admission and the
T560 C16/C17/final-qualification successor remain intact.

### P2: Whole-PC Qualification

The 30-item declaration capability/disposition/seam contract moves out of the
neutral machine header into `core/board-base/timing_interface.h`. Its names
qualify the actual Board owner. Values, item order, grades, seams and complete
table validation do not change. The copied table and its once-only flag move
from Core storage to Board state; the existing plan owns validation, install
and getters. No table copy, manager, allocation or new persistent state is
introduced. Construction failure still publishes neither output, failed
installation changes no entry, and reset retains the installed table.

All direct receivers now pass their real Board handle rather than ask Core
to inspect PC data. The neutral deadline callback loses its qualification
argument; the Board deadline owner reads its own installed flag. Core receives
only the existing copied deadline observation. Neutral CPU retirement source
eligibility and physical-time admission remain Core-owned and unchanged.

The former neutral-link table test follows its actual Board owner into the
existing board timing qualification smoke. Its reversed order, malformed and
duplicate entries, failed publication, copied input, second-install refusal,
freeze and reset assertions are retained. The old direct write to private
Core firmware-active state is replaced by an actual synchronous firmware
callback attempting the same prohibited installation. It exercises the real
reentry guard without a new production API. Board deadline tests now inject
their own private Board qualification state, not a synthetic Core marker.

### P3: Producer-Owned Trace Identities

Seven PC device tags move into `core/board-base/trace_interface.h`, preserving
the existing values 9--13 and 17--18. Core's neutral CPU/bus tags, including
DMA 8, retain their original numbers; explicit 14 and 19 preserve the gaps.
The existing copied event ID and recorder parameter use `lib_u32`, so a board
can emit its own ID without a reverse include or pretending it is a Core enum.
The supported ABI width, timestamp, sequence, single buffer, event call sites,
emission order and compile-time Release no-op are unchanged.

Mixed-domain test consumers use the same scalar envelope. Existing arbitration,
timeline, input/display, storage and transaction tests retain their ordering
checks. The arbitration test additionally fixes the numeric identities of
the split events and the first/last neutral transaction identities. No new
trace renderer, registry, queue, worker or recovery path is added.

### P5: Explicit Neutral Inputs, PC Defaults And Resize Admission

Board preparation alone resolves DEFAULT CPU to 80386 and zero RAM to 16 MiB,
exactly as before. Neutral creation now requires an explicit admitted CPU and
nonzero allocation size. The existing production 486SX gate is not opened.
The Core allocator already supports any nonzero backing size; no artificial
minimum is added. A new 64 KiB allocation / 16 KiB resize regression demonstrates
that this is a real generic capability, not a PC-sized fixture shortcut.
It does not claim the full non-PC execution proof reserved for S5.

Only stopped PC RAM replacement retains the existing 2--64 MiB limits through
the existing Board memory-admission callback. They are not newly imposed on
PC construction or the generic allocator. Parity/mapping/lifecycle rejection
and failure-before-allocation semantics remain intact. The Board test proves
the actual default CPU/RAM and unchanged rejected resize boundaries; the
neutral resize case now accepts a valid 1 MiB allocation rather than enforcing
the former PC limit. No alternative allocator or configuration object is added.

The receiver audit finds 55 initializer cases in 29 files that previously
relied on implicit 16 MiB/80386 values. They receive those same explicit values,
unless their existing later assignments already supply them. The Board-prepare
output object is not mistaken for an input. Negative qualification cases get
otherwise valid CPU/RAM so they still reach their intended failure condition.
Ordinary test allocation sizes preserve their old literal values without
importing PC policy headers; genuine PC-boundary assertions keep the owned
constants. Existing six opaque-executor helper callers explicitly request
their prior 16 MiB fixture size.

## Similar-Issue Sweep And Gates

The family authority verifier now permits qualification state only in the
Board plan/deadline implementations, and rejects PC table/default/trace data
throughout neutral C/H files. It checks the validator at its new owner rather
than disabling the old ownership assertion. Related obsolete board-common and
pre-split App path predicates are corrected to the actual Board-Base/XT/AT and
all four current App paths. Public Core READY, firmware, mapping and time
owners retain their previous checks. PC tag ordering checks use the new names.

Initial compile failures identify an ignored obsolete callback argument and
test setup borrowing the moved PC constants. The complete initializer/receiver
sweep repairs that class before subsequent verification; no production fallback
or new type alias is used. The first full gate failure correctly rejects the
new table location under its old owner assertion; the verifier is updated to
enforce the actual new owner and reject the old one. These attempts are not
counted as qualification passes.

## Actual-Change Accounting

Count ordinary `git diff --numstat` against `1df04f40d`, including the two new
headers and excluding documentation, manifests and binary artifacts. No file
rename occurs in S4. C/H counts include source receivers and all four Apps'
test receivers, including mechanical policy-name-to-nominal-size changes.

| Surface | Files | Added | Deleted | Net |
| --- | ---: | ---: | ---: | ---: |
| Production C/H | 15 | 282 | 266 | +16 |
| Test C/H | 151 | 602 | 460 | +142 |
| Family CMake checks | 2 | 20 | 8 | +12 |

The two new public Board headers expose relocated qualification declarations
and a Board-owned enum grouping the seven existing PC trace IDs. They add no
ID, execution operation, state manager or allocation. Four existing table
operations move and take their real Board receiver; no old-name alias remains.
The two trivial default resolver functions disappear. Test growth reflects
explicit prior inputs, real firmware-reentry proof and new neutral/PC boundary
assertions, not a second test framework or repeated instruction suite.

[Frozen file hashes](t561-s4-file-review.tsv) account for all 170 changed
source/test/CMake/manifest files. The independent byte comparison verifies the
complete 30-item declaration block after owner-name normalization, all 28
trace numeric identities across their two owners and all three original PC
memory constant values. This supplements, rather than replaces, actual code
review of the moved state, guards, callback boundary and test routing.

## Frozen Delivery Verification

The six focused x64 tests pass after the complete implementation: neutral link,
plan, scheduler, display authority, Board timing qualification and RAM resize.
The complete x64 run passes all 627 repository units and the specialized-gate
aggregate; log `build/t561-s4-x64-proof.log`, unit duration 244.78 seconds.
Package manifests, family DAG, documentation governance and whitespace checks
pass. The complete x86 run passes 627/627 repository units in 91.57 seconds
(`build/t561-s4-x86-final-units.log`). Both final specialized-gate aggregates
pass, including the strict-compilation matrix (510 strict rows and three
explicit retained exceptions); this is not a claim of exception-free compilation.

| Receiving integration | x64 | x86 |
| --- | --- | --- |
| Default PC/AT, including 12 MyNES cases | 35/35, 66.67 s | 35/35, 68.38 s |
| IBM 5160 | 3/3, 32.65 s | 3/3, 35.16 s |
| IBM 5170 | 5/5, 43.95 s | 5/5, 54.29 s |
| DeskPro 386 Model 40 | 5/5, 138.14 s | 5/5, 185.29 s |

Each width therefore passes 48 receiving executions. Both original graphs
are restored to default, with an additional passing restored-profile identity
check. The frozen functional x64 implementation is not subjected to a second
complete unit run: its final nine manifest/affected-contract rechecks pass in
5.72 seconds after the public-header comment clarification. Actual published
INFO/SPEED/HELP/EXIT/INI checks pass for all four Apps per width. Logs use
`build/t561-s4-<width>-<profile>-integration.log`,
`build/t561-s4-<width>-final-*.log` and
`build/t561-s4-<width>-<app>-published-cli.log`.

Eight rebuilt optimized/stripped 0.5.0561 EXEs are deployed in the existing
four Assets roots. Final PE width, embedded version, two-per-root count and
SHA-256 verification are recorded below. Imported ten trees, X86 Core CPU/FPU,
Lib/Emulator, MyNES, owner INI and other input assets have no change against
the admission baseline. No source/ROM import or alternative build graph is
introduced. I3 neutral execution, I4 consolidation and I5 final task acceptance
remain open; no partial result is presented as T closure.

### Published Artifact Identity

| Path | PE machine | Bytes | SHA-256 |
| --- | --- | ---: | --- |
+| `assets/nxvm/nxvm_0_5_0561_x64.exe` | 0x8664 | 1512515 | `d8e87edc50da3d1c8acb1b3a5272c0c6418f625044b9118868c6633d647905fc` |
| `assets/nxvm/nxvm_0_5_0561_x86.exe` | 0x014C | 1658220 | `20ecb5bd6b7a3d016844a6be1042ee71394457ce90253715532c922c5c42176a` |
| `assets/my5160/my5160_0_5_0561_x64.exe` | 0x8664 | 1501493 | `2205e5122850b68cc8205414f6d4c2af117eeed4dd3bbe77b3d970fa8bae18b5` |
| `assets/my5160/my5160_0_5_0561_x86.exe` | 0x014C | 1645667 | `487bd1de26ac31fe31c648067266a739fb813bf109b01b439e6c6c4f0e53e22c` |
| `assets/my5170/my5170_0_5_0561_x64.exe` | 0x8664 | 1513345 | `c4b69f19881aec26ebba6e94b46e2c5a197d0b6b4d677ba163af50fee3b5404c` |
| `assets/my5170/my5170_0_5_0561_x86.exe` | 0x014C | 1659434 | `d97cec120d2c8b4e484e87d6215a1c76f7b178c2d897be465b67a66a507bd64b` |
| `assets/mydeskpro386/mydeskpro386_0_5_0561_x64.exe` | 0x8664 | 1500585 | `34bb06a0b9be5af76c53ef006055d8242224817f30493382472ba17471cc7869` |
| `assets/mydeskpro386/mydeskpro386_0_5_0561_x86.exe` | 0x014C | 1646169 | `40d81e3a7e399e39e8b2146a4e511c3fad5bfb683210bce703cb3f26d40e73d0` |

Manual RDP, physical timing, full interactive Windows GUI, PC110 or new CPU
admission are not claimed. Existing automated desktop-labelled cases are
reported separately from those unexecuted manual/external qualifications.
