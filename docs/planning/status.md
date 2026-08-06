# Project Status

## Current Work

**M5 T234 S4 complete:** removed the remaining target-only
`vm-platform-requests -> core-machine` and `vm-product -> core-machine` edges;
the former now depends on `type-facade`, the latter only on its actual product
tooling. GCC 16.1.0 `current-gates-gcc` passed again with 67/67 smoke tests.

**M5 T234 S5 design complete:** the approved opaque-session migration keeps the
complete VM layout composition-private and moves test observation behind narrow
fixture probes. It deliberately changes no runtime code; see [the design
record](m5-t234-s5-session-layout-design.md).

**M5 T234 S7 closed by P2 review:** replacing direct implementation access with
hundreds of test-only fixture wrappers would be overbuilt. Tests may directly
use existing implementation state when they test the same module and introduce
neither mirror state nor alternate runtime behavior. See [the S7
record](m5-t234-s7-fixture-contract.md).

**M5 T234 S3 complete:** `core/utils` is now the strict neutral utility/callback
layer. VM machine/platform/profile no longer import `core/product`; composition
owns both BIOS text-to-bytes assembly and machine-record disassembly binding.
The source/target boundary gate and GCC 16.1.0 current-gates run passed 67/67
smokes. Artifact `nxvm_0_5_0234.exe` is SHA-256
`1A94A5D3D9E09FD2302CE71C3DC51434E9AA9915C2F2879BB42589AAE29591E9` from
`263bf0d`; see [the task record](m5-t234-core-utils-boundary.md).

**M5 T233 complete:** the default primary-master ATA controller now admits
LBA28 PIO read/write and count-zero=256, while explicitly rejecting absent
slave selection and retaining no secondary channel. SRST is BSY on assertion
and DRDY+DSC on release, without IRQ; status acknowledges IRQ14 while alternate
status only observes it. The ATA boundary gate and 67/67 current GCC/CTest
matrix, including HDD system-image boot, pass. Artifact `nxvm_0_5_0233.exe`
is SHA-256 `35CD218379869A05D0B68435E65B70DC7EFD01EA584B4495DD7D327087967324`
from `46c6b97`; see [the task record](m5-t233-ata-pio-feature-matrix.md).

**M5 T228 complete:** VADP now owns the bounded CGA `320x200x4` controller
subset and copied indexed-pixel snapshot. Composition is the sole frame-copy
bridge; platform never reads guest VRAM. The owner-approved `console`,
`window`, and `auto` policy is implemented: Console stops clearly at supported
graphics, Window presents either admitted frame kind, and Auto moves the same
session once from Console text to Window graphics. The GCC governance gate and
62/62 current smoke matrix pass. Artifact `nxvm_0_5_0228.exe` is SHA-256
`B2D78D1B756A4BF5A1EAA1E038CDD7F4DD22C8487B5A3A5BE9EDF4C223047EB8` from
`d527f23`; see [the task record](m5-t228-cga-320x200x4.md).

**M5 T232 complete:** VM-owned MC146818-compatible RTC/NVRAM subset now uses
only core elapsed ticks, profile-bound 70h/71h + IRQ8, and a PIC source route.
The admitted register slice covers BCD/binary and 12/24-hour data, SET,
periodic/update/exact-alarm flags, read-C acknowledgement, and cold-reset
NVRAM retention; host wall clock cannot mutate guest RTC state. The RTC
boundary gate and 67/67 current GCC/CTest matrix pass. Artifact
`nxvm_0_5_0232.exe` is SHA-256
`B3BDABEEA31E4F61063447905259286AD374EB2661CE7492D21030F58EA7E37B` from
`8f90a92`; see [the task record](m5-t232-cmos-rtc.md).

**M5 T231 complete:** the VM-owned default FDC now has explicit command,
execution, and result phases, a validated fixed-geometry FDD backend, bounded
non-DMA/format behavior, media-change reporting, rate admission, and the sole
DMA2/IRQ6 completion lifecycle. Core remains the only DMA, PIC, guest-memory,
and elapsed-tick owner. The focused port smoke, static ownership gate, and
66/66 current CTest matrix passed; see [the task record](m5-t231-fdc-state-machine.md).

**M5 T230 complete:** core owns the dual-8237 controller state, scheduling,
and guest-memory transfers; the default FDC retains only a frozen DMA2 binding.
The DMA2 cascade/port probe, FDC boundary gate, and 65/65 GCC/CTest matrix
passed. Artifact `nxvm_0_5_0230.exe` is SHA-256
`512358DD2053FDA21B3B1A4B5BB6E8A29724E9098CCDAFC340423D99917777D0` from
`c30331e`; see [the task record](m5-t230-dma-fdc-contract.md).

**M5 T229 complete:** the bounded 8042 AUX controller has one tagged KBC output
owner, PS/2 three-byte packets, and IRQ12; host mouse enters only through the
profile mapper and existing session ingress. An ordinary guest IVT `74h`
fixture, the retained NXVM behavior matrix, GCC/CTest gates, and artifact
`nxvm_0_5_0229.exe` passed. Its SHA-256 is
`62E7AE972C0D4B433A4842A4756E4D88B36D8F1AEAED2D5F22B883E3297B4ADA` from
`a60b57a`; see [the task record](m5-t229-kbc-aux-mouse.md). No
BIOS/DOS/BDA/guest-memory shortcut is admitted.

**M5 Td S10 complete:** defect and compatibility fixes now have an enforceable
local similar-issue sweep. The active packet and closure evidence must record
the original defect, full relevant-repository query, every production-hit
disposition, focused regression, and a recurring closure gate when mechanical
detection is possible. This is governance-only: no source, build, artifact,
or M5 technical closure item changes.

| Requirement | State | Evidence or decision |
| --- | --- | --- |
| R1: retain a local enforceable rule | Verified | Execution Policy defines mandatory sweep fields and a no-undocumented-hit closure condition. |
| R2: make task packets auditable | Verified | Execution Workflow requires the sweep record at intake and at closure. |
| R3: avoid unrelated cleanup | Verified | Out-of-scope hits require a recorded deferral or owner-approved scope change. |
| R4: preserve public-skill division | Verified | Local text names ntvdm64 evidence and gates; general discovery practice remains in the public skill. |

**M5 Td S11 complete:** make governance self-contained and remove duplicated
type-vocabulary authority. Original request: “Resolve every audit finding and
make a `Td` commit. This command also follows the governance specifications in
the project and public Skills.”

| Requirement | Owner | State | Evidence or decision |
| --- | --- | --- | --- |
| R1: local execution works without a Skill install | M5 Td S11 | Verified | Execution Workflow now defines the mandatory local lifecycle and closure contract. |
| R2: make all execution authorities discoverable | M5 Td S11 | Verified | `AGENTS.md` and the documentation guide name the workflow and Execution Policy. |
| R3: remove overlapping `type.h` rules | M5 Td S11 | Verified | C-Library Facade is the sole detailed authority; other current docs link to it. |
| R4: close every audit finding | M5 Td S11 | Verified | Link, authority-marker, duplicate-scope, diff, and working-tree audits pass. |

| Dependency or risk | Decision / trigger | Resolution owner | State |
| --- | --- | --- | --- |
| Local workflow could again be replaced by a link | Skills are explicitly supplementary; the local lifecycle is mandatory. | M5 Td S11 | Closed |
| Vocabulary detail could be lost during consolidation | One current facade authority remains; superseded material is non-normative history. | M5 Td S11 | Closed |

**M5 Td S7 complete:** the execution workflow is the seventh principal
documentation entry. Its closure audit verified R1 (entry/navigation), R2
(requirement packet, state machine, and closure audit), and R3 (docs-only,
concise scope). It changes no source, build, artifact, or M5 technical closure
item.

**M5 Td S8 complete:** external governance-skill adoption. Original request:
“让 ntvdm64 的文档去引用它，不要丢失自己的细节（比如任务如何编号：MTSP
规则，比如自己的架构怎么设计的），但是和 skill 重复的部分直接改为 skill
的引用并在 ntvdm64 本身删掉重复内容；引用的时候，只引用公开的 GitHub
链接，不引用本地硬编码路径。”

| Requirement | Owner | State | Evidence or decision |
| --- | --- | --- | --- |
| R1: use public links only | M5 Td S8 | Verified | All Skill references use public `https://github.com/cshaxu/skills/...` links. |
| R2: retain local detail | M5 Td S8 | Verified | MTSP, artifact, roadmap, dual-product, module, toolchain, and acceptance decisions remain local authorities. |
| R3: remove duplicated general rules | M5 Td S8 | Verified | Principal local standards cite skills and remove their duplicated generic method text. |
| R4: close against the original request | M5 Td S8 | Verified | Link, scope, local-detail, duplicate, and working-tree audits pass. |

| Dependency or risk | Decision / trigger | Resolution owner | State |
| --- | --- | --- | --- |
| Local detail could be lost during consolidation | Reviewed every changed document against its project-specific authority. | M5 Td S8 | Closed |
| A local filesystem link could leak in | Final audit rejects every filesystem-local skill reference. | M5 Td S8 | Closed |

**M5 Td S9 complete:** retire root and source-local governance readmes. Original
request: “The source-local README is no longer retained; move its content into
the coding standard and repair every reference. Do the same for the root rule
document:
fold project-specific rules into architecture, coding, and execution-governance
documents, and discard material already owned by an external Skill.”

| Requirement | Owner | State | Evidence or decision |
| --- | --- | --- | --- |
| R1: retire source-local README | M5 Td S9 | Verified | Its source-layout content is in `docs/coding-standard.md`; the source-local file is deleted. |
| R2: retire root rule document | M5 Td S9 | Verified | Architecture, coding, source, and execution authorities retain local detail; the root file is deleted. |
| R3: repair references | M5 Td S9 | Verified | Repository-wide scan finds no reference to either retired filename. |
| R4: discard Skill-owned duplication | M5 Td S9 | Verified | Generic method text remains in public Skills and was not re-homed. |
| R5: close against the original request | M5 Td S9 | Verified | Absence, authority coverage, local links, scoped diff, and clean-tree audits pass. |

| Dependency or risk | Decision / trigger | Resolution owner | State |
| --- | --- | --- | --- |
| A deleted rule could lose project-specific detail | Covered by architecture, coding, source, execution, and M5 closure authorities. | M5 Td S9 | Closed |
| Historical records could retain a misleading live reference | Repository-wide scan found none. | M5 Td S9 | Closed |

**M5 T227 complete:** core KBC owns the bounded controller/keyboard protocol:
set-1 query/select, observable translation bit, LEDs, typematic, ACK/RESEND,
and elapsed-tick command-response delay. Set-2/3 conversion and AUX/IRQ12
remain explicitly unsupported. Command, timing, DOS keyboard, retained
Console/debugger, and 59/59 GCC/CTest evidence pass. The `0.5.0227` artifact
SHA-256 is `9DE95F8E2CC55B554404D5EC9B170526E5F983F85E03A3E4FBFD5F5F39374E65`.
See [the task record](m5-t227-kbc-controller-protocol-phase2.md).

**M5 T226 complete:** host key down/up now follows the only guest input path:
platform ingress -> default-profile set-1 mapper -> core KBC FIFO/IRQ1 -> ROM
`INT 09h` -> BDA -> `INT 16h`. The phase covers normal/E0/E1 make/break and
rapid typeahead without a host queue or direct BDA mutation. Mapper, KBC,
prompt and DOS keyboard probes plus the 59/59 current GCC/CTest matrix pass.
The `0.5.0226` artifact SHA-256 is
`2FE4D3833409BC9037FD13CFC7EB4DDC6F14CD83CE8CA75A623FDB7DE050B292`.
See [the task record](m5-t226-kbc-guest-input-phase1.md).

**M5 T225 complete:** the S7 closure locks cold-reset divider-remainder
disposal, paused-session time freeze, one-step elapsed-tick advance, and
reset-origin period repetition without changing device behavior. The current
GCC/CTest matrix passes 59/59 and the reverified `0.5.0225` artifact retains
its recorded SHA. KBC work remains deferred to T226. See [the task
record](m5-t225-time-pit-closure.md).

**M5 T222 complete:** the core PIT owns effective binary/BCD reload, GATE edge
handling, modes 0--5 waveform state, read-back, and OUT-level delivery to the
PIC IRQ0 source. The focused waveform/read-back probes and 57/57 current
CTest matrix pass; the T222 `0.5.0224` artifact and relative-rate evidence are recorded
in [the task record](m5-t222-pit-waveform.md). The scheduler remains the sole
time source; no host timer or VM-side device loop is admitted.

**M5 T223 complete:** default-profile keyboard delivery again preserves BIOS
extended keys with a zero ASCII byte and consumes one ingress event per guest
execution boundary. This restores real `EDIT.COM` Alt-menu and arrow behavior
without a QDX opcode, firmware portal, or a second input route. The owner
confirmed the `fdd.img` interaction; the 56-test current CTest matrix passes.
See [the task record](m5-t223-keyboard-set1.md).

**M5 T221 complete:** deterministic reset-vector checkpoint sequences now lock
the core tick, run-result, and `3DAh` raster behavior across one default
period. The 56/56 current CTest matrix passes. See [the task
record](m5-t221-deterministic-calibration.md).

**M5 T220 complete:** VADP owns frozen PC/AT text-raster timing and derives
`3DAh` display-enable/retrace states only from core elapsed ticks. Focused
probes and the 55/55 current CTest matrix pass. See [the task
record](m5-t220-cga-text-raster.md).

**M5 T219 complete:** `core_machine_run()` is the sole elapsed-tick scheduler;
after each completed instruction it advances DMA, PIT, VADP, then PIC visibility
in a fixed order. No budget-exhausted or faulting instruction advances time.
Focused probes and the 55/55 current CTest matrix pass. See [the task
record](m5-t219-elapsed-tick-scheduler.md).

**M5 T217 complete:** `core_machine` solely owns elapsed guest ticks and the
frozen per-instruction clock parameter; run budgets, reset origin, result, and
observation semantics are covered by a focused smoke plus 53/53 current CTest
coverage. See [the task record](m5-t217-core-machine-time.md).

**M5 T218 complete:** the retained real-mode 8086/80186 CPU corpus now has
actual reset-vector instruction/tick probes for MOV, port I/O, INT, segment
prefix, HLT, and rejected 386 prefix behavior; 54/54 current CTest coverage
passes. See [the task record](m5-t218-real-mode-cpu-ticks.md).

**M5 T216 complete:** PIT, KBC, FDC, and HDC signal IRQs through the core PIC
source boundary. The PIC alone owns edge/level delivery, IRR/ISR, mask, EOI,
priority, and cascade; S4 also locks level-mode IRQ14 re-presentation through
slave/master double EOI. S5 additionally repairs rapid keyboard delivery, the
ROM `INT 16h` return-FLAGS frame, deterministic CGA text-status retrace, and
the normal-path `NEW CODE PATH` Console leak; real FDD `EDIT.COM` launch now
passes. The 52-test current matrix passes. See [the task
record](m5-t216-pic-irq-lifecycle.md).

**M5 T215 complete:** the unused generic firmware-interrupt portal API,
executor storage, dispatch branch, portal-only smoke, and closure gate are
deleted. Ordinary CPU `INT` followed by IVT delivery is the sole interrupt
path; 8086/80386 focused coverage and all 51 current CTest smokes pass. See
[the task record](m5-t215-remove-firmware-interrupt-portal.md).

**M5 T214 complete:** CPU/machine faults now return stable `STOP_FAULT` with
detail and diagnostic before the normal reset path. The VM session owns one
copied outcome for retained Console/debugger inspection; only explicit reset
clears it. The runner-level smoke and all 51 current CTest smokes pass. See
[the task record](m5-t214-cpu-fault-outcome.md).

**M5 T213 complete:** the default profile now owns a primary master-only ATA
PIO HDC at `1F0h`--`1F7h` and `3F6h` with IRQ14. ROM `INT 13h` performs PIO
through that controller, not the old F4/F5 helpers; the bounded HDD-image
smoke reaches the active partition VBR after two reads. See [the task
record](m5-t213-hdd-controller-profile.md).

**M5 T211 complete:** the default profile's F0
boot-failure portal is retired. The ROM reports an acknowledged failure through
its BDA POST status; the VM session runner consumes it and requests the sole
core stop. See [the task record](m5-t211-boot-failure-portal-retirement.md).

**M5 T210 complete:** the default profile's F1/F3
keyboard portals are retired. The retained input path is KBC -> IRQ1 -> ROM
`INT 09h` -> BDA -> ROM `INT 16h`; platform input still submits only host
events. See [the task record](m5-t210-keyboard-portal-retirement.md).

**M5 T209 complete:** the default profile's QDX
opcode override is replaced with frozen, ROM-origin-limited private `INT`
portals. Standard `INT`/IVT semantics remain intact outside a matching frozen
portal. The default machine remains `80386 + no FPU`; its focused strict-8086
diagnostic stops at a genuine 80186 instruction in the DOS image. See [the
task record](m5-t209-firmware-interrupt-portal.md).

**M5 T208 complete:** the default PC/AT profile is an immutable declaration
interpreted by composition. Its ROM mapping, firmware service order,
CMOS/FDC port configuration, and FDC IRQ/DMA route no longer reside in session
wiring or VM device implementation. The retained startup, Console, debugger,
and DOS-prompt paths are unchanged. See
[`pc-at-profile.md`](../architecture/pc-at-profile.md).

M5 remains open. `M5 Td S2` clarified documentation entry points only; it did
not close any M5 technical or verification item. The earlier documentation-only
commit labels `T208` and `T209` are reclassified as `M5 Td S1` and `M5 Td S2`;
they do not reserve numeric implementation task identifiers.
`M5 Td S3` records the ROI-ordered NXVM PC/AT workstream in `TODO.md`; it does
not activate an implementation task or change M5 closure state.
`M5 Td S4` establishes the required hardware-device verification template:
every admitted device task has S1 contract/port probe, S2 owner-local
implementation, and S3 DOS/system-image regression, with a fixed retained
NXVM matrix and bounded optional Bochs differential rules. It changes no guest
behavior and produces no artifact.
`M5 Td S5` closes the T212 governance record and reconciles stale active-work
and TODO wording with its completed artifact and current-gate evidence. It
changes no source or build output.
`M5 Td S6` establishes the authoritative ROI-ordered PC/AT hardware queue,
corrects the baseline to T216 S5, and reserves T217--T233 for time, devices,
storage, and display work. T234 was allocated to the boundary closure, which
is complete; the unstarted EGA/VGA and CPU/FPU work follows as T235--T241.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M11 | Not started | [Roadmap](roadmap.md) |

## Latest Technical Baseline

M5 T216 retains NXVM behavior while making core PIC state the sole owner of
device IRQ delivery. T217 S1 now defines the next design boundary: one
core-owned guest-time model before device timing evolves. The recorded T216
developer artifact is `nxvm_0_5_0216.exe`; its historical evidence is
summarized in [M5 History](../history/m5.md).

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Roadmap](roadmap.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)

Completed task-level records are historical context, not current authority.
