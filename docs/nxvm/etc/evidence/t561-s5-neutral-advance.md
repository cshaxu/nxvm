# T561 S5 Neutral Advancement And Interrupt Boundary

## Admission And Design

Baseline is accepted S4 `91046e0f8`. This batch consumes I3 P4/P6, not the
neutral Machine directory move or complete T561 acceptance. Architecture
governance first assigns board device sequencing to Board-Base and guest
elapsed time, timeline, bus/refresh/DMA arbitration, FPU/WAIT progress and
interrupt transactions to the existing neutral executor. Coding governance
then removes the old device-specific slots rather than preserving aliases.

Before implementation the packet estimates production +110--200/-150--260,
tests +120--240/-70--160 and checkers +20--70/-20--70 lines. Actual review
finds a smaller implementation: the board work forms two contiguous blocks
around internal readiness. Two bounded callbacks replace PIT conversion/PIC,
media, RTC and peripheral slots. Both receive source delta and absolute due
tick; they do not publish elapsed time. No callback registry or phase manager
is needed. PIC-named pending/ack slots become interrupt-signal operations;
the real Board implementation still owns PIC scan and acknowledgement.

The two PIT clock accumulators stay on Board. Their conversion moves from
before bus arbitration to immediately before PIT progression, using the same
source delta and existing clock phases. The conversion has no guest effect:
only the Board clock accumulator changes; refresh/DMA arbitration consumes
its own clocks, pins and bus state, not PIT accumulators. Actual PIT outputs
still change after the bus phase and before PIC refresh. No pending-clock
fields, saved tick pairs or second scheduler are introduced. This is a real
boundary reduction, not renaming named PC callbacks in the neutral scheduler.

## Preserved Causal Order

The sole scheduler continues to publish the due tick and deliver due timeline
work, then refresh/DMA/HOLD/READY/prefetch arbitration. Board's before-internal
callback advances PIT/PIC, then services FDC/HDC. Core advances internal FPU
and CPU WAIT readiness. Board's after-internal callback advances RTC, keyboard,
PIC input progression and video. A source becoming ready after PIC refresh
does not jump backwards into that same tick's completed PIC arbitration.

CPU interrupt sampling and acknowledgement use the same bus functions and
transaction begin/value/commit path, with only the neutral slot names changed.
Existing IRQ source ownership and cascade behavior remain Board-local. No CPU
implementation, instruction timing, device algorithm, public CPU API, bus owner,
thread, host input, media or production 486SX admission changes.

## Test And Gate Ownership

The existing Board scheduler test instruments the two actual production
callbacks rather than deleted individual device slots. It retains zero-budget,
source-delta, due-tick, overflow-horizon, reset and provider assertions. The
Board arbitration trace test expands its existing chain to include RTC,
keyboard and video after DMA/PIT/PIC, at the same expected guest tick.
The existing neutral attachment test starts a real FPU command: before the
internal boundary it must remain pending; after it, completion must be visible,
with exactly one invocation of each phase and one teardown. This rejects swapped
callbacks, skipped internal progression or forwarding the wrong time.

FPU/PIC and power tests use the neutral slots. Display tests call the Board's
actual after-internal phase rather than a removed peripheral entry. Existing
clock-ratio, refresh/DMA, controller, interrupt, deadline, media and reset tests
remain the receiving regression owners. No test-only public interface or new
test framework is added. Tests stay in their production component's matching
directory; the later extraction migrates each whole owner together.

Six family checks follow the new contracts. Their native effect ownership,
public/private boundary and required binding predicates remain; readiness and
peripheral checks now verify Core's explicit before/internal/after call order
and Board's own PIC/media and RTC/input/video order. The PIT/PIC check also
rejects the retired device-specific attachment fields and type.

## Actual Code Size And Preserved Paths

Count `git diff --numstat 91046e0f8` for tracked C/H and six changed CMake
checkers, excluding manifests, documents and binary artifacts. No rename occurs.

| Surface | Files | Added | Deleted | Net |
| --- | ---: | ---: | ---: | ---: |
| Production C/H | 6 | 64 | 111 | -47 |
| Test C/H | 7 | 130 | 82 | +48 |
| Family checks | 6 | 47 | 24 | +23 |

The PIT-pair type and five named board progression entry points disappear;
two cohesive Board callbacks replace them. The neutral internal-advance helper
retains actual FPU/WAIT work and is not a forwarding facade. DMA/refresh bus
callbacks remain because Core must arbitrate physical bus ownership and wait
phases; Board's real DMA device operations remain behind those contracts.
New test state is fixture-local only; no production state is added.

[Frozen review hashes](t561-s5-file-review.tsv) record all 21 changed
source/test/checker/manifest paths. A comparison against accepted S4 confirms
the complete refresh/HOLD and DMA grant functions, bus arbitration after the
removed PIT plumbing, Board clock/DMA/PIC functions, each moved device-effect
block and the entire IRQ bus file after its declared name/comment substitution.
The checked effect bodies supplement actual diff review; they do not replace
dynamic clock/ordering and receiving qualification. Changed checkers are LF.

P7/P8 A20/reset/decode work remains S6; actual neutral extraction and private
CPU-header receiver cleanup remain S7; outer-package consolidation and Board
fixture cleanup remain S8; independent final qualification remains S9. These
refine the original large batch without shrinking I0--I5 or losing CPU residuals.
The receiving sweep additionally fixes the finite scope of S2's existing T1
private-fixture obligation: 25 Board/adapter test files directly include a
current Core execution private header. This includes this batch's scheduler
instrumentation. They are presently inside the one Core test package; they
cannot retain that borrowing after X86 Core/X86 NXVM separate. S7/S8 must
preserve the assertions through owned neutral tests or public Board integration,
not copy private layouts or publish test-only manipulation APIs. Final I5
checks every such receiver, not only the earlier CPU-private-header family.

## Frozen Delivery Verification

The first x64 focused batch passes 8/8: scheduler, arbitration, board timing,
PIC phase, attachment phases, display authority, FPU interface and system
power. Duration is 6.86 seconds; logs are `build/t561-s5-x64-focus-*.log`.
All directly affected owner checks pass as standalone scripts. Initial build
attempts used unregistered target spellings; they are attempts, not compile
failures or test passes. The canonical registered target builds.

Complete x64 qualification passes 627/627 repository units in 282.74 seconds,
all specialized gates, 48 receiving integrations (default 35/35 in 63.92 s,
XT 3/3 in 33.23 s, AT 5/5 in 53.02 s, DeskPro 5/5 in 151.77 s), restored
default identity and four real published CLI/INI checks. Its four x64 EXEs
are rebuilt/deployed. Logs use `build/t561-s5-x64-<profile>-integration.log`
and `build/t561-s5-x64-final-*.log`. No repeated full x64 unit run is performed.

X86 passes 627/627 repository units in 69.44 seconds and all specialized gates.
Its 48 receiving integration executions pass: default 35/35 in 69.86 s,
XT 3/3 in 35.70 s, AT 5/5 in 53.01 s and DeskPro 5/5 in 172.27 s.
Both original graphs are restored to default; the additional identity check
and all four actual published CLI/INI checks pass per width. Eight optimized,
stripped 0561 PC EXEs are rebuilt/deployed and verified below for PE width,
embedded version, two-per-root release count and SHA-256. The persistent cache
is preserved. Both final strict-compilation matrices retain their declared
510 strict rows and three exceptions; exception-free compilation is not claimed.
Both widths include three passing automated desktop-labelled unit cases.

Lib/Emulator, imported packages, CPU/FPU, MyNES, owner INI and protected input
assets remain unchanged against the baseline. The whole batch is delivered
for coordinator actual-change acceptance; I3 extraction, I4 and I5 remain open.
No claim of full task completion, manual
RDP/physical timing, full interactive Windows GUI or PC110 qualification is made.

### Published Artifact Identity

| Path | PE machine | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `assets/nxvm/nxvm_0_5_0561_x64.exe` | 0x8664 | 1511397 | `4450211e0ea320fd013da6ef2a6e7d8a8c5d4d2e302edd9fc5f3c95ad56f5456` |
| `assets/nxvm/nxvm_0_5_0561_x86.exe` | 0x014C | 1657508 | `2304f02953276144e840b2430877d822b00e400c3b05ebc5e649f42d74df8e40` |
| `assets/my5160/my5160_0_5_0561_x64.exe` | 0x8664 | 1500375 | `8d8a177938dd097efafb533e9282f3675166020dd41ccbbb9cafc4ac5f83b9e5` |
| `assets/my5160/my5160_0_5_0561_x86.exe` | 0x014C | 1644955 | `8769c90fb9c23f3f41e8e481192a1b1bb07c61f18fb3d466b59c625cb5822d23` |
| `assets/my5170/my5170_0_5_0561_x64.exe` | 0x8664 | 1512739 | `5b9cae539199d0e13d9c2310493132528d88b4fda0856663038a65e19a4a1eca` |
| `assets/my5170/my5170_0_5_0561_x86.exe` | 0x014C | 1658722 | `96b96275d66e93691a7ee8ed9dd46683d84f5a2ec050fbb7d6b2496844c009c0` |
| `assets/mydeskpro386/mydeskpro386_0_5_0561_x64.exe` | 0x8664 | 1499467 | `a068a94ecbfdc6eb2b840b7af07c05a94d2e1fa15f17e33ebaf5f607f0c3a5bd` |
| `assets/mydeskpro386/mydeskpro386_0_5_0561_x86.exe` | 0x014C | 1645969 | `06bbe83ca345128214695cd78d876f43c60ff3297b16af792edf256cfa2cbb50` |
