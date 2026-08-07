# M5 NXVM PC/AT Hardware Convergence

## Scope

This is the ROI-ordered queue for remaining M5 NXVM PC/AT work. It does not
redefine the roadmap, start a later milestone, or define NXVDM completion.
Completed M5 evidence is summarized in [M5 History](../history/m5.md). The
current baseline lives only in [Project Status](status.md); this document
contains only the unstarted queue. Each admitted task follows the
hardware-device verification template and must preserve that baseline.

## ROI-Ordered Queue

This queue advances the four product goals recorded in [TODO](../../TODO.md).
Tasks are sequential; each begins with its own S1 contract and may split when
its bounded stop condition is reached. A runnable implementation task uses its
numeric task identifier as its artifact revision when it completes.

### A. Second Core/Composition Boundary Migration

This phase implements only profile-neutral machine mechanisms and policy-free
host backing primitives. It does not implement `mantle`, an external runtime
adapter, DOS services, host path policy, or a host-filesystem product layer.
Composition alone selects an opaque host resource and adapts it into guest
media; VM retains mount/eject, persistence, media paths, PC/AT topology,
firmware, boot policy, and Console/UI behavior.

The completed migration order was `T270 -> T271 -> T272 -> T275 -> T276` and
`T272 -> T277 -> T278`. `T273` was independent after the media baseline was
understood; `T274` waited for T270, T271, and T273. Task-created registered
temporary media, traces, and logs may be cleaned by their task; generic build
trees, `build/output`, and user files are never cleanup targets.

The already-planned queue is renumbered, not discarded: former T270--T278 are
now T279--T287 respectively. Historical completed task records retain their
original identifiers.

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
### B. Machine Foundation And Hardware Timing

### C. Windows 3.x Display And Storage Prerequisites

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T279 | EGA/VGA admission map and failing corpus. Freeze the first Windows-facing mode/register family, its memory map, ROM service boundary, and user-supplied fixture evidence. | Design/probe within the task; do not implement a generic "VGA" layer or bundle guest assets. |
| T280 | First selected EGA/VGA register-family implementation, following T279's corpus; retain VADP as the sole video-state owner and copied platform frames. | T279. Split before DAC, planar/latch, raster, or VBE if not selected by the corpus. |
| T281 | ATA/FDC Windows-startup storage gap selected by a reproducible fixture: controller status/error/reset/timing or a missing transfer form. | T268--T278 as applicable. Image files remain backends, never controller substitutes. |
| T282 | Windows 3.x Standard-mode readiness corpus and gap map using lawful, user-supplied media. Record the exact boot/checkpoint result and convert each unmet prerequisite into a bounded later admission. | T264--T281. This is evidence collection, not a support claim or committed guest asset. |

### D. Protected-Mode And FPU Work Triggered By Evidence

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T283 | First corpus-proven 286/386 compatibility gap from T282 or another owned reproducer. | T282 preferred. One instruction/system family only; no speculative opcode sweep. |
| T284 | 80386 Enhanced-mode prerequisite selected by corpus, such as CPL3 paging permissions, 32-bit segmentation/control transfer, or 32-bit TSS behavior. | T282 plus a failing prepared-state or system corpus. No broad 386-complete claim. |
| T285 | Present-FPU extension selected by corpus: 80287/80387 profile state, format, environment, or exception delivery. | T282 or another owned reproducer. No host floating-point shortcut or blanket FPU claim. |
| T286 | Windows 3.x Enhanced-mode readiness corpus and bounded deferral map. | T280--T285 as dictated by T282. No Windows 95 claim. |

### E. M5 Closure And Handoff

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T287 | Current-source M5 closure audit against the closure checklist, including single-owner/path, CMake graph, retained NXVM UX, Windows checkpoint evidence, and all remaining deferrals. | T264--T286. M5 may close only if the checklist, roadmap and TODO agree; otherwise this task records the next bounded M5 queue rather than declaring closure. |

## Later-Milestone Handoff

- **M6:** implement the neutral `mantle` session envelope only after T274 has
  proved its admitted core configuration shape; no DOS policy or external VDM
  backend enters it.
- **M7:** implement default PC/AT, Compaq DeskPro 386, and IBM PC 110 profiles
  from the frozen admission contract, with user-supplied legal ROM/media
  manifests and profile-specific corpus. PC 110's 486 requirements become
  separate evidence-led admissions.
- **M8:** implement owned `dos` and `vdm` into `nxvdm.exe`, including the
  approved CLI, containment, display, debugger, cancellation and exit policy.
  External VDM/DOS remains isolated research, not a product dependency.
- **M9 and later:** decide Windows 95/486 and cycle-exact profile work only
  from prior corpus evidence; neither is an implied M5 exit.

## Constraints

- Keep one active subtask. Each task begins with a bounded S1 contract, owner,
  probe, deferred behavior, and stop condition.
- `core/machine` owns generic guest state and elapsed time; `vm/machine` owns
  VM-only devices; profiles provide frozen topology and ROM/firmware contents;
  platform never mutates guest state.
- Do not add a second executor, machine/session, device-state mirror,
  VM-side instruction loop, host-clock guest shortcut, global/TLS selector, or
  unapproved NXVM Console/debugger/startup/boot behavior change.
- A behavior-changing task records its focused evidence, retained regression
  matrix, source commit, and verified task artifact before completion.
