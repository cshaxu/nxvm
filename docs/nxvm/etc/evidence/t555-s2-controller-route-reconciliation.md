# T555 S2 Controller Route Reconciliation

## Status And Baseline

S2 implementation is ready for coordinator review against `710e0da4f`. This record supplements, rather than
replaces, the S1 coverage universe. It is not complete controller qualification.
The final PIC/PIT/DMA and board/refresh/request candidate passes complete units
on both widths, and eight current PC artifacts are rebuilt. Final integration
passes all four selected Apps on both widths. DeskPro x86 reaches the original
checkpoint in 179.93 seconds. Earlier candidates missed that 180-second checkpoint;
their failures remain recorded below. No Shared, MyNES, firmware, media or
owner INI has changed. Earlier pending statements below describe chronological
candidates, not final qualification.

The actual original-manual archive is `manuals-nxvm`, not the historical
`manuals` spelling in S1-era authority text. Originals stay external; ignored
review PNGs are below `build/t555-manual-review` and are not product inputs.

## Primary Source Review

- Intel 8254, September 1993, order 231164: printed pages 9-16, mode
  definitions, null-count operation and Figures 15-20. Pages 10 and 16 were
  rendered and visually inspected, as was page 15's mode-4 definition.
  SHA-256:
  `E5397A387D24FFF21C8B531A320251F89B103274BBCE0C057A654873E494D726`.
- Intel 8253/8253-5, November 1986, order 231306: Figure 7, printed 3-56,
  visually reviewed from archive PDF page 6. It shows the trigger-started
  one-shot and hardware strobe continuing after a falling GATE. SHA-256:
  `5E5B5737ED82D244682C9E768098F910326697DE9589DFA09EE2ABE6030C8DB5`.

Manual-derived behaviors below are independently expressed in project-owned
tests. No source from another emulator is copied or transliterated.

## PIT Confirmed Defects And Repair

| Row | Before | Owner-local change and proof | Current disposition |
| --- | --- | --- | --- |
| Mode 1 trigger/load | Pending CR was committed on the next clock even without a trigger. A coincident write/trigger required another clock, and a rewrite stopped the active one-shot. | `x86_pit_tick_mode1` loads the pending CR only when consuming the existing trigger. Without a trigger the active CE continues to terminal count; no new state. | Repaired; both personalities' regression passes, complete qualification pending. |
| Modes 1/5 low GATE deadline | Tick handlers continued an active pulse while deadline rejected all low-GATE counters. | `x86_pit_ticks_until_output` ignores the level gate only for these trigger modes, matching their existing CE progression. | Repaired; low-GATE count, terminal output and mode-5 pulse completion proven. |
| Pending CR event eligibility | Any pending CR returned one tick, including untriggered modes 1/5 and active modes 2/3 waiting for their period/half-period boundary. | Deadline uses current mode/active/trigger/restart facts. Immediate software loads remain modes 0/4 and initially inactive modes 2/3; active periodic rewrites keep the current OUT deadline. | Repaired; armed-inactive and active-rewrite deadlines proven. |
| Mode 4 strobe/GATE | A falling GATE during the terminal low pulse stalled its return high and removed its deadline. | Tick and deadline permit the existing one-clock pulse to complete even with GATE low, while low-GATE counting still pauses. | Repaired after its own failing runtime negative control; both personalities pass. |

There is still one chip timer, one CR/CE state and one output callback.
`core/board-base/board_deadline.c:board_consider_pit` consumes that same
deadline for the main and auxiliary PIT, converting through the board clock.
No board fallback or new scheduler is introduced. This call-path inspection
does not substitute for the remaining direct board-deadline regression.

## Reproduction And Verification So Far

The added `pit_waveform_smoke.c` pending/trigger cases failed on the unchanged
production implementation (exit 1). The first compile attempt used a nonexistent
array-count macro; it was corrected to the existing sizeof idiom before the
runtime negative control. After the owner repair:

- x64/x86 builds in the original `mingw-gcc-*-release-ccache` trees succeed.
- Direct execution of `pit_waveform`, `pit_readback`, `pit_contract`,
  `pit_8253` and `pit_ports` returns zero for all ten executions.
- The waveform receiver covers these new cases on both 8253 and 8254.
- All five focused PIT/port CTest routes pass on both widths (5/5 each).

No full unit/integration run, desktop verification or artifact update has yet
been performed for this candidate. The S must not close or be pushed as a
complete P until its full packet is satisfied.

## PIC Board Signal And Poll Repair

Intel 8259A, December 1988, order 231468, printed pages 17-18 and Figure 9
define edge versus level recognition. Page 17 was visually inspected.
SHA-256: `71F62B2FA1F13F6547DCEE26A2D570BF9BCFCFE6F2F84B24BC01053BB6153533`.

`pic_bus` previously supplied an asserted-request edge for every newly asserted
source lease, even when another source kept the same physical IRQ high. The
chip correctly trusted its board input; the error belonged to aggregation.
The board now emits an edge only on its existing assertion count's zero-to-one
transition. The regression covers master IRQ6 and slave IRQ14, a second source
asserting after acknowledgement, deassert/reassert while the peer remains
asserted, and a real new edge after every source releases the line.

Slave poll reads also acknowledge its request, but the read adapter did not
refresh the dependent master cascade. The read path now calls the existing
refresh after slave reads, matching the slave-acknowledgement path used by
`get_interrupt`. Master poll remains separate: an initial unconditional
refresh reintroduced its already-acknowledged cascade before slave polling;
the existing OCW3 test caught that and the candidate was narrowed at the
actual dependent boundary. This intermediate failure is not a passed result.

Both added receivers fail against unchanged `pic_bus`, and both pass after
the final repair on x64/x86 (2/2 each). Initial attempts used nonexistent
target/executable spellings and are excluded from runtime evidence; the
actual negative controls ran the rebuilt `test/core` executables and each
returned 1. No interface, chip state or second request queue is added.

## DMA Count, EOP And Master Clear Repair

Intel 8237A, September 1993, order 231466, printed pages 7-9 define
count-minus-one programming, external EOP, TC status and Master Clear.
Pages 7 and 9 were visually inspected; page 9's text was rechecked after
the reset repair. SHA-256:
`2A1AC5BBF5B4BE75BA9272647EFFB389BCC5E76CBFE293D5C56E75217E2BC8A8`.

The chip treated an initially programmed FFFFh count as an already completed
transfer. Demand, block and memory-to-memory consequently skipped a valid
65,536-transfer request. The existing EOP latch now records the actual
zero-to-FFFFh decrement, rather than interpreting the count value alone as
completion. Single mode also honors an EOP already present before its service.
TC status is set for external EOP independently of auto-initialization, as
required by page 9. No state or public interface was added.

Master Clear previously erased all chip data, including the address/count
and mode registers that the specified reset does not clear. Reset now clears
only the control/service fields. First construction remains zero-initialized
by the existing allocator. The board port adapter now forwards Master Clear
to that sole chip operation instead of using whole-board reset and erasing
independent page latches. Explicit board reset retains its existing page-reset
responsibility; no second chip reset API was introduced.

The chip receiver exercises 65,536 actual services in each ordinary transfer
mode, the first memory-to-memory service from FFFFh, external EOP before
service in all three modes with/without auto-init, and preserved programming
after Master Clear. The port receiver proves page retention on primary and
secondary Master Clear while explicit board reset still clears the pages.
Both reset receivers failed against the preceding implementation and pass
after the repair. Earlier maximum-count/EOP receivers likewise failed against
the unchanged chip before its repair.

Relinking the board channel receiver exposed one stale expectation that
external EOP with auto-init leaves TC clear. It was corrected to the manual's
explicit TC requirement; its existing data/count/mask checks remain. The
memory-to-memory comment was also corrected to match its existing TC assertion.
The final rebuilt board channel receiver passes on x64/x86. The selected
11-route x86 DMA run passes, but some App deadline executables have not yet
been relinked for this candidate and are not accepted as final regression.
The x64 run before correcting the stale oracle passed 10/11; the subsequent
selected rerun passes 11/11. These focused counts still include the noted
not-yet-relinked App receivers and do not prove final regression. No artifact
or full-suite claim is made from these results.

## PIC OCW2 Priority Repair

Intel 8259A printed pages 15-16 distinguish the auto-EOI rotation latch
(set/clear OCW2 operations) from specific/non-specific EOI and priority
operations. The implementation instead stored every last OCW2 command in
the same field used to determine auto-EOI rotation. A plain EOI inadvertently
disabled rotation, while Set Priority inadvertently enabled it. Specific
rotate was also ignored when ISR was empty, although its priority operation
is independent of whether the specified service bit is currently set.

The existing field now retains only the set/clear auto-rotation command;
other operations use their command argument directly. Specific rotation
clears its named ISR bit and changes the priority base without an ISR-wide
precondition. The existing command receiver tests enabled rotation across
plain EOI, disabled rotation across Set Priority, and specific rotation with
empty ISR. It fails before repair and passes after repair. Chip contract,
commands, board OCW3 and IRQ-source lifecycle pass 4/4 on each architecture.
No public contract, state field or additional command path is introduced.

## Batch Reconciliation Worklist

### PIC Initialization And Held-Level Reconciliation

Intel 8259A printed pages 9 and 13 require the initialization sequence before
normal interrupt selection. The chip previously selected pending inputs while
still expecting ICW2/3/4. Both controller-local poll selection and the public
master/slave selection now use the existing initialization status. Failed
selection preserves the caller's output. No second initialization flag is used.

An ICW1 restart clears the old in-flight IMR-release delay but retains the
configured per-line timing inputs. The receiver restarts initialization with a
three-tick release still pending, checks each incomplete phase, then proves a
new request is immediately eligible. A later mask/unmask still receives those
same three configured ticks. This is lifetime reconciliation of the existing
L2 timing model, not a claim that Intel specifies that delay.

Printed page 18 requires a still-high level request to recur after EOI or
AEOI. Acknowledgement previously erased IRR and relied on a later board input
refresh to restore it. The chip now retains the request while its already
owned input level remains high; ISR still blocks it until EOI where required.
The direct receiver checks normal EOI, AEOI and final deassertion without an
extra input-refresh call. Its pre-repair execution exits 1; after repair the
chip contract/command and board OCW3/IRQ-source receivers pass 4/4 on each
width. Board callbacks and all public interfaces remain unchanged.

After relinking the complete `core-board-tests` receiver aggregate, the current
31-route PIC/PIT/DMA/clock/arbitration selection passes 31/31 on each width.
This includes the freshly repaired chip and existing CPU/PIC delivery,
source-reset and cascade receivers; it is still not full repository or
four-App external qualification. Core source/test manifests, documentation
governance and diff whitespace checks pass for this candidate.

### XT Versus AT Refresh Wiring Finding

Original IBM XT Technical Reference, April 1983, System Board description,
assigns PIT channel 1's dummy DMA refresh to DMA0; its transfer takes four
processor clocks. SHA-256:
`18CD473851FDFE40C5BF2C7CCC870772857D84509D95ADA7880473CD802C63B8`.
IBM AT Technical Reference, March 1984, System Board 1-7 (PDF page 25) and
RAM/DMA description 1-12 (PDF page 30), instead describes a separate 6-MHz refresh
controller, five-clock refresh cycle and seven usable DMA channels. DMA0 is
not the XT-style refresh channel. SHA-256:
`F6CB4A270E0F49A79940FA1EE1084CA419AF15B463C870D652EA74CED011EA0D`.
These are original archived text extractions; exact selected-board schematic
and later 8-MHz model reconciliation remain pending, not visually qualified.

Current `core_machine_configure_dma` unconditionally binds DMA0's refresh
provider and PIT1 output, while `core_machine_board_after_pit_reset` restores
that binding unless a profile owns refresh output. My5170 does not supply such
an override; Model40 does. This is a confirmed topology mismatch for AT,
not an 8237 counter defect or a reason to alter XT. Its repair must use the
existing Core refresh-request/arbitration boundary with one board signal owner;
the selected AT duration and address semantics still require source/code
reconciliation before implementation. No new API or synthetic timing constant
has been introduced to cover it. The earlier description of PIT1-to-DMA as
a physical 5170 route is rejected by this source finding.

#### Selected Construction And Receiver Reconciliation

The archived page 1-12 explicitly assigns DMA0 to `Spare`, DMA2 to diskette
and DMA4 to the cascade. It states that PIT1 requests a memory cycle every
15 microseconds. Page 1-7 independently gives five refresh clocks and five
DMA data-transfer clocks, on different 6-MHz and 3-MHz clock domains. Equal
cycle counts do not make these the same request or timing route.

The current construction order in `board-base/machine_plan.c` admits planar
Port B or a profile binding before DMA wiring. `pc_at_profile.c` materializes
Port B for both My5170 and default, even when memory parity is absent. Thus
neither a CPU-family test nor the descriptor's memory-parity flag is a valid
refresh-topology selector. XT has its single-controller DMA route; Model40
already supplies its own frozen refresh binding. The selected composition,
not the BIOS or current instruction, must determine the route.

`core_machine_dma_rtc_authority_smoke.c` currently proves PIT1 low/high
drives DMA0 and asserts that DMA0 cannot accept another provider. Its fixture
has two DMA controllers but no AT Port-B construction. This receiver proves
the existing generic DMA route, not the selected IBM AT topology. A passing
result cannot qualify My5170's refresh routing. A repaired receiver must
separately prove XT's reserved DMA0 and AT's independently serviced refresh,
including DMA0 availability, DMA masks, reset and PIT-output rebinding.

Core already has one attachment `refresh_request` / `refresh_complete`
boundary and one REFRESH transaction owner. Its current service commits and
releases in the same arbitration callback. It does not charge the documented
five refresh clocks, and `core_machine_transaction_contract` exposes only
DMA wait quanta. Reusing that point-service path fixes neither the AT cycle
duration nor its competition with DMA by itself. Adding a second scheduler
or borrowing DMA's wait counter would conceal this missing contract.

The March 1984 source is for the 6-MHz system. My5170's current Rev-3 choice
uses an 8-MHz model axis. The 6-MHz page proves the topology conflict but does
not authorize copying its clock ratio into the selected 8-MHz declaration.
The subsequent March 1986 acquisition below supplies the missing later-board
authority; integration of that source into the timing contract remains open.
The readable Port-B bit-4 Other-L2 observation is a separate retained fact;
it cannot supply the missing refresh arbitration duration.

#### Later AT Source, Acquired And Visually Reviewed

On 2026-10-11, acquired IBM Personal Computer AT Technical Reference,
March 1986, through the original-manual scan at
<https://www.minuszerodegrees.net/manuals/IBM/IBM_5170_Technical_Reference_MAR86.pdf>.
The original is retained outside the repository as
`manuals-nxvm/platform/ibm-5170-technical-reference-mar1986.pdf`, SHA-256
`3224C26B424A4745C765F56AF7D957D3C4AD959E0F719EBEB39540A2508E95D5`.
No firmware, media or third-party implementation was acquired or imported.
Page renders are ignored task inputs under `build/t555-manual-review`.

Full visual review of printed 1-7 (PDF page 22) confirms that the manual
distinguishes 6/8-MHz CPU clocks and 3/4-MHz DMA clocks. It specifies eight
CPU clocks per refresh, not the early edition's five. The selected 8-MHz
board therefore supplies a source-backed DMA ratio of 1:2; the existing
3:8 ratio cannot be qualified by this later-board page. Any retained model
axis must be declared separately rather than called that physical ratio.

Printed 1-22 through 1-24 (PDF pages 37-39) describe PIT1 as the refresh
generator. Visual review of the complete timer diagram on 1-23 confirms
that OUT1 clocks a D flip-flop with its D input tied high; active-low
REFRESH clears it. The request is latched across OUT1's subsequent change,
not a DMA DREQ level that simply follows the low pulse. The pictured clock
input has no inversion bubble, so the rising OUT1 transition sets the latch.
Reset/control-write transitions and request coalescing need their own
receivers; changing only the channel number would miss this behavior.

Printed 1-10 (PDF page 25) lists a separate refresh page register at 008Fh.
The current DMA bus owns its readable/writable page byte, while Core's
existing refresh-request address is only eight bits and represents a row
cycle rather than a normal byte transfer. Qualification must preserve that
register owner and explicitly distinguish row refresh from RAM reads;
it must not emulate refresh by an arbitrary memory read.

This evidence changes the next repair design: latch and completion-clear at
the board owner, independent REFRESH arbitration with declared duration,
and selected-board clock reconciliation. XT's DMA0 route and Model40's
profile-owned route remain separate, source-backed contexts. No production
change or timing-accuracy upgrade has yet been claimed for this finding.

### PIT Terminal Count And Strobe Reconciliation

Intel 8254 printed pages 11, 12 and 16, Figures 15, 16 and 20, show CE
continuing from zero through FFFFh after the single OUT sequence finishes.
The selected 8253 mode descriptions retain the same countdown semantics.
The old implementation instead left modes 0/1/4/5 frozen at zero. The new
terminal-count receiver fails on that candidate and passes after the repair.
It covers both personalities, binary/BCD wrap, GATE-controlled versus
triggered counting, and no second OUT sequence through a full counter wrap.

Modes 4/5 also retain exactly one CLK of low strobe when a software reload
or hardware retrigger arrives during that low period. The existing output
callback now observes the restored high level; the receiver checks the
actual callback, not only the chip's readable output field. Previously mode
4 could remain low after a reload and mode 5 could hide the rise from its
consumer. Counter completion uses one private modular-decrement helper;
the existing reload value distinguishes loaded CE from a freshly programmed
control word. No new state or public interface is introduced. Pending load,
trigger and restart still take precedence over passive post-terminal counting.

The fresh 31-route chip/board selection passes on both widths after the initial
production repair. Final source review of printed page 15 preserves the separate
mode-4 rules: low GATE freezes CE, but cannot extend the low OUT pulse. The
strobe completion therefore decrements CE only when its gate permits counting;
the receiver explicitly reads zero after a gate-disabled pulse completes.
The final strengthened full-wrap/callback/gated-count waveform receiver and
the other four PIT/port routes were rebuilt and pass 5/5 on each width.
These are bounded regressions, not full S qualification or current EXE proof.

Both original release-ccache trees completed `core-board-tests` rebuilding:
x64 171 build steps and x86 359 build steps. The first fresh x64 related
31-route selection passed 30/31: the auxiliary-PIT receiver expected a
mode-1 CR to load without a GATE trigger. Its initial count-latch/clock-ratio
and primary/auxiliary isolation checks remain; it now proves no CE load
before trigger, then uses the existing GATE API and proves next-clock CE
load. The amended auxiliary receiver passes directly on both architectures.
The subsequent fresh 31-route chip/board selection passes 31/31 on x64
and 31/31 on x86, covering IRQ, priority, competition, arbitration, phase,
clock, auxiliary-PIT, DMA channel/ports and chip contracts. This is focused
board regression, not the full repository suite or external integration.

These rows refine S1's category inventory into the selected S2 mechanisms;
they do not remove a capability or redefine controller qualification around
the repaired examples. `Pending` is not acceptance even when an older test
has passed. Original manual pages identify the authority; test filenames
identify the owned receiver rather than an immutable focused-run list.

| Mechanism/context | Current owner and receiving evidence | Disposition |
| --- | --- | --- |
| PIT modes 0-5, binary/BCD, CR/CE load and gate/trigger | `chips/pit825x/pit.c`; waveform/contract/8253 receivers; 8254 pp. 10-16 and 8253 Fig. 7 | Trigger, count-read, wrap, strobe and distinct 8253/8254 waveforms reconciled below; direct receivers pass the final complete units on both widths. |
| PIT LSB/MSB, count/status latch, null count, 8253 no read-back | Same chip; read-back/contract/8253 receivers; 8254 pp. 7-10 | Source/code review retained; direct receivers pass final complete units on both widths. No additional latch repair selected. |
| PIT OUT-to-IRQ0, speaker gate/output and refresh request | `board-base/machine_board`, `pit_bus`, `board_advance`; IRQ0/speaker/refresh receivers | One provider/clock route retained; direct board receivers and all four selected integration groups pass on both widths. |
| PIC ICW1-4 initialization, OCW1, OCW2 EOI/rotation, OCW3 read/poll/SMM | `chips/pic8259/pic.c`; chip contract/commands and board OCW3; 8259A pp. 9-18 | OCW2, initialization eligibility, stale delay and held-level acknowledgement repaired; direct chip and board reset/consumer receivers pass final complete units on both widths. |
| PIC fixed/rotating nesting, cascade ID, SFNM, spurious vector | Same chip plus `board-base/pic_bus`; chip contract and board command/lifecycle receivers | Source aggregation, reset-held inputs and slave Poll repaired; master Poll shares SFNM selection with INT. Direct receivers and all four selected integration groups pass on both widths. |
| PIC physical INTA/CAS and unmask latency | Chip/board timing inputs, 5170 descriptor | No electrical acknowledge phases are modeled. 5170 IRQ1 120-source-tick IMR-release input is expressly Other-L2, not an Intel propagation formula or a BIOS-specific response. |
| DMA address/count/byte pointer, mask/request/command/mode/status | `chips/dma8237/dma.c`; contract and board channel/port receivers; 8237A pp. 7-9 | FFFFh count, EOP/TC, Master Clear and software-request status repaired. Software block requests remain nonmaskable as specified. Direct receivers pass final complete units on both widths. |
| DMA demand/single/block/cascade, M2M, decrement, auto-init, priority | Same chip and `board-base/dma_bus`; contract/channel/competition receivers; 8237A pp. 4-9 | Phase review reconciled below; auto-init preserves the current mask, and hardware cascade masking governs primary admission. Direct receivers pass final complete units on both widths. |
| DMA page decode, byte/word channels, AT cascade, request lease/reset | `board-base/dma_bus`; port/channel/token/rollback receivers | Master Clear is chip-local; whole-board reset remains separate. Direct register, configuration rejection, lease and rollback receivers pass final complete units on both widths. |
| Refresh, FDC DMA2, XT HDC DMA3 and Model40 refresh | Board composition plus App-owned selected wiring | XT DMA0 and AT refresh DFF routes separated; selected AT hold is eight source clocks. Model40 retains its own zero-duration provider. Real consumer setup corrected; direct receivers and all four selected integration groups pass on both widths. FDC/HDC command semantics remain S4. |
| Rational clocks, phase, deadline and guest-source eligibility | `core/x86/clock`, `board-base/{board_advance,board_deadline}` and selected App descriptors | One conversion owner confirmed. XT PIT ratio 1:4 is sourced but macro-axis remains non-physical; selected March 1986 AT DMA ratio is repaired to 1:2, with PIT ratio 596591:4000000 retained. Model40/default retain explicit L2 boundaries; direct receiving proof passes both widths. |

Current inspection confirms the 5170 readable port-61h refresh waveform is
explicitly a 64-source-tick Other-L2 observation, separate from PIT1-to-DMA
wiring. It must not be reported as a sourced physical refresh half-period.
AT reset's fixed PIT1 divider, XT firmware programming and Model40's owned
refresh binding remain distinct facts, not candidates for a shared timer
constant or scheduler.

## Similar-Issue Sweep And Remaining Work

Search/read scope: `src/core/chips/pit825x/pit.c`, all matching chip tests,
`src/core/board-base/{pit_bus,board_deadline}.c`, PIC aggregation and DMA
request/service paths. Queries include `flagLoadPending`, `flagTrigger`,
`flagGate`, `ticks_until`, `irq_source_assert`, and `dma_pending_requests`.

- Mode 0 software load and gate-paused CE remain distinct and covered.
- Mode 2/3 restart and period-boundary loads remain distinct. Odd mode 3
  starts at N-1 on 8254; the 8253 variant is reconciled separately below.
- Mode 4 strobe completion across a falling GATE was reproduced and repaired
  in the existing tick/deadline mechanism, with a dedicated negative control.
- PIC ICW3 cascade addressing is implemented now; the obsolete fixed-IRQ2
  historical finding is not a current repair requirement.
- PIC same-line source aggregation and slave poll refresh are repaired with
  direct source and board tests. The public rebind contract requires a
  quiescent/reset source; live asserted-source rebinding is not an admitted
  runtime operation. Full priority/cascade/reset/timing review remains open.
- DMA software request masking, transfer phases, refresh/FDC/Xebec consumers
  and selected clock conversion remain under audit, not accepted from S1's
  category-level L3 wording.

The complete finite source/register/mode/reset/consumer ledger must be
reconciled before S2 acceptance; these initial repairs do not reduce it to
three observed failures.

## Current Candidate Review

### Complete-unit receiving failures and reconciliation (2026-10-11)

Both original build trees completed their full repository-only unit builds.
The first x86 complete run finished 539/546, with seven failures; the first
x64 complete run reached its 300-second containment deadline and is not a
complete qualification result. No new suite budget or Shared change is made.

Two prefetch failures exposed a real candidate regression: pre-admitting
zero-duration refresh consumed the request before the existing arbitration
point could exclude that interval from prefetch grants. Pre-admission now
requires the declared nonzero refresh duration at its two existing entry
points. Zero-duration Model40 and neutral providers retain their original
arbitration point; there is still one service function and transaction owner.
The neutral/Model40 prefetch receivers pass without assertion changes.

Four Model40/Port-B receivers still sampled the old mode-2 first-pulse phase.
Count 18 reaches low on CLK 18 and high on CLK 19; count 2 reaches low on
CLK 2 and high on CLK 3. Their observation times now use that original Intel
sequence, retaining status, row, hold, DMA and speaker predicates. The four
receivers pass on x86 and, together with prefetch and AT arbitration, the
fresh eight-route selection passes on x64.

The seventh receiver, Default-PC FDC ports, used HLT-only fixture firmware
without PIC initialization, then demanded successful PIC IRQ selection.
The new chip initialization contract correctly rejected that request. The
test now programs the four master-PIC initialization bytes through real
ports before testing FDC. A named failure-stage diagnostic identifies setup
versus controller operations. Both freshly built FDC routes pass. An initial
diagnostic compile lacked the Types file declaration; that unsuccessful build
and a subsequent old-executable run are excluded from repair proof.

The AT authority receiver also resets during the seventh occupied tick,
then proves the new reset request cannot commit at the old completion tick:
row stays zero after one tick and advances only after the full eight ticks.
This strengthened receiver passes on both widths. Complete fresh unit runs,
selected integration, artifact deployment and final ledger review remain open.

### PIT mode-2 CE and first-pulse phase (2026-10-11)

This paragraph records the initial 8254 repair. Its application to 8253 was
rejected by the later original-diagram review below; the current candidate
does not apply the 8254 count/deadline sequence to both personalities.

Visual review of Intel 8254 printed page 13, Figure 17, confirms
`3 -> 2 -> 1 (OUT low) -> 3 (OUT high)`. The old implementation instead
reached zero for the low pulse and decremented the reloaded count immediately.
This delayed the first low pulse by one CLK and made count reads disagree
with OUT and the existing N-1 deadline. The owner-local correction changes
three lines (net -1): enter low at CE=1 and reload N on the next CLK.
No deadline API, state or board special case is added.

The existing waveform receiver now independently latches CE and checks OUT
and next deadline through two periods, binary/BCD, on 8253 and 8254. Its
negative control fails before the correction; after correction it passes.
The previous mode-2 pulse locator is corrected to the sourced first phase,
superseding the unresolved count/phase item in the earlier notification note.
Old rewrite, GATE restart and alias assertions now use that same source
sequence rather than the previous off-by-one phase.

### Original 8253 personality reconciliation (2026-10-11)

Intel 8253/8253-5 order 231306, November 1986, printed 3-55 and 3-56,
mode descriptions and Figure 7, distinguish two counter sequences from
8254 order 231164. Existing source hashes and provenance above apply; the
original pages were visually inspected, not inferred from another emulator.

For odd mode 3, 8253 reloads the full N. The first high-half decrement is
one and the first low-half decrement is three; subsequent decrements are
two. For N=5 the sampled sequence is `5,4,2,5,2,5`. The 8254 sequence is
`4,2,0,4,2,4`; both retain the same three-high/two-low OUT phases. The
existing personality and remaining phase identify the first decrement,
without a flag, new owner or public interface. A new per-CLK latch/OUT/
deadline receiver rejected the old shared N-1 implementation, then passed
after the owner-local repair on both widths, in binary and BCD.

For mode 2, 8253 Figure 7 labels the low interval `0(N)`, with the
following interval N-1. Its initial N=3 sequence is `3,2,1,0,2,1,0`;
8254 Figure 17 instead places low at one: `3,2,1,3,2,1,3`. The existing
elapsed-tick representation retains zero during the 8253 low interval,
reloads and decrements at the next CLK; the 8254 branch reloads without
that decrement. The first 8253 low deadline is N, not N-1. Tick and
deadline consume the same existing personality; no board-specific rule
or additional persistent state is introduced. The waveform receiver now
has separate original-source count/OUT/deadline tables, and its pending
rewrite and GATE-output locators respect that same distinction. It fails
against the preceding candidate and passes against the corrected candidate.

Fresh `pit_waveform`, `pit_8253` and `pit_contract` executions pass 3/3
on each width. This is focused chip evidence, not the final board, suite,
integration or artifact qualification. The prior x64 full execution completed
544/546: only Core source/test manifests failed while PIT work changed their
inputs. Those manifests have now been refreshed; the old result is retained
as a failed qualification, not relabelled successful.

### Secondary DMA cascade mask reconciliation (2026-10-11)

Intel 8237A order 231466, September 1993, printed pages 5-8, defines
cascade as an incoming DREQ/acknowledge connection and mask as disabling
that incoming DREQ. The board instead injected a primary request directly
into the secondary priority candidates, bypassing the secondary channel-0
mask. The new same-owner port/request receiver masks that cascade, asserts
an unmasked primary request, and requires no pending service, grant or RAM
change; unmasking must resume the same request. It rejects the old candidate.

One private board wiring helper now drives the existing secondary DREQ0
from primary eligibility at register writes, bound external request changes
and completed controller clocks. The secondary chip remains sole owner of
mask and priority. Admission and pending-request observation both use its
resolved request; no public API, signal-struct field, mask mirror, fallback
path or scheduler is added. Already granted services retain the existing
completion semantics; this change gates new incoming cascade requests.

Old positive primary-service fixtures that reset both controllers but only
unmask the primary now explicitly unmask the secondary cascade through the
guest port. The seven-channel phase matrix does the same for its byte-side
rows. The explicit cascade-mask negative still masks after setup and retains
all no-service predicates. Temporary per-assertion diagnostics identified the
one remaining verify fixture missing cascade initialization and were removed.
The freshly rebuilt complete DMA-channel receiver passes on both widths,
including its mode/phase, failure, reset, EOP, priority and new mask cases.
This is not final whole-board or integration proof.

The x64 36-route chip/board selection initially passes 34/36; both failed
competition receivers had the same missing secondary cascade unmask in
their guest-port setup. They now perform that explicit write, preserving
all hold/arbitration/order/duration assertions. Both rebuilt competition
receivers pass on x64. No source fallback was introduced for these fixtures.
Fresh x86 board re-link completes 171 steps and the same selection passes
36/36 (1.95 seconds). The 22 Core and seven chip receivers are freshly
linked; seven App-labelled registrations still need their final full-graph
re-link before current-artifact qualification. The same limitation applies
to the earlier x64 App-labelled entries. No external or desktop integration
is inferred from this result.

The subsequent x64 complete run also finished 544/546 (246.03 seconds),
with only Core manifests failing as DMA source/test inputs changed during
execution. It is not current-candidate qualification. Both manifests were
refreshed after that run. Do not repeat the full suite until the remaining
source/board batch review is stable; use focused receivers during repair.

Fresh first board execution found three stale receiving oracles: IRQ0's
initial pulse and BCD reload count, PIT1-to-DMA's low/high phase, and divider
edge totals. These were reconciled against the existing input-clock ratio
and CPU retirement duration, not softened to accept arbitrary values.
The fresh ten-route receiving selection passes 10/10 x64 and 10/10 x86.
Complete repository-only build/qualification is now in progress in the two
original release-ccache directories; no artifact or integration success is
claimed from this focused proof.

### PIT programming/GATE OUT notification (2026-10-11)

Intel 8254 printed pages 10 and 12 define initial OUT on programming and
immediate high OUT when GATE falls during the mode-2 low pulse or mode-3
low half-cycle. The chip previously changed its copied OUT value but suppressed
the rising callback in those two paths. The output contract now reports each
actual transition through the existing helper; its `notify_rise` argument and
conditional suppression are removed, with no public API or state addition.
Registration still emits no synthetic edge, unchanged writes/GATE values
emit no duplicate, and reset/destruction retain their deassertion contract.

The existing waveform receiver adds both-personality initial-mode and GATE
transition checks. Its negative control fails before the production repair.
The real PIT-port-to-PIC receiver accepts the initial IRQ, acknowledges it,
and separately verifies a GATE-induced rise reaches IRQ0. Divider checks
now include the programming rise, rather than silently excluding it.

The first new low-pulse locator used three elapsed ticks for the existing
mode-2 count-four implementation and was corrected to locate its actual low
pulse at four. This notification test is not source qualification of the
initial mode-2 low-pulse phase: the manual says low when CE reaches one,
whereas the current implementation sets zero. That phase/count discrepancy
is retained as an explicit pending S2 item, not accepted by the adjusted
locator. Mode-3 lookup and notification remain independently checked.

Initial regression failed four routes. AT reset programs PIT1 and therefore
now correctly generates a real rising request; the new authority/competition
fixtures first service that initial refresh before isolating their later
request, preserving row progression and reset request assertions. The divider
receiver includes the initial edge. After these explicit fixture corrections,
fresh focused selection passes 24/24 on each width. These results supersede
the earlier focused run, not the complete S2 qualification boundary.

### Selected AT refresh candidate (2026-10-11)

The later selected-board source is the IBM 5170 Technical Reference,
March 1986, archive `platform/ibm-5170-technical-reference-mar1986.pdf`,
SHA256 `3224C26B424A4745C765F56AF7D957D3C4AD959E0F719EBEB39540A2508E95D5`.
Original scan acquired from minuszerodegrees on 2026-10-11; PDF page 22
(printed 1-7) and page 38 (printed 1-23) were visually reviewed. The former
specifies eight CPU clocks for refresh and 3/4 MHz DMA for 6/8 MHz CPU;
the latter connects PIT OUT1 to the rising-edge refresh latch, cleared by
refresh service. This supersedes the early March 1984 five-clock value for
the selected later 8 MHz board; it does not change XT's DMA-channel-0 route.

The candidate therefore leaves AT DMA0 available, latches an OUT1 rise
independently of its later fall, and services the row through the existing
Core REFRESH transaction. One appended transaction-contract duration value
and one private remaining-duration value express occupation of the existing
bus. Board pending/row values are the actual latch and address owner, not a
second scheduler. The reported public-value extension is admitted in CURRENT
under the owner's automatic timing-upgrade approval. My5170 supplies eight
source ticks and DMA ratio 1:2. Default supplies the same nominal AT-like
duration but retains its unqualified/model axis; Model40 retains its own
refresh provider and zero-duration point service. Sourced board constants do
not establish electrical cycle accuracy for the macro execution axis.

Existing DMA/RTC authority regression proves DMA0 availability, rising-edge
latching, no withdrawal on falling OUT1, seven ticks with no CPU retirement,
eighth-tick completion, next-deadline capture and row/reset behavior. Existing
competition regression proves no DMA write/transaction during the eight-tick
refresh, then subsequent accepted DMA transfer. Both direct receivers pass
x64/x86. The first competition candidate omitted board DMA configuration and
therefore never installed the PIT refresh callback; its failure was corrected
by completing the real board route, not changing production or the oracle.
Earlier authority-test oracles were also corrected for the first counter-load
clock and the existing three-tick NOP; these failures are not passing evidence.

Both original Ninja Core-board aggregates rebuilt successfully (369 actions
per width). The fresh 24-route chip/board selection passes 24/24 on x86
and 24/24 on x64. Both CTest processes are terminal. Core source
and test manifests verify, and whitespace review passes. Full repository,
cross-profile integration and eight artifacts remain pending; these focused
results do not close S2. The separately rebuilt My5170 clock-contract receiver
also passes once on each width, checking the selected DMA ratio and duration.

The older counts and AT clock statements below describe the pre-AT candidate,
not final current qualification. Remaining PIT programming OUT notifications
and complete refresh/reset/arbitration disposition still require review.

Current `git diff --numstat` counts five production C files +79/-53 (net +26)
and nine test C files +334/-8 (net +326). Manifests and documentation are
excluded. The test increase provides actual maximum-count, EOP, reset and
physical IRQ/GATE boundary receivers rather than production scaffolding.
All added test code stays in existing receivers. The one new private helper
owns binary/BCD decrement across zero; no public interface, enum, persistent
state, thread or allocation is added. Core source/test
manifest verification, NXVM documentation governance and `git diff --check`
pass. These are candidate checks, not S closure or current artifact proof.

### DMA autoinitialize preserves a software mask (2026-10-11)

Re-read original Intel 8237A September 1993, order 231466, printed page 6:
Autoinitialize restores base address/count after EOP but leaves the mask bit
unchanged. The current completion owner instead cleared that mask. A mask
written during an active transfer was therefore lost at completion. Removing
that one clear repairs the single completion path, including memory-to-memory;
non-autoinitialize completion still sets its mask.

The existing chip contract receiver now tests the complete three-mode matrix
of autoinitialize on/off, mask written during service on/off, and external
EOP versus count-zero terminal completion (24 contexts). It checks cycle count,
terminal notification, address/count reload, TC, grant release and request
eligibility; an explicit subsequent unmask must admit the still-held DREQ.
The expanded receiver failed on the preceding production candidate and passes
once on each width after the one-line removal (x64 0.43 s, x86 0.11 s).
No new state, API, helper or fixture framework is added. The stale board-test
comment claiming automatic mask clearing is corrected; its existing unmasked
autoinitialize path remains distinct from the new masked-service contexts.

This increment is production +0/-1, test +22/-11 including the board comment,
net +10 C lines. It is direct chip evidence only; complete board/integration
qualification and eight current artifacts remain pending. PIC reset inspection
also found that aggregate counters are cleared separately from source leases;
the full producer reset order and held-input semantics require disposition
before changing that owner or claiming its reset complete.

### PIC reset retains physical IRQ inputs (2026-10-11)

Intel 8259A December 1988, order 231468, printed pages 9-10 and 17,
distinguishes initialization of the edge-sense circuit from the external IR
inputs. Edge mode requires a new low-to-high transition after initialization;
level mode disables that edge detector. Resetting the PIC cannot itself
deassert an independently owned source. Board reset separately resets keyboard,
DMA, RTC, FDC and HDC before PIC, then PIT afterwards; those producer callbacks
own actual signal changes.

The board PIC reset nevertheless erased the aggregate counts while retaining
asserted source leases. Subsequent refresh lost a held level, and releasing
one of two asserted sources could no longer preserve the other's contribution.
The repair removes the two counter clears and reuses the existing refresh after
chip reset. No source, edge, register or request mirror is introduced. The
existing aggregate remains the one resolved physical input owner; chip reset
still clears its own state and does not synthesize a new edge.

The existing lifecycle receiver tests master IRQ6 and slave IRQ14 in both edge
and level modes, holding two independent source leases across reset and the
actual ICW port sequence. Level mode must remain eligible until both release;
edge mode must remain ineligible until a real release/reassert. Both then
acknowledge the expected vector and clear through normal EOI. The added receiver
fails on the preceding production candidate and passes once on x64 (0.29 s)
and x86 (0.08 s) after repair. Production +1/-2, tests +28/-0, net +27.
Full rebuilt board/reset and four-profile receivers remain required; the focused
pass does not establish complete S2 qualification.

### DMA request status and PIC Poll selection (2026-10-11)

8237A printed pages 7-9 distinguish nonmaskable block-mode software requests
from masked external DREQ, and define Status bits 4-7 as channels requesting
service. Status read previously exposed only external DREQ. It now ORs the
existing Request register into those bits; reading still clears only TC bits.
The existing chip receiver proves a masked software block request remains
visible across repeated status reads, clears under the request command, and
clears at terminal completion while TC survives until read. The new receiver
failed before the one-line production replacement. No request eligibility,
mask, priority, state or API changes are made.

8259A printed page 16 specifies normal INT operation during Poll and treats
the subsequent read as an acknowledge. The existing Poll-specific selection
excluded the SFNM exception that normal INT already accepted. Poll now calls
the existing `x86_pic_select` and acknowledges the same copied request. The
old test intentionally expected the inconsistent result; its source-corrected
case now requires master Poll to return IR2 and the higher-priority slave Poll
to return IR0. This receiver fails before the repair and passes afterwards.
The slave's Poll remains a separate local acknowledge; no pair-level implicit
acknowledge, second selection policy or public interface is introduced.

Both fresh chip contracts pass once on x64 (2/2, 0.57 s) and x86 (2/2,
0.19 s), including the strengthened software-TC receiver. These focused
results precede the complete freshly linked chip/board and product regression.

### Remaining source-batch reconciliation before final runtime qualification

The mode/CR/CE read and write-during-strobe candidates are covered by the
existing waveform tables: terminal binary/BCD wrap, modes 1/5 pending CR
without trigger, active rewrite, mode-4 low pulse/reload and mode-5 low
retrigger. Read-back separately proves count/status selection, first-latch
retention, status-before-count, null count before CLK, independent counters,
and the one-byte forms. The 8253 receiver rejects 8254 read-back; its original
mode-2/odd-mode-3 count sequence remains separate. No additional production
repair is selected from this read review.

PIC initialization, mask/nesting, OCW2 rotation/EOI, OCW3 SMM/read/Poll,
SFNM, spurious vector and held-input reset now use the existing chip and
board receivers. Poll and normal INT share selection, while actual master
and slave acknowledgements remain separate. Electrical INTA/CAS pulses and
8080 CALL-byte delivery are not selected PC consumers; no electrical timing
qualification is claimed for the retained logical interface.

DMA phase review retains S1 on first grant/high-address carry, S2/S3/S4 normal
service, S3 omission for compressed service, and independent four-state M2M
read/write. Direct phase fixtures exercise byte/word channels, three transfer
modes/types, decrement, auto-init, fixed/rotating priority, cascade delegation,
EOP and failure/reset. READY pin wait states, polarity pins and electrical
extended-write pulse widths are not represented by the selected logical bus
callback; this is a declared model boundary, not full pin-level qualification.
Software requests retain the manual's nonmaskable block-mode rule. Board
reserved cascade is a fixed AT connection, not a new bindable device channel.

Selected XT, AT and Model40 refresh routes remain different sourced wiring
contracts; FDC/HDC command/media qualification remains S4. Cross-profile
reset, deadline, speaker/IRQ and DMA consumers must now pass against the
freshly linked candidate. No row is accepted merely from this static review;
complete units, selected integration and artifacts remain required.

## Complete-unit replay and real DMA consumer initialization

The first immutable x64 replay terminated at its 300-second aggregate budget,
after 509 of 546 results. Its temporary CTest log records ten failures; it is
not a passing complete qualification. The failed routes were the FDC controller,
transaction, plan, Model40 refresh-hold and FDC, three AT DMA-deadline consumers,
default FDC read-track, and XT Xebec wiring. No manifest failure was found in
this replay. The terminal process status is failure, not a live wait.

Nine routes omitted the secondary DMA cascade unmask in their own AT guest
programming. With the old primary-arbitration bypass removed, channel 2 alone
cannot request the AT bus while the secondary cascade is masked. The repair
adds D4h=00h to existing AT setup functions. The shared Core deadline fixture
does this only for two-controller configurations, retaining the XT route.
No controller command, expected transfer byte, deadline, transaction ordering
or failure assertion is relaxed. These are test initialization changes only.

The Xebec receiver is different: its wiring declared one controller while its
machine creation omitted dma_controller_count and therefore created the default
two. Diagnostic observation showed unchanged address 2200h/count 03FFh with
request 08h and no active grant. Explicitly constructing one controller fixes
the receiver's contradictory XT topology. Temporary byte/register diagnostics
were removed after proof; the original two-sector contents and response checks
remain. Production HDC and FDC semantics are unchanged.

The ten affected x64 receivers now pass (the nine-route replay initially retained
the Xebec failure, followed by its passing exact receiver). The x86 ten-route
replay passes 10/10 in 3.07 seconds after an incremental 22-action unit build.
Core test manifest verifies after updating all changed fixture hashes.

A new x64 complete-unit replay uses the existing RunTestAggregate tool and
original release-ccache tree with jobs=4 and an explicit 900-second aggregate
containment budget. Individual CTest budgets are unchanged. This does not modify
the runner, CMake, Shared code, system settings, or hardware timing. The longer
aggregate budget permits the finite 546-route collection to finish; neither an
observation timeout nor that containment limit is evidence of device timing.
Final full-suite results, integrations and artifacts are still pending.

The immutable x64 replay finished 546/546 with zero failures in 68.30 seconds.
One XT deadline executable still required relinking after the shared Core test
header changed; the two-action rebuild and its exact receiver pass in 0.23
seconds. The source, Shared and MyNES trees did not change during the replay.
The complete x86 replay is now active with the same 900-second aggregate
containment and unchanged per-test limits. No complete x86, integration or
artifact qualification is claimed yet.

Construction review additionally distinguishes two admission boundaries:
machine_plan.c already rejects a plan's DMA wiring count when it disagrees
with the selected creation count; direct core_machine_configure_dma() currently
validates each wiring value, but does not compare its count to the previously
created hardware. The Xebec receiver exposed that lower-level mismatch. This
is a remaining S2 configuration-preflight candidate, not proof of a failing
four-App product plan. It must be disposed before S2 acceptance; no new public
getter or controller-count mirror is justified merely to inspect the hardware.

### Direct DMA wiring admission

The preceding complete x86 replay also finished 546/546 in 73.88 seconds.
Both results qualify the candidate before the direct-admission change below;
they are not reused as proof for a later source revision.

The lower-level count mismatch is now repaired at board construction/configuration.
Construction initializes the already-existing dma_wiring.controller_count from
the actual creation count. Configuration compares a proposed count before binding
channels or publishing wiring, then uses the same existing wiring record on
success. No public function, enum, field, new manager or mirrored controller
state is added. A Plan still retains its independent preflight responsibility.

The existing DMA/RTC authority receiver tests both mismatch directions with
otherwise valid wiring: one constructed controller versus a two-controller
proposal, and two versus one. Rejection leaves the caller's binding sentinel,
dma_configured and original hardware-count record unchanged. Correcting the same
proposal succeeds on that same machine, proving rejection did not poison future
configuration. The new receiver failed before the production repair; the repaired
x64 authority and original XT Xebec receivers pass 2/2. Final rebuilds and replay
are pending for this source revision.

The board channel test's mask assay remains internal to its existing receiver.
Its callers either use hardware-only requests or have already completed/cleared
the software request (including M2M TC/EOP). Invalid-route software-request tests
do not invoke the assay while the request remains pending. Thus the now-complete
status register is not used to turn a live software request into an external
DREQ in these callers. No chip-private include or public test getter is added.

The subsequent complete-unit builds finish 265 x64 and 268 x86 incremental
actions. Both final source candidates pass 546/546: x64 250.27 seconds, x86
68.46 seconds. These are automated repository-unit qualifications, including
the registered desktop-labelled receivers, not new manual-desktop acceptance.
The two runs used original release-ccache trees, jobs=4 and 900-second aggregate
containment. Manifests were stable throughout each run.

Artifact metadata now selects PC revision 0.5.0555 in the existing Core Product
version, NXVM artifact target and the three PC-only build-preset references.
No MyNES preset, source, test, INI or binary is touched. Core manifest is updated
for the version header. The eight PC artifact builds reuse original default and
XT/AT/Model40 Ninja/ccache directories; deployment and integrations are in progress.

Protected INI baseline hashes before integrations (SHA256):

- My5160: EC2AFB0E89421ED4BDA95D8B1797864D19EC360A3AD57237D2256DCE78950338.
- My5170: F4D85EE2E7BFF403F8C19588DA8416CC96FE5AC72FE7A1764817EDDDC9B76F19.
- MyDeskPro386: C812A0C99D258C19CA8BB4BED584617DF2F4D25F17372583EB58BBD71D378599.
- NXVM: FEEFDF82F40A8C7C1CD96C130BBEC4B5C41E099E194FA955AD42B99F35E8FC4F.

### Current artifact and integration replay

All eight PC 0.5.0555 artifact targets completed in the original persistent
Ninja/ccache directories. Each deployment's existing gate confirmed the
selected PE width. The default x64 integration target completed 22/22 in
65.48 seconds, including guest timer, keyboard, DMA/FDC, HDD boot and Windows
checkpoint routes. The x86 default replay is running; XT, AT and DeskPro
receivers still require their own runs. These results do not close S2 or T555.
Both Core source/test manifests verify after the metadata update. Superseded
0551 artifacts remain temporarily until the replacement qualification is
complete; no protected INI or media is modified by artifact deployment.

The default x86 integration replay subsequently passed 22/22 in 79.49
seconds. XT passed 2/2 on x64 (22.91 seconds) and x86 (28.99 seconds).
Selected 5170 passed 4/4 on x64 (47.55 seconds) and x86 (60.18 seconds),
including its deployed Console lifecycle, CMOS seed, INI status and actual
firmware floppy-boot checkpoint. DeskPro qualification remains in progress.
These are one execution of each selected integration route, not repeated
three-run or 24-run sampling. Automated Console receivers are distinct from
manual desktop acceptance, which was not performed in this replay.

Actual added/deleted line review uses `git diff --numstat` against `710e0da4f`,
counting C/H source, C/H/CMake tests and the two artifact configuration files
separately. Source is +243/-111 (net +132), test code/registration +814/-44
(net +770), artifact build references +5/-5 (net zero). Manifests, supporting
documentation and EXEs are excluded from these code totals. Most added test
lines express independent manual-conditioned waveforms, TC/EOP/mask matrices,
held-input reset and refresh bus occupation; they are not new production
state or a framework.

The one appended public contract value is the admitted refresh source-tick
duration. Board owns its AT request latch/row; Core owns the remaining bus
occupation interval, transaction and guest-time publication. These represent
different hardware responsibilities rather than two copies of a clock or
request. One private cascade helper resolves primary HRQ into secondary DREQ0;
one PIT decrement helper owns binary/BCD terminal wrapping. Existing chip
handlers and the sole scheduler remain the production paths.

All eight deployed artifacts have the correct PE machine (8664 for x64, 014C
for x86), and `objdump -h` finds no compiler debug sections. SHA256 values:

| Artifact | SHA256 |
| --- | --- |
| nxvm_xt_0_5_0555_x64.exe | 1747907AD70F7807D41E1916C9F872E96253E61F11208B40BFDAA8BE420B6895 |
| nxvm_xt_0_5_0555_x86.exe | E2D604FDB3BF3784D433EC5E7A677FC6F07E64D413CE97182D221F060A43EE73 |
| nxvm_at_0_5_0555_x64.exe | E027E402A1FDF2F0035039EC3D4016E78A17FFFC88565E0A84662E6E3DBD4911 |
| nxvm_at_0_5_0555_x86.exe | 50921677D11D3FF0DEC96B695A424F092C2D5B2E7954359E55654684CF1AA5C3 |
| nxvm_model40_0_5_0555_x64.exe | D891DFE60CD1BC25C89A016475D678A97168BFC96795E2605C717229038582D8 |
| nxvm_model40_0_5_0555_x86.exe | 31EA30BB62298927DD835BCF4176C267950D2F4D6D42D9FCA089F3063FAFFAF9 |
| nxvm_default_0_5_0555_x64.exe | DC36B0B48738791C78C50FA8353F179D26C08D40AACCB55081788FB812E7EC44 |
| nxvm_default_0_5_0555_x86.exe | A88C09DDCFAE49B56F6C609B7FE76E8223FB736D3637DFFB69E064C87897C391 |

The four adjacent INIs still match their recorded pre-integration hashes.
Core source/test manifests, Core corpus and documentation governance pass.
Actual diff review found no change in Shared-six or MyNES paths. Artifact
configuration edits only advance the existing PC targets; they do not alter
the MyNES presets, compiler launcher, parallelism or shared library sources.

### Remaining DeskPro x86 qualification failure

The sequential original integration target completed DeskPro x64 4/4 in
150.01 seconds (boot checkpoint 148.99 seconds). The final DeskPro x86 target
completed only 3/4: its boot receiver reached its own unchanged 180-second
containment without the installer checkpoint. At the paused observation it
reported elapsed source tick 422032111, next deadline 422032124, PC
F000:90E5 and instruction bytes EB/00/EB/00/E4/40/8A/E0, in a BIOS PIT-read
sequence. Lifecycle was running, not a crashed or stopped process. This
observation alone cannot distinguish slow host execution from a guest defect.
The terminal aggregate handle 99337 returned exit 1. No passing result is
claimed for that group; S2 acceptance, old-artifact retirement and commit
remain blocked on its disposition.

A separate bounded diagnostic invokes the already-built x86 boot receiver
with its existing optional 240000-ms argument and unchanged App INI. This is
an investigation run, not a raised qualification budget or replacement for
the original failing CTest. It adds no source, timeout-policy, host-system or
checkpoint change. Its result must be reconciled before another repair is
selected.

That diagnostic handle 81575 terminated with exit 0 and
`installer-running`. Its process was independently observed live, consuming
CPU, and its creation-to-observation interval was at most approximately
183 seconds; no exact runtime is claimed from that upper bound. This proves
the same x86 candidate can reach the unchanged guest checkpoint, not that
it meets the original 180-second CTest condition. The original single failed
CTest is now replayed with its unchanged arguments and budget. Earlier
failure evidence is retained regardless of that result.

The original-budget replay handle 10335 also failed, at 180.05 seconds with
elapsed source tick 420766837 and BIOS PC F000:90E9. No further unchanged
retry is selected. Existing TODO already records Model40 x86 throughput as
a separate known boundary, but that is not used to waive this S's required
integration result.

Current-code review identifies one repeated empty-work case within the
admitted DMA request mechanism: `dma_pending_requests()` checks all four
software-request channels even when the software Request register is zero.
In that case their predicates cannot contribute to the already computed
masked hardware requests. A one-line owner-local early return skips that
empty loop. Nonzero software requests retain the existing loop, nonmaskable
block/M2M rules and priority selection. No timing, guest work, scheduler,
state, public API or qualification-budget change is made. Existing complete
DMA chip/board matrices cover both empty and populated software requests;
fresh focused/full qualification and receiving artifact builds are required
for this new candidate. Expected production delta is +1/-0; measured
throughput benefit remains unproven until the receiving replay.

The repaired request-owner candidate's handle 24225 completed the DMA chip
contract and board channel receivers 2/2, then the original registered DeskPro
x86 boot receiver passed in 178.94 seconds with its unchanged 180-second
internal condition and installer checkpoint. The previous failures remain
above; this is a narrow-margin single receiving pass, not a statistical
stability or host-wide speed claim. Full units, the other receiving integration
routes and eight artifacts require fresh qualification of the one-line change.

Both final unit-target rebuilds completed 266 incremental actions in their
original x64/x86 trees (handle 22440, exit 0). A persistent sequential final
qualification pipeline is running under handle 17051, with generated output
recorded in ignored `build/t555-s2-final-requalification.log`: complete x64
and x86 registered NXVM units, all eight artifact targets, then the eight
selected integration groups. It stops at the first failure. The current
x64 complete unit run passed 546/546 in 249.97 seconds. x86 units and the
subsequent artifact/integration stages remain pending; no final acceptance
is claimed. The MyNES build/test graph is not a Core-change receiver and
was not rebuilt or tested by this qualification pipeline.

The final x86 complete unit run also passed 546/546 in 77.20 seconds.
All eight current 0555 PC artifact targets subsequently rebuilt in the
original persistent Ninja/ccache trees. Direct PE-header inspection confirms
8664/014C respectively; section inspection confirms no compiler debug
sections. The final artifact hashes below supersede the earlier candidate
table without deleting that chronological evidence:

| Artifact | Final SHA256 |
| --- | --- |
| nxvm_xt_0_5_0555_x64.exe | 3EC84E25D64DBD01AB487F8AA1CA780FADF37248F822FD6106CFFCF95EE261D2 |
| nxvm_xt_0_5_0555_x86.exe | BFA3D298C8CF90E032F7C5B02D0177D63552EF78BDC0AAAF02E8E5DBDDAFC719 |
| nxvm_at_0_5_0555_x64.exe | 78D840C9938BBBC8C55EF79D149E6E0F6FA11A1630E63439452A78C1A7301FAB |
| nxvm_at_0_5_0555_x86.exe | 14F6A778388F15EF39C025666D713C512BFFEEB2663871062F09414A76F355AC |
| nxvm_model40_0_5_0555_x64.exe | 13606A99C3258E306224BF3F013C2D07E42258BA93BDEBA3BB092178D449A85D |
| nxvm_model40_0_5_0555_x86.exe | 995E7E000BF8D0679633EDA0ECBD5DE26A647B75647DC0ED7403FD393F97B107 |
| nxvm_default_0_5_0555_x64.exe | 6750E3CAD778FAEFD4ECB561ED8FFF6D40223FCC7DE50217056EA96D881792B3 |
| nxvm_default_0_5_0555_x86.exe | 1307F65DF403B991C90D806E6F8B6706A73664BECC8DA7373079BEAA8062BC17 |

The sequential final integration pipeline has completed default 22/22 per
width (62.69/79.25 seconds), XT 2/2 per width (22.54/29.13 seconds), and AT
4/4 per width (45.86/60.14 seconds). DeskPro qualification remains live under
the same pipeline handle; no final acceptance or old-artifact retirement is
claimed before its terminal result. These are automated receiving tests,
not manual desktop acceptance or a statistical throughput guarantee.

### Final candidate self-review

The actual chip, board, Core scheduler, selected descriptor and receiving
test diffs were inspected against the S2 packet. PIC Poll uses the same
selection contract as INT; aggregated physical lines generate only a combined
rising edge. DMA Master Clear preserves programmed registers and board page
latches, while whole-board reset remains its separate owner. Cascade masking
is enforced through the secondary controller rather than bypassed. PIT CR/CE,
gate, wrap and output transitions remain within the original chip handlers;
the binary/BCD decrement helper owns genuine shared arithmetic, not a second
counter or timer. The empty software-request fast path does not skip nonzero
requests or any hardware request calculation.

The added refresh latch/row and Core remaining-duration state are distinct:
board request generation versus single-bus service occupation. The existing
transaction/scheduler owns completion and reset cancellation. No CPU
instruction implementation, BIOS response, Shared source/test, MyNES input,
user configuration or integration checkpoint is changed. Existing private
PIC selection guards are retained as local predicate preconditions, not a
parallel eligibility or recovery path. Positive code growth supplies the
missing hardware behavior and independent source-conditioned tests.

Direct Core source/test manifest, Core dependency and NXVM documentation
checks pass, and `git diff --check` has no whitespace defect. One attempted
test-manifest command named a nonexistent test-local checker; it did not
verify anything. The correct registered arrangement uses the source checker
with `CORE_ROOT=test/core`; that command passed. The four protected INI hashes
still exactly match their recorded baseline. Manual desktop validation and
MyNES runtime testing are not claimed for this Core-only batch.

### Terminal final qualification

Pipeline 17051 exited 0. DeskPro x64 completed 4/4 in 148.07 seconds
(boot 146.93); x86 completed 4/4 in 180.60 seconds (boot 179.93).
Its internal boot budget remains 180 seconds; the total includes the three
other tests and CTest overhead. Together with default 22/22, XT 2/2 and AT
4/4, that is 32/32 receiving integration tests per width. No unchanged group
was repeated for a multi-run stability claim. All build/test handles owned
by the final qualification are terminal.

Counted `git diff --numstat` source paths excluding manifests are +244/-111,
net +133; test C/H/registration paths are +814/-44, net +770. Artifact build
references are +5/-5, net zero. The final tables and dispositions supersede
chronological pending notes; they do not erase rejected candidates or their
failure evidence. S3-S6 retain RTC/KBC/PPI, FDC/HDC, video and cross-profile
closure. This S does not qualify the complete T555 domain.
