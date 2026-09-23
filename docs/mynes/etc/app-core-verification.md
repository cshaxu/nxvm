# App/Core Design And Verification Matrix

Supporting detail for [Architecture](../design/ARCHITECTURE.md). This is the
single T4 convergence ledger and future acceptance mapping. "Design complete"
means the contracts below are present, mutually consistent and reviewable; it
does not mean their runtime tests passed. No implementation is admitted here.

## Design Coverage Universe

| ID / complete domain | Contract evidence | Design disposition |
| --- | --- | --- |
| D01 Composition/dependencies/source boundaries | [App/Core Contract](app-core-contract.md#scope-and-assembly), principal Architecture and Source Layout. | Specified; one session/executor, Core driver injection, public value-only debug exception. |
| D02 Ownership/lifetime/failure/teardown | App/Core Contract public interface and assembly. | Specified; candidate-before-swap, borrowed callback lifetime and failed-join retention. |
| D03 Commands/config/parser/startup | App/Core Contract commands/defaults. | Specified; grammar, state restrictions, busy handling, outputs, limits and exit policy. |
| D04 Lifecycle/media/reset/recovery | App/Core Contract driver and cartridge transaction. | Specified against actual Common states; guest traps paused, host ERROR restart, power/soft reset distinct. |
| D05 Debug/step/peek/poke/break/disassembly | App/Core Contract protocol. | Specified; nine bounded messages, explicit byte offsets, lease and partial-progress semantics. |
| D06 CPU instruction universe/flags | [CPU Design](cpu-bus-cartridge.md), [151-row ledger](cpu-opcodes.csv). | Specified; 56 mnemonics/151 opcodes, remaining 105 trap; binary RP2A03 arithmetic. |
| D07 Bus-cycle order/interrupt/reset/time | CPU Design and MOS-H/NES-IRQ references. | Specified; single per-cycle engine, production bus, polling and recognition gates; no transistor/sub-cycle accuracy claim. |
| D08 ROM format/mapper/RAM/storage | CPU Design and App/Core transaction. | Specified; exact strict header/length policy, PRG/CHR mapping and immutable ownership. |
| D09 PPU/scheduler/DMA | [Hardware And Presentation](hardware-presentation.md). | Architectural device contract specified, implementation belongs to M3/M4; no later task decomposition. |
| D10 Window/text output/conversion | Hardware And Presentation representation and switching. | Specified within actual 256-color and 80x25 capacities; explicit approximation and resume-time publication. |
| D11 Controller/keyboard/hotkeys | Hardware And Presentation input. | Specified; bounded held ledgers, source retirement, deterministic mapping and immutable return hotkey. |
| D12 APU/audio/pacing | Hardware And Presentation audio. | Core/host responsibilities and buffering specified; missing neutral Lib audio is an explicit future upstream dependency, not a hidden local fork. |
| D13 Persistence/compatibility scope | Hardware And Presentation persistence and Roadmap. | Architecture specified; persistent cartridge/config saving requires neutral atomic storage capability. Save states/PAL/peripherals excluded by declared scope. |
| D14 Build/test/provenance | App/Core Contract build; references; this matrix. | Specified; C11 component targets, x64/x86 and original fixtures; no external ROM/source reuse implied. |
| D15 Source reliability/owner review gate | [References](hardware-references.md), [T4 evidence](evidence/m2-app-core-design.md). | Primary/archive versus research versus product policy distinguished; final delivery stops for owner interaction. |

Completion predicate: D01-D15 each has a concrete design disposition and named
proof receiver, every S1 outstanding row is resolved by these contracts, current
authorities/proposals agree, source roots remain unchanged, and the full document
gate passes. Runtime proof is not substituted for design or vice versa. A missing
Lib audio/atomic-save implementation is a known later capability prerequisite;
it does not leave App/Core ownership undecided. The exact supported game/mapper
set of later milestones is intentionally not invented before their admission.

## Historical M2 Runtime Acceptance Cases (Closed)

| ID | Future test owner / cases | Success and failure oracle |
| --- | --- | --- |
| BUILD | test/app-mynes/unit/product, test/app-mynes/unit/core and root composition | C11 x64/x86 strict warnings; NXVM-style machine file/private-public/owner structure review per Source Layout; public-header isolation; hardware cannot include Common/platform/file/time; only composition links Core machine driver. |
| ROM | test/app-mynes/unit/core cartridge | Cross-product PRG 1/2, CHR 0/1, mirroring 0/1 accepted; 0..15 header truncations; every rejected flag/padding byte; bad magic; bank under/overflow; exact size minus/plus one; close/read/allocation failures. Old cartridge/RAM remains byte-identical after rejection. |
| MAP | test/app-mynes/unit/core bus | Every address class at start/end, all RAM mirrors, 16K/32K PRG vectors and CHR ownership; ROM write no mutation; open-bus latch; unsupported-device read/write including dummy cycles; peek has no cycles/effects and rejects whole invalid range. |
| CPU-ALL | test/app-mynes/unit/core CPU | Each of 151 matrix rows: legal decoding, length, result/flags, cycles and ordered bus trace. Reject each of 105 unlisted bytes after exactly its opcode fetch; no invented NOP or partial success. |
| CPU-ADDR | test/app-mynes/unit/core CPU | Every used mode: zero/nonzero index; page-cross/no-cross; zero-page pointer $FF; PC/address $FFFF wrap; positive/negative branches, all taken conditions; JMP ($xxFF); stack wrap at S=0/$FF; dummy reads and RMW old/new writes. |
| CPU-ALU | test/app-mynes/unit/core CPU | ADC/SBC all 256*256*2 input triples with D=0 and D=1; compare results/C/V/N/Z and unchanged flags. Exhaust byte values for shifts/rotates, BIT and comparisons with carry boundaries; transfers/TXS/store flag preservation. |
| CPU-CTRL | test/app-mynes/unit/core CPU | JSR/RTS/RTI/BRK/PHP/PLP stack bytes/order; BRK padding; reset seven cycles; warm RAM preservation and S-3; power seed; IRQ mask, NMI edge/priority, CLI/SEI/PLP versus RTI latency. Inject assertion/release at every cycle of short/branch/interrupt sequences; assert source trace gating/hijack expectations. |
| RUN | test/app-mynes/unit/core | 256-instruction/1024-cycle slice bounds; count includes instructions but not interrupt entry; callback before first instruction; normal stop, pending wake, no stale pointer after paused replacement, counter exhaustion; repeated pause/resume/rom insert/rom eject; input-reset signal and bounded resume drain through real Common. |
| DBG | test/app-mynes/unit/core, test/app-mynes/unit/product | All nine protocol requests, boundary sizes and bad version/op/length/reserved/capacity; no mutation before full validation; lease stale after reset/load/resume; STEP partial domain stop; breakpoint rearm/bypass; BRK not stolen; unsupported gameplay messages in M2. |
| APP | test/app-mynes/unit/product | Every command in each completed state plus STARTING/pending; quoted paths/extra tokens/empty paths/non-ASCII/numeric overflow/4095-byte boundary; one request per command; missing/unknown rom subcommands, missing insert path and extra eject arguments reject; failed rom insert preserves accepted media label; failed startup media unwinds before session_run; request admission never prints completed success. |
| LIFE | test/app-mynes/integration | Real Common executor with Core: startup empty -> rom insert -> RESET_COMPLETED (zero guest instructions) -> debug step -> start -> pause -> debug regs -> failed replacement preserves RAM -> reset -> stop -> start cold -> rom eject -> exit. Guest trap remains inspectable; host ERROR allows help/exit only. |
| HOST | test/app-mynes/integration | Inject each construction failure, sink delivery failure, native wait failure and failed join; retained contexts must outlive workers. No calls after indeterminate debug wait except serialized shutdown. Quit from every state restores host resources or reports terminal failure. |
| PROMPT | test/app-mynes/integration | Cooked partial-line notification, complete queued line consumed once, one reader, no worker print, no double prompt, exit without processing an extra command. |

CPU unit fixtures use production bus callbacks with a recording RAM provider;
they do not bypass the decoder or mutate private registers solely for tests.
Set up registers via production instructions/reset and assert copied observations.
Golden short bus traces and mathematical ALU expectations are independent from
production metadata. Original generated ROMs set explicit reset/interrupt vectors
and end at named state checkpoints; an iteration budget is failure containment,
not success. No executable or asset is created by this design document.

## M2 Package Proof Ownership

The owner approved five outcome-bearing packages. This table maps proof scope,
not task identifiers or another queue; Queue owns admission order. All entries
below are planned, not passed runtime results. Each admitted task instantiates
its full coverage ledger from these rows before implementation.

| Existing obligation | Initial delivered proof | Full proof receiver |
| --- | --- | --- |
| BUILD | Vertical: real App/Common/Core entry, x64/x86, private/public dependencies, strict warnings and artifact identity. | Every package preserves its own build/artifact proof; reliability verifies final product and unchanged shared standalone suites. |
| ROM | Vertical: exact accepted format, bounds, atomic candidate acceptance, basic failure preservation. | Reliability: all flag/length/mode/allocation/read/close rejection combinations and repeated replacements. |
| MAP | Vertical: full M2 address map with small opcode set. CPU/bus: all addressing forms and effects. | Cycle refinement: ordered dummy/RMW effects; debugger: non-effecting peek and RAM-only poke; reliability: retained full regressions. |
| CPU-ALL | Vertical: frozen 12 forms, remaining 244 bytes explicitly unavailable. CPU/bus: all 151 result/flag/length/cycle totals and 105 illegal bytes. | Cycle refinement: every form's ordered bus trace. |
| CPU-ADDR | Vertical: modes used by its fixed program. CPU/bus: every admitted addressing/wrap/cross functional class. | Cycle refinement: complete ordered dummy/carry/branch traces. |
| CPU-ALU | Vertical: its limited flag effects. | CPU/bus: full functional arithmetic/flags and exhaustive ADC/SBC matrix; regressions retained thereafter. |
| CPU-CTRL | Vertical: cold reset/vector and finite execution. CPU/bus: all stack/control/reset and functional interrupt forms. | Cycle refinement: all sampled pin windows, I latency, priority/hijack and reset bus traces. |
| RUN | Vertical: real executor callback, slices, stop/wake and media lifetime. | Cycle refinement: instruction/interrupt counters and timing bounds; reliability: full state/input-cleanup/failure sequences. |
| DBG | Vertical: OBSERVE for paused status. | Debugger: nine operations including explicit unsupported gameplay operations, limits, leases and breakpoint/step semantics; reliability: cross-operation failures. |
| APP | Vertical: management grammar/startup/defaults and explicit unavailable commands. | Debugger: remaining debug commands/defaults; reliability: complete command/state/error matrix. |
| LIFE | Vertical: real insert/start/pause/reset/eject/exit route. Debugger: add paused operations and inspectable stops. | Reliability: full named lifecycle script, failed/successful replacement and terminal host fault. |
| HOST | Every package: correct cleanup, bounds and propagation for every resource/path it introduces. | Reliability: full construction/sink/native wait/join fault matrix and dependency-retention integration. |
| PROMPT | Vertical: real cooked broker/session path, one reader and clean exit. | Reliability: complete queued-line/partial-edit/notification/raw-endpoint-inapplicable matrix. |

The early implementation profile is explicitly partial. Complete M2 contracts
remain the final acceptance target; early unavailable features are never silent
success or claims of full CPU/game support. A later exhaustive proof receiver
does not permit a known unsafe path in an earlier deliverable. Package closure
uses the whole admitted row set, not retirement/time budgets as success criteria.
No package consists solely of research, audit or test setup.

## Later Capability Proof Receivers

| Capability receiver | Required cases, without task allocation |
| --- | --- |
| M3 PPU/controller | Dot/register/scroll/mirroring/OAM/sprite/NMI fixtures; all eight controller bits, strobe-high/read-past-eight, opposite directions, source-specific release/rebind/stale-generation rejection. |
| M3 equal KVM | Same pixel fixture through actual Window and Core text -> Common -> Console routes; all-black/all-white/checker/color-edge patterns; viewport min/max/stride/maps; >256 colors quantization; deterministic fixed palette; Esc and Window close restore monitor; switching paused changes presentation on resume without reset/reload. Both backends block acceptance independently. |
| M4 APU/DMA/audio | Channel counters/mixer/sample counts, OAM parity and DMC overlap, pause flush, queue full/underrun, wake during host wait, slow-host rebase and elapsed-time drift. Requires an adopted neutral audio API before source implementation. |
| M5 storage | ROM identity/profile mismatch, partial write/close failure, atomic replacement, missing/damaged save and dirty-shutdown behavior. No state-stream hook claims. |

## Review Scenarios For The Owner

The design is ready for conversation using these concrete flows, not a request
to approve implementation implicitly:

1. Start without ROM, `rom insert "demo.nes"`, `debug regs`, `debug step`, `start`,
   `pause`, `rom eject`, `exit`.
2. Encounter an unsupported opcode/device; inspect the copied trap while paused,
   then power reset or replace cartridge. A genuine host ERROR requires restart.
3. Compare `reset` (power/clear RAM) and paused `reset soft` (warm/preserve RAM).
4. Select ASCII while paused, resume into raw Console, Esc back to monitor,
   select Window and resume the same machine.
5. Review deliberate first-profile limits: strict iNES mapper 0, official opcodes,
   ASCII file paths, bounded debug and approximate color/text rendering.

Owner interaction may revise any product choice. Such revision precedes further
implementation admission; design completion does not pre-approve a runtime T.
