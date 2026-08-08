# Project Status

## Current Work

**M5 T297 S1 active - opaque core-invoked firmware capability.**

T296 is the reference baseline. S1 freezes and implements the firmware
operation whitelist, lifecycle, failure/atomicity, and re-entry contract;
S2 migrates the default ROM and QDCGA consumers; S3 adds the static and
focused evidence, runs the full gates, and produces the `0.5.0297` developer
artifact. Do not admit T298 or later work, close this task, merge, or push.

### Task Packet

- **Original request:** Replace the VM/default-profile firmware's long-lived
  raw core profile binding with an opaque firmware provider/context. During
  configuration core accepts only the opaque provider/context; after freeze
  only core invokes it at explicit firmware-service boundaries. VM/profile
  receives no executor, CPU, RAM, port, or device pointer; no mode/CRx setter
  or direct invocation is admitted.
- **S1 objective and completion condition:** Add one narrow `core/machine`
  firmware provider contract and opaque invocation context. Its complete
  operation enum is checked physical guest-memory read/write, checked port
  read/write, and copied CPU-state read; no state patch is admitted because
  the default ROM/QDCGA consumers have none. A provider binds only while
  `INITIALIZED`; freeze makes the registration immutable; reset retains the
  capability; destroy invalidates it. Core opens an operation borrow only for
  the synchronous callback, rejects all nested/reentrant mutable machine
  operations, and closes the borrow before returning even on failure. Binding
  and freeze failures leave no half-bound provider. Complete S1 with focused
  lifecycle/re-entry probes and a static whitelist gate.
- **S2 objective and completion condition:** Convert default-ROM materialize,
  BIOS reset/BDA update, QDCGA reset, and boot-failure-report handling to
  core-invoked firmware services. Preserve the current ROM map, boot, Console,
  debugger, display, and BIOS behavior; neither the VM session nor profile
  calls firmware callbacks directly after freeze.
- **Reference baseline:** T296, developer artifact revision `0.5.0296`, SHA-256
  `28DBFE1A57EA2A1D53276CA9CED5D9E3A8B742F557C9A13076274AE2067EA02A`.
- **In scope:** the core firmware capability/provider and service boundary;
  default-profile BIOS/QDCGA migration; public contract/architecture updates;
  targeted lifecycle/probe/static checks, current GCC gates, and the T297
  developer artifact.
- **Non-goals:** debugger capability (T298), broad public-borrow removal
  (T299), a generic firmware registry without a consumer, DOS/BIOS semantics
  in core, CPU-state mutation, CRx/mode/IP/segment setters, a new executor,
  machine, scheduler, VM-side instruction loop, or host/product policy.
- **Applicable rules:** `core/machine` owns generic guest state, topology
  freeze, lifecycle, and invocation; VM profile owns immutable ROM and BIOS
  policy. Core must not depend on VM/profile or learn PC/AT/BIOS/DOS meaning.
  Callbacks are synchronous, transfer no raw storage pointer, and cannot
  re-enter mutable operations. No source import, third-party material, guest
  media, Microsoft research, or license/provenance change is involved.
- **Consumer/whitelist audit:** default firmware needs (1) immutable-ROM
  registration during configuration, (2) reset-time checked BDA/IVT/mutable
  ROM reads and writes, (3) reset-time checked CGA/CRTC port writes, and (4)
  post-quantum boot-report checked memory read/write. ROM registration is a
  core-controlled configuration callback, not a runtime service operation.
  No default consumer needs CPU state, state patching, or a CPU/mode/CRx
  operation; copied CPU-state read stays enumerated only if the focused
  provider contract proves its independent consumer rather than creating an
  unconsumed API.
- **Similar-issue sweep:** defect class is a profile/VM firmware context that
  retains or obtains a raw core profile binding, RAM, executor, CPU, port, or
  device pointer, or invokes firmware directly after freeze. Query:
  `rg -n "core_machine_profile_binding|profile_binding_(memory|execution)|context_(memory|execution)|vm_profile_default_(bios_reset|cga_reset)" src tests --glob '*.[ch]'`.
  Every production hit must be migrated, rejected by the new static gate, or
  documented as an explicit out-of-scope future task; test-only fixtures must
  use the capability contract rather than a mirror route.
- **S3 evidence:** `core-machine-firmware-capability-smoke` proves capability
  configure/reset/after-run invocation, configuration freeze, expired-context
  rejection, rejected nested `core_machine_reset`/`request_stop`, completed
  cold reset, and a subsequent run boundary (`M5:T297:S3:FIRMWARE-CAPABILITY:OK`).
  `verify-firmware-capability` rejects raw binding and requires only the
  checked firmware-memory/port and stop operations
  (`M5:T297:S3:FIRMWARE-CAPABILITY-STATIC:OK`). T264 now requires QDCGA's
  firmware-capability memory/port path and forbids direct port/raw binding;
  T211 requires core-invoked capability report consume/request-stop and
  forbids runner direct access. `current-gates-gcc` passed 49 static/build/docs
  gates and 126/126 CTest cases using untracked owner-provided media cache;
  documentation governance and `git diff --check` passed. The rebuilt local
  developer artifact is `build/output/nxvm_0_5_0297.exe`, 2,685,736 bytes,
  SHA-256 `86748E4CC13C28934F3BA3399DBD807B07B3251E44B996E42E9693FCB151D393`,
  produced by the current target with runtime build version `0.5.0297`; its
  executable source is committed as `5d2f1bd1304edcb1dbaa45d4787c4ed57b3c4a69`.
- **Stop conditions:** stop for any need to broaden the whitelist, expose a
  raw pointer, encode BIOS/DOS policy in core, retain a callback borrow beyond
  its call, alter retained product behavior, or begin T298+; record the issue
  for owner direction rather than extending scope.

## Current Technical Baseline

- **T297 active artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0297`. The T296 baseline
  developer artifact was 2,682,261 bytes, SHA-256
  `28DBFE1A57EA2A1D53276CA9CED5D9E3A8B742F557C9A13076274AE2067EA02A`, built
  from `fa18847d0aed685554f786c89ba0f5908e539fb7`; T297's local developer
  artifact is recorded in the active packet and awaits coordinator review.
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
| T289 | Materialized the default PC/AT ROM image before provider freeze, including its A20 reset alias, while reset restores only IVT/BDA and mutable device tables; ROM, boot, Console, debugger, and display regressions pass. |
| T290 | Replaced the FDC single-media binding with frozen drive slots and exact DOR/unit selection, proven through core ports, VM composition, and DOS FDD0 regression without broadening FDC behavior. |
| T291 | Made FDC generation, disk-change and motor/DMA cancellation per frozen drive slot, with core port and DOS FDD0 evidence while leaving timing, UI, and commands deferred. |
| T292 | Added prepared-state evidence that selected 67h REP comparison, scan, and segment-override forms already conform; no CPU behavior or artifact changed. |
| T293 | Removed the unused post-`#UD` transition surface, made protected outer `RETF`/`IRET` frame validation non-mutating until commit, and completed its exception/atomicity matrix: non-present returned `CS` is `#NP`, non-present `SS` is the retained terminal `#SS`, and invalid entries are `#GP`; all six `RETF`/outer-`IRET` cases preserve the defined pre-commit boundary. |
| T294 | Completed the codebase-driven public-surface/raw-borrow matrix, fixed T296's three stages and T297's future firmware-capability lifecycle precondition, and left T300/T302 conditional; no runtime behavior, CMake graph, or artifact changed. |
| T295 | Moved CPU execution-to-shared-PIC binding into `core_machine_create`; VM session no longer borrows or binds that path. CPU/PIC lifecycle evidence and all current gates passed; RTC/FDC/HDC/DMA remain T296 and debugger raw borrows remain T298/T299. |
| T296 | Completed A VADP/ports, B DMA/RTC/CMOS/NMI, and C FDC/HDC ownership migration through frozen typed core submissions. S2 `e84199e`, S3 `a02a0f0`, and S4 `fa18847d0aed685554f786c89ba0f5908e539fb7` passed focused owner/lifecycle evidence and the final 49-gate, 125-test verification; T297--T299 remain deferred. |

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
