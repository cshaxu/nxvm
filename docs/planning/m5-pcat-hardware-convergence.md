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

This phase implements only profile-neutral machine mechanisms. It does not
implement `mantle`, an external runtime adapter, DOS services, host path
policy, or a generic host-filesystem layer. `M5 Td S32` records the related
architecture correction: host file/directory/stream capabilities remain above
core until two product forms demonstrate one policy-free contract.

The already-planned queue is renumbered, not discarded: former T270--T278 are
now T278--T286 respectively. Historical completed task records retain their
original identifiers.

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T270 | Replace the fixed single-slot block boundary with a frozen multi-device core media-provider contract: identity, capabilities, logical-sector I/O, geometry, generation, and typed failure. Add a core-only provider fixture. | No host path, file handle, DOS namespace, topology default, or controller moves. Stop if a contract needs product policy. |
| T271 | Make VM FDD/HDD backing objects implement T270's media provider and rebind the existing firmware geometry consumer. Remove the superseded fixed block slot rather than retaining a forwarding facade. | T270. FDD/HDD allocation, mount/eject, image path, persistence, and Console commands stay in VM. |
| T272 | Move the MC146818-compatible controller mechanism to `core/machine` with explicit optional-device configuration and profile-supplied NVRAM image, port/IRQ wiring, clock ratio, and sideband address-line output. | T270 is not required. PC/AT NMI routing, defaults, wall-clock policy, BIOS/POST and BDA services stay in VM profile/composition. Stop if the controller requires a PC/AT default. |
| T273 | Add a core-only mantle-shape configuration fixture: create, bind neutral providers, freeze, reset, apply an entry plan, run bounded slices, and report typed outcomes without any VM header, firmware, UI, or host path. | T270 and T272. This is a fixture, not `src/mantle` implementation; M6 still owns mantle. |
| T274 | Decouple the existing FDC controller from `t_fdd` while it remains in place: consume only the frozen T270 media provider and explicit port/IRQ/DMA wiring. | T270--T271. Preserve the one existing FDC state machine and all retained DOS/FDC regressions; no duplicate controller or multi-drive topology. |
| T275 | Move the now-neutral FDC implementation into `core/machine` and rewire the default PC/AT composition/profile adapter to own only topology, DOR/NMI glue, IRQ6/DMA2 route, media policy, and firmware. | T274. Use `git mv`; remove VM controller code and prove a core-only fixture plus retained FDD/DOS boot. |
| T276 | Decouple the existing ATA/HDC controller from `t_hdd`: use the T270 media provider for present/read-only/range/failure behavior and make all port/IRQ/master-slave choices explicit configuration. | T270--T271. Do not add ATA DMA, new commands, host I/O, or profile defaults in core. |
| T277 | Move the now-neutral ATA PIO controller into `core/machine` and rewire the default PC/AT composition/profile adapter to retain IDE topology, image policy, boot/firmware policy, and UI only. | T276. Use `git mv`; preserve ATA PIO and HDD boot corpus, and add a core-only fixture. |

### B. Machine Foundation And Hardware Timing

### C. Windows 3.x Display And Storage Prerequisites

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T278 | EGA/VGA admission map and failing corpus. Freeze the first Windows-facing mode/register family, its memory map, ROM service boundary, and user-supplied fixture evidence. | Design/probe within the task; do not implement a generic "VGA" layer or bundle guest assets. |
| T279 | First selected EGA/VGA register-family implementation, following T278's corpus; retain VADP as the sole video-state owner and copied platform frames. | T278. Split before DAC, planar/latch, raster, or VBE if not selected by the corpus. |
| T280 | ATA/FDC Windows-startup storage gap selected by a reproducible fixture: controller status/error/reset/timing or a missing transfer form. | T268--T277 as applicable. Image files remain backends, never controller substitutes. |
| T281 | Windows 3.x Standard-mode readiness corpus and gap map using lawful, user-supplied media. Record the exact boot/checkpoint result and convert each unmet prerequisite into a bounded later admission. | T264--T280. This is evidence collection, not a support claim or committed guest asset. |

### D. Protected-Mode And FPU Work Triggered By Evidence

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T282 | First corpus-proven 286/386 compatibility gap from T281 or another owned reproducer. | T281 preferred. One instruction/system family only; no speculative opcode sweep. |
| T283 | 80386 Enhanced-mode prerequisite selected by corpus, such as CPL3 paging permissions, 32-bit segmentation/control transfer, or 32-bit TSS behavior. | T281 plus a failing prepared-state or system corpus. No broad 386-complete claim. |
| T284 | Present-FPU extension selected by corpus: 80287/80387 profile state, format, environment, or exception delivery. | T281 or another owned reproducer. No host floating-point shortcut or blanket FPU claim. |
| T285 | Windows 3.x Enhanced-mode readiness corpus and bounded deferral map. | T279--T284 as dictated by T281. No Windows 95 claim. |

### E. M5 Closure And Handoff

| Task | Owner and purpose | Dependency and stop condition |
| --- | --- | --- |
| T286 | Current-source M5 closure audit against the closure checklist, including single-owner/path, CMake graph, retained NXVM UX, Windows checkpoint evidence, and all remaining deferrals. | T264--T285. M5 may close only if the checklist, roadmap and TODO agree; otherwise this task records the next bounded M5 queue rather than declaring closure. |

## Later-Milestone Handoff

- **M6:** implement the neutral `mantle` session envelope only after T273 has
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
