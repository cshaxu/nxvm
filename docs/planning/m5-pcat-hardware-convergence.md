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
Tasks are ROI-ordered, subject to their stated dependencies; a task begins with
its own S1 contract and may split when its bounded stop condition is reached. A runnable implementation task uses its
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

The boundary migration consumed T270--T278. The pre-migration future queue
that formerly began at T270 was first displaced to T279--T287; this approved
reliability and interface package consumes T279--T283. T284/T285 then froze
and implemented the mode-10h EGA direct path; T286 closed the single
corpus-proven ATA `nIEN` IRQ-visibility gap; and T287 has recorded external
fixed-drive initialization plus one real-mode 80386 address-size CPU gap. The
remaining queue begins at T288. Historical completed task records retain their
original identifiers.

### B. Reliability And Contract Evidence

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |

### C. Windows 3.x Display And Storage Prerequisites

T287 remains a bounded external readiness corpus. Its fixed-drive BDS geometry
and real-mode 80386 address-size failures are closed as individual admissions.
The current external checkpoint crosses the former post-copy call-gate `#CE`
and stops at `#UD(0)` on `MOV CR0,EAX` while Setup attempts to load Windows.
That next stop remains a separate CPU admission; it does not establish a
Standard-mode checkpoint or pre-admit broad CPU, FPU, or FDC work.

### D. Windows Checkpoint, Firmware, FDC, And CPU Evidence

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T292 | 80386 real-mode high-frequency semantic conformance selected from T288's admission map. | Prepared-state corpus first, with only demonstrated failing forms from a non-overlapping set of `66h`/`67h`, REP variants, segment overrides, address-size stack behavior, or one far-transfer form. No broad 386 claim, paging, LDT, task gate, 32-bit TSS, or FPU work. |

### E. Later CPU, FPU, And Readiness Work

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T293 | 80386 Enhanced-mode prerequisite selected by corpus, such as CPL3 paging permissions, 32-bit segmentation/control transfer, or 32-bit TSS behavior. | A failing prepared-state or system corpus is required. No broad 386-complete claim. |
| T294 | Present-FPU extension selected by corpus: 80287/80387 profile state, format, environment, or exception delivery. | T288 or another owned reproducer. No host floating-point shortcut or blanket FPU claim. |
| T295 | Windows 3.x Enhanced-mode readiness corpus and bounded deferral map. | T285--T294 as dictated by T287. No Windows 95 claim. |

### F. M5 Closure And Handoff

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T296 | Current-source M5 closure audit against the closure checklist, including single-owner/path, CMake graph, retained NXVM UX, Windows checkpoint evidence, and all remaining deferrals. | T264--T295. M5 may close only if the checklist, roadmap and TODO agree; otherwise this task records the next bounded M5 queue rather than declaring closure. |

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
