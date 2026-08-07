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

### A. Machine Foundation And Hardware Timing

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T268 | FDC/FDD fidelity increment. Choose one observed controller gap such as rotation/motor timing, disk-change, multi-drive mechanics, or a deferred command family and prove it through ports, DMA and DOS media. | Requires T265 when the selected behavior is timing-sensitive. No broad controller rewrite. |
| T269 | Level 3 bus-timing contract and first PC/AT transaction corpus: memory/I/O wait states, DMA bus ownership, and device visibility order. | Builds on T265; no prefetch or cycle-exact profile claim. Stop if a device requires an unbounded microstate model. |

### B. Windows 3.x Display And Storage Prerequisites

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T270 | EGA/VGA admission map and failing corpus. Freeze the first Windows-facing mode/register family, its memory map, ROM service boundary, and user-supplied fixture evidence. | Design/probe within the task; do not implement a generic "VGA" layer or bundle guest assets. |
| T271 | First selected EGA/VGA register-family implementation, following T270's corpus; retain VADP as the sole video-state owner and copied platform frames. | T270. Split before DAC, planar/latch, raster, or VBE if not selected by the corpus. |
| T272 | ATA/FDC Windows-startup storage gap selected by a reproducible fixture: controller status/error/reset/timing or a missing transfer form. | T268/T269 as applicable. Image files remain backends, never controller substitutes. |
| T273 | Windows 3.x Standard-mode readiness corpus and gap map using lawful, user-supplied media. Record the exact boot/checkpoint result and convert each unmet prerequisite into a bounded later admission. | T264--T272. This is evidence collection, not a support claim or committed guest asset. |

### C. Protected-Mode And FPU Work Triggered By Evidence

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T274 | First corpus-proven 286/386 compatibility gap from T273 or another owned reproducer. | T273 preferred. One instruction/system family only; no speculative opcode sweep. |
| T275 | 80386 Enhanced-mode prerequisite selected by corpus, such as CPL3 paging permissions, 32-bit segmentation/control transfer, or 32-bit TSS behavior. | T273 plus a failing prepared-state or system corpus. No broad 386-complete claim. |
| T276 | Present-FPU extension selected by corpus: 80287/80387 profile state, format, environment, or exception delivery. | T273 or another owned reproducer. No host floating-point shortcut or blanket FPU claim. |
| T277 | Windows 3.x Enhanced-mode readiness corpus and bounded deferral map. | T271--T276 as dictated by T273. No Windows 95 claim. |

### D. M5 Closure And Handoff

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T278 | Current-source M5 closure audit against the closure checklist, including single-owner/path, CMake graph, retained NXVM UX, Windows checkpoint evidence, and all remaining deferrals. | T264--T277. M5 may close only if the checklist, roadmap and TODO agree; otherwise this task records the next bounded M5 queue rather than declaring closure. |

## Later-Milestone Handoff

- **M6:** implement the neutral `mantle` session envelope only after M5 has a
  stable core boundary; no DOS policy or external VDM backend enters it.
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
