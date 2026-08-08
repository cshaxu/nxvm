# Project Status

## Current Work

### M5 T295 S1 -- core-owned CPU/PIC/lifecycle initialization

**Objective:** move the retained NXVM CPU execution-to-shared-PIC initialization
and lifecycle authority fully into `core_machine`; VM composition may submit
typed configuration/providers only and must not borrow CPU execution or PIC
storage to initialize or bind that path.

| Requirement | Completion condition | Evidence / risk boundary |
| --- | --- | --- |
| Core owner | `core_machine_create`, freeze, reset, and destroy own the CPU/instruction/execution/PIC initialization, binding, reset, and finalization order. | Focused owner/lifecycle regression plus CPU/PIC corpus. |
| VM composition | `vm_session_storage_initialize` no longer borrows CPU execution or PIC storage and performs no direct PIC binding. | Production source sweep/static gate. |
| Retained behavior | Run/stop/reset, Console/debugger, FDD/HDD boot behavior remain unchanged. | Existing focused product corpus; debugger raw borrows remain T298/T299. |
| Exclusions | No VADP, DMA, RTC/CMOS/NMI, FDC/HDC migration (T296); no debugger UX/raw-borrow replacement (T298/T299); no scheduler, VM mirror, or profile policy in core. | Source review and deferred-hit record below. |
| Artifact | Build `nxvm_0_5_0295.exe`, copy it to `build/output/`, record SHA-256 and source commit. | `current-gates-gcc` and artifact verification. |

**Frozen order:** create initializes CPU/instructions/execution, then core port/bus,
RAM, shared PIC pair, and binds the execution context to that owned pair before
the configuration window is exposed. Freeze freezes provider/memory topology;
reset resets CPU/FPU, port/RAM/devices, PIC, then clocks/provider and reaches
`STOPPED`; destroy finalizes controllers, PIC, execution, port, RAM, bus, and
trace in the existing core-owned order. The typed execution provider remains an
external time/service provider and is not a scheduler.

**Applicable rules:** `module-layout.md` keeps CPU/PIC/execution in
`core/machine`; `contracts.md` permits only typed configuration/provider
bindings during configuration and one `core_machine_run` loop; coding standard
requires narrow C changes and tests; source policy permits only existing
first-party NXVM code; execution policy requires a source sweep, focused
regression, developer artifact, and full current gates. No exception requested.

**Source sweep (before):**

| Query / production hit | Disposition |
| --- | --- |
| `core_machine_cpu_execution_context_bind_pic` | `src/vm/composition/session/session.c:135` is T295's direct wiring; move into core create. |
| `core_machine_configuration_cpu_execution_borrow`, `core_machine_configuration_shared_pic_*_borrow` | `session.c:132-134` are T295 direct init borrows; remove. `machine_devices.c` RTC/FDC/HDC PIC hits are T296 deferred. Tests/debug hits are test-only or T298/T299 deferred. |
| CPU/instruction configuration borrows | `session/control.c:204-205` feed retained debugger initialization; T298/T299 deferred unless removal becomes unavoidable (not planned). |

**Planned verification:** focused owner/lifecycle CPU/PIC reset regression and
static product-source check; retained CPU/protected-mode/IRQ/PIC,
Console/debugger, and FDD/HDD boot corpus; `git diff --check`, documentation
governance, `current-gates-gcc`. Stop for any necessary T296/T298/T299 scope
change, new scheduler/machine, profile-policy move, media change, or failed
retained behavior.

**Implementation evidence (in progress):** core create initializes the shared
PIC pair and immediately binds the owned CPU execution context; VM storage no
longer performs that borrow/bind. `core-machine-cpu-pic-lifecycle-smoke` emits
`M5:T295:S3:CORE-CPU-PIC-LIFECYCLE:OK`; the narrow static gate emits
`M5 T295 CPU/PIC lifecycle authority: OK`. The post-change sweep leaves no
CPU/PIC initialization/bind borrow in `session.c`; the three remaining
`machine_devices.c` PIC routes are RTC/FDC/HDC and remain deferred to T296;
the control/debugger CPU borrows remain deferred to T298/T299. With the
owner-provided FDD/HDD images supplied read-only as CMake cache inputs,
`current-gates-gcc` passed all 122 current smoke tests and all configured
static/documentation gates. `git diff --check` passed. The developer artifact
below will be rehashed after its source commit is recorded.

## Current Technical Baseline

- **T295 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0295`. Artifact
  `nxvm_0_5_0295.exe` SHA-256:
  `52B291B1E1100D945BD44B7B1F88A622F7B2B7D3468BC78997ED90732BCA179A`.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, immutable ROM
  mapping, and validated real-mode entry plans. The obsolete post-`#UD`
  transition has no public or runtime path.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T287 | Fixed bounded ROM CHS device/head, AH=08h caller-pointer, FDISK text-service/window-clear, and HDD-only boot-selection defects; external DOS registers C:, presents stable FDISK copied frames, and hands off from HDD-only ATA boot, while the Standard-mode checkpoint remains a research result, not a Windows support claim. |
| T288 | Resolved the external Setup post-copy `#CE` with the bounded 16-bit protected-mode call-gate and outer-`RETF` path; core and real replay now reach the next `MOV CR0,EAX` `#UD` checkpoint without claiming Windows support. |
| T289 | Materialized the default PC/AT ROM image before provider freeze, including its A20 reset alias, while reset restores only IVT/BDA and mutable device tables; ROM, boot, Console, debugger, and display regressions pass. |
| T290 | Replaced the FDC single-media binding with frozen drive slots and exact DOR/unit selection, proven through core ports, VM composition, and DOS FDD0 regression without broadening FDC behavior. |
| T291 | Made FDC generation, disk-change and motor/DMA cancellation per frozen drive slot, with core port and DOS FDD0 evidence while leaving timing, UI, and commands deferred. |
| T292 | Added prepared-state evidence that selected 67h REP comparison, scan, and segment-override forms already conform; no CPU behavior or artifact changed. |
| T293 | Removed the unused post-`#UD` transition surface, made protected outer `RETF`/`IRET` frame validation non-mutating until commit, and completed its exception/atomicity matrix: non-present returned `CS` is `#NP`, non-present `SS` is the retained terminal `#SS`, and invalid entries are `#GP`; all six `RETF`/outer-`IRET` cases preserve the defined pre-commit boundary. |
| T294 | Completed the codebase-driven public-surface/raw-borrow matrix, fixed T296's three stages and T297's future firmware-capability lifecycle precondition, and left T300/T302 conditional; no runtime behavior, CMake graph, or artifact changed. |

Detailed contracts, commands, artifact provenance, and prior closures are in
[M5 History](../history/m5.md) and Git history. The [M5 convergence queue]
(m5-pcat-hardware-convergence.md), [roadmap](roadmap.md), and
[M5 closure checklist](m5-closure-checklist.md) define current M5 scope; an
active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S33:** corrected the external-consumer boundary and shifted the
  former forward queue to T279--T287.
- **M5 Td S34:** repaired queue wording and inserted the approved T279--T283
  reliability/interface package.
- **M5 Td S35:** tightened T279, T280, T282, and T283 task contracts.
- **M5 Td S36:** recorded the WASM media-backing admission boundary.
- **M5 Td S37:** made M5 history static, capped closure summaries, and
  hardened the mojibake governance check.
- **M5 Td S40:** inserted the approved T293--T303 core public-surface closure
  and shifted remaining Windows CPU/FPU/readiness/closure work to T304--T308.
- **M5 Td S41:** retired completed planning detail, made TODO open-only, removed
  tracked local-path samples, and added recurrence gates for those boundaries.
- **M5 Td S43:** superseded the unimplemented T291 S4 admission by merging its
  protected-return fault-atomicity scope into active T293 S1, which retains its
  original post-`#UD` transition-removal scope. T291 remains closed FDC history;
  the unstarted continuation remains T294--T308.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Project Status](status.md) and [M5 convergence queue](m5-pcat-hardware-convergence.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
