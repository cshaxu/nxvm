# M5 NXVM PC/AT Hardware Convergence

## Scope

This is the ROI-ordered queue for remaining M5 NXVM PC/AT work. It does not
redefine the roadmap, start a later milestone, or define NXVDM completion.
Completed M5 evidence is summarized in [M5 History](../history/m5.md). The
current baseline and active queue head live only in [Project Status](status.md);
this document contains the unstarted continuation after that head. Each admitted task follows the
hardware-device verification template and must preserve that baseline.

## ROI-Ordered Queue

This queue advances the four product goals recorded in [TODO](../../TODO.md).
Tasks are ROI-ordered, subject to their stated dependencies; a task begins with
its own S1 contract and may split when its bounded stop condition is reached. A runnable implementation task uses its
numeric task identifier as its artifact revision when it completes. Task
identifiers are allocated linearly: this table is the reservation order, and a
conditional row must be resolved before any later row may begin.

T270--T300 are closed and archived in [M5 History](../history/m5.md). T301 is
the next reserved numeric task. It may begin only through the normal linear
admission workflow and must not imply that the conditional T302 is admitted.

### A. Core Public-Surface Closure And Mantle Prerequisites

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T301 | Add a stopped/paused, copied-value, prevalidated multi-span ordinary-RAM transaction. | Depends on closed T300. It uses the existing checked route, rejects non-RAM/overlap/overflow, guarantees zero writes on preflight failure, and does not claim concurrent or cycle atomicity or create a second memory route. |
| T302 | Add a transition-local combined commit only if a real consumer requires it: checked staged RAM plus an allowed decoded-transition CPU patch at one execution boundary. | Depends on T300--T301 and a concrete consumer. Otherwise it remains deferred. |
| T303 | Prove second-consumer readiness with a first-party core-only fixture and current NXVM regressions. | Depends on T294--T301, plus T302 only if it is evidence-admitted. It does not create a `mantle` runtime. |

### B. Later CPU, FPU, And Readiness Work

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T304 | 80386 Enhanced-mode control-transition family selected by the current Setup `MOV CR0,EAX` checkpoint, including exact decode, profile gate, `#UD/#GP`, and PE/PG state-transition semantics evidenced by corpus. | Depends on T303. No broad 386-complete claim. |
| T305 | Next 80386 Enhanced-mode prerequisite selected by the next failing prepared-state or system corpus, such as CPL3 paging permissions, 32-bit segmentation/control transfer, or 32-bit TSS behavior. | Depends on T304 and a failing corpus. |
| T306 | Present-FPU extension selected by corpus: 80287/80387 profile state, format, environment, or exception delivery. | An owned reproducer is required. No host floating-point shortcut or blanket FPU claim. |
| T307 | Windows 3.x Enhanced-mode readiness corpus and bounded deferral map. | T285, T304--T306 as dictated by the external checkpoint. No Windows 95 claim. |

### C. M5 Closure And Handoff

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T308 | Current-source M5 closure audit against the closure checklist, including single-owner/path, CMake graph, retained NXVM UX, core second-consumer readiness, Windows checkpoint evidence, and all remaining deferrals. | T264--T307. M5 may close only if the checklist, roadmap and TODO agree; otherwise this task records the next bounded M5 queue rather than declaring closure. |

## Later-Milestone Handoff

- **M6:** implement the neutral `mantle` session envelope only after T303 has
  proved the cleaned core consumer boundary; no DOS policy or external VDM
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
- `core/machine` owns generic guest state, elapsed time, shared-device storage,
  and initialization/teardown order. VM/profile supplies only frozen topology,
  providers, ROM/firmware contents, and PC/AT policy; platform never mutates
  guest state.
- Do not add a second executor, machine/session, device-state mirror,
  VM-side instruction loop, host-clock guest shortcut, global/TLS selector, or
  unapproved NXVM Console/debugger/startup/boot behavior change.
- A behavior-changing task records its focused evidence, retained regression
  matrix, source commit, and verified task artifact before completion.
