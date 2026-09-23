# Td S117: T434 Timing-Plan Convergence And Closure Ledger

`M5:Td:S117:TIMING-PLAN-CONVERGENCE:OK`
`M5:Td:S117:PROFILE-MATERIALIZATION:OK`

## Decision and frozen input

This is the implementation baseline for the queued **T434 Core timing
contract and machine plan**.  It consumes, without re-research or
reclassification, the 30 capability IDs and source/owner decisions in
[T433 S7](t433-s7-core-source-sufficiency-ledger.md).  A row is complete in
T434 only when its named seam is usable by the current production path and its
default disposition is copied and validated.  A later rule task may replace a
named disposition with a sourced L3 rule through that seam; it may not add a
seam, a publication path, an implicit fallback, or a Core/VM ownership rule.

`L2` below means a named, deterministic compatibility disposition, not a
guessed scalar and not a claim of manual timing accuracy.  `NGT` is a
non-guest-time boundary: it is declared only to prove isolation/lifetime and
must never become a guest clock.  A profile which requests unavailable L3 is
rejected before machine publication; it never silently falls back.

## Current machine-publication inventory

The source inspection found exactly three current `vm_session` production
materialization families.  They are all in T434 scope, including the private
BYOB Model-40 family: it is a supported `vm_session` profile kind, although it
is not one of the two console-startup choices.  T434 must migrate all three in
the same cutover batch.

| Family | Current source route | T434 required result and focused proof |
| --- | --- | --- |
| Default PC/AT | `vm_session_create` -> `vm_session_profile_select` -> `vm_session_materialize_profile_core_config` -> `vm_session_initialize` -> `core_machine_create` in `src/vm/composition/session/session.c`; descriptor is `vm_profile_default_pc_at_descriptor_get` in `src/vm/profile/default_profile/pc_at_profile.c`. | Materialize one copied plan, validate it, then publish through the plan-only Core entry.  Prove default creation, configuration failure rollback, and pre/post event, reset, IRQ/DRQ and observation equivalence. |
| IBM 5170 model 339 | Same route, with `vm_profile_ibm_5170_model_339_descriptor_get`; the session selects its virtual-time source after profile materialization. | Materialize the same plan type with Model-339 declarations.  Prove virtual-time source, reset/cancellation and PC/AT isolation remain equivalent. |
| Compaq DeskPro 386 Model 40 BYOB | `vm_session_create` dispatches to `vm_session_create_model40_byob`/`vm_session_create_model40_private`; `vm_session_initialize` dispatches to `vm_session_model40_storage_initialize` in `src/vm/composition/session/model40_composition.c`, which calls `core_machine_create`. | Materialize the same plan type before either private composition can publish.  Prove BYOB validation/rollback, ROM/reset, D4/device routes and default Model-40 behavior remain equivalent. |

Two direct-create classes are deliberately **not** extra production paths:

| Class | Current location | T434 treatment |
| --- | --- | --- |
| Core unit fixtures | `tests/support/core_machine_cpu_fixture.h`, `tests/support/core_machine_executor_fixture.h`, and focused `tests/core`/`tests/machine` fixtures. | Keep only as test-only low-level construction.  T434 must give them an explicit test builder or internal test entry; no test helper may be used by `src/` to publish a product machine. |
| Future VDM | `src/vdm/machine/dos_minimal.c`. | It is outside the retained runnable product, per `CURRENT.md`.  It cannot be cited as T434 production coverage or be converted into an unreviewed second entry.  When that product is admitted, it must consume the already-public plan-only Core entry. |

The post-cutover production invariant is therefore finite and mechanical:
`src/vm/composition/session/{session.c,model40_composition.c}` contain no call
which can publish a Core machine except the one validated-plan publication
entry.  Existing configuration values may be input to VM materialization, but
may not independently create or configure a published machine afterwards.

## 30-row seam and disposition ledger

`Current owner` names the present code domain, not an assertion that its
implementation already meets L3.  The proof column is the focused T434 test
obligation; it supplements the exact source/evidence gate retained by T433.

| ID | Current owner/path | Required copied plan declaration and seam | Default / focused proof | Remaining rule owner |
| --- | --- | --- | --- | --- |
| `CPU-EXEC` | `src/core/machine/cpu*.c`, instruction timing data | `cpu_program`: profile-neutral program/form selector | L2; form selection and invalid selector rejection | Later CPU programs |
| `CPU-EXCEPT` | CPU exception and transaction paths | `cpu_program.exception_delivery` | L2; delivery order and rollback | Later CPU programs |
| `CPU-PREFETCH` | CPU prefetch/external-cycle state | `cpu_program.prefetch` + transaction link | L2; fill/flush/reset cancellation | Later CPU-to-board |
| `CPU-RETIRE` | CPU retirement and `machine_interface.h` qualification | `retirement` observer/qualification seam | L2 infrastructure; successful retirement publication | T434 seam; later formula users |
| `CPU-FPU` | CPU/FPU profile and execution owner | `cpu_program.fpu` personality/disposition | L2; no-FPU/error route and invalid request rejection | Later x87 admission |
| `TIME-CLOCK` | `clock.c`, `timeline.c`, VM virtual-time source | `clock_domains` ratios, source identity and phase | L2 declaration; deterministic order/reset | T434 seam; later board-clock L3 |
| `TIME-LIFECYCLE` | `timeline.c`, reset/finalize paths | `lifecycle` reset/cancel/finalize ownership | L2 infrastructure; cancellation and reset replay | T434 seam |
| `TXN-MEMORY` | `transaction.c`, memory port | `transactions.memory` class/availability/BUSRDY | L2; wait and invalid class rejection | Later CPU-to-board |
| `TXN-PORT` | `transaction.c`, port executor | `transactions.port` class/availability | L2; completion order and invalid decode rejection | Later CPU-to-board |
| `TXN-ARBITRATION` | DMA/HOLD/arbitration paths | `transactions.arbitration` declarations | L2; grant/withdrawal/reset order | Later CPU-to-board |
| `MEM-RAM-A20-PARITY` | `memory.c`, machine RAM/A20/parity configuration | `memory.ram` map, A20 and parity declarations | L2; map/A20/parity reset equivalence | Later CPU-to-board/device phases |
| `MEM-ROM-FIRMWARE` | ROM mapping and `core_machine_config` ROM fields | `memory.rom` immutable mappings/lifecycle | L2 declaration; copied lifetime and invalid mapping rejection | T434 seam; later board access L3 |
| `MACHINE-CONFIG` | `machine_interface.h`, `machine.c` create validation | top-level immutable plan and validation result | L2 infrastructure; deep-copy/atomic failure | T434 seam |
| `CTRL-PIC` | `pic.c` and machine route binding | `devices.pic` topology, IRQ route, phase registration | L2; assertion/withdrawal/cascade reset | Later device phases |
| `CTRL-DMA` | `dma.c` and bindings | `devices.dma` channel, DRQ/HOLD route, phase registration | L2; request/grant/reset | Later device phases |
| `CTRL-PIT` | `pit.c` | `devices.pit` clock input, GATE/OUT routes, phase registration | L2; counter/reset/route ordering | Later device phases |
| `CTRL-RTC-CMOS` | `rtc.c` / CMOS configuration | `devices.rtc_cmos` oscillator/IRQ route, phase registration | L2; register-C/IRQ/reset | Later device phases |
| `CTRL-KBC-NMI` | `kbc.c`, input bridge and NMI route | `devices.kbc_nmi` controller profile/routes/guest phase | L2; FIFO/IRQ/NMI/reset; host ingress isolation | Later device phases |
| `CTRL-FDC` | `fdc.c`, media and DMA/IRQ bindings | `devices.fdc` drive/routes/phase registration | L2; DRQ/IRQ/result/cancel/reset | Later device phases |
| `CTRL-HDC` | `hdc.c`, storage binding | `devices.hdc` explicit unsupported-rule disposition and topology | L2 functional route; required-L3 atomic rejection | Later HDC source admission |
| `MEDIA-BACKING` | media registry/provider lifecycle | `devices.media` immutable logical provider result | L2; attach/eject/error/reset lifetime | Later device/media phases |
| `DISPLAY-VADP` | `vadp.c`, display binding | `devices.vadp` decode/topology/phase registration | L2; port/memory decode and reset | Later device phases |
| `DISPLAY-PRESENT` | display provider and presentation mailbox | `observation.display` NGT declaration | NGT; copied frame lifetime/non-mutation | T434 boundary only |
| `INPUT-HOST` | platform input ingress | `observation.host_input` NGT declaration linked to KBC | NGT; arrival cannot advance guest time | T434 boundary only |
| `TRACE-DEBUG` | Core trace/debug observation | `observation.trace` NGT declaration | NGT; trace does not perturb timeline | T434 boundary only |
| `PLATFORM-MAILBOX` | platform presentation mailbox | `observation.mailbox` NGT declaration | NGT; copy/lifetime isolation | T434 boundary only |
| `PLATFORM-RESOURCE` | platform resource providers | `observation.resource` lifecycle declaration | NGT + Core lifecycle; host latency excluded | T434 boundary only |
| `PLATFORM-WAIT` | platform wait/pacing | `observation.wait` NGT declaration | NGT; reject it as a guest clock | T434 boundary only |
| `SESSION-COMMAND` | session control/console command path | `observation.session_control` NGT declaration | NGT; stop/reset ownership and no guest-clock effect | T434 boundary only |
| `PRODUCT-DEBUG` | product debugger context | `observation.product_debug` NGT declaration | NGT; observation/control non-perturbation | T434 boundary only |

Every row is a T434 seam and default-disposition obligation.  Only the final
column is downstream work.  In particular, `CTRL-HDC` is not deferred: T434
must encode its explicit L2 functional disposition and rejection behavior;
the later task owns only the missing sourced L3 rule.

## Executable implementation batches and closure barrier

The coordinator may allocate the following two implementation batches as S
packets, in order.  They are a bounded breakdown, not pre-created active S
tasks.  Each packet must reproduce this ledger's full exit condition for its
own batch and cannot create a temporary product dual entry.

| Batch | Complete implementation obligation | Required review/proof before its P can close |
| --- | --- | --- |
| B1 - plan internals and every seam | Add the neutral capability/disposition types, copied immutable plan, validation, all 30 declaration seams, plan-aware Core consumers and focused low-level plan tests.  Production remains on the existing sole legacy route during this batch; the new plan has no production publisher yet, so it is not a second production entry. | Architecture review: Core owns algorithms and VM owns selected facts; no profile-name branch/callback enters Core. Coding review: public headers, ownership, error/rollback and copy lifetimes follow project rules. Test proof: every row can be declared/validated; invalid/missing/required-L3 cases fail atomically; L2/NGT are visible; reset/cancel/trace tests pass. |
| B2 - atomic product cutover and deletion | Convert **all three** inventory families to materialize the copied plan and use the one plan-only Core publication entry.  Delete or make private the old publishing route, migrate product-level tests, and isolate low-level test builders. | Architecture review: one VM-to-Core publication boundary, no Core machine identity, no VM algorithm callback. Coding review: no compatibility wrapper that can independently publish; all errors roll back. Product proof: default PC/AT, Model-339 and Model-40 equivalence; validation rollback; IRQ/DRQ, reset/cancel, trace/observation; static call-site sweep proves no production bypass. |

T434 closes only after B2.  There is no B3 "integration audit", no
compatibility grace period, and no later task permitted to connect a listed
consumer.  Discovery of a missing listed seam, a fourth current product
materializer, an ownership contradiction, or a capability required outside
this frozen universe is a T434 stop condition and must receive an architecture
decision before any scope change.  It is not a transfer note.

## Mechanical closure checks for T434

The allocated task must retain and execute these checks in addition to normal
build/test gates:

1. A 30-ID equality check against T433 S7, with exactly one declaration and
   one default disposition per ID.
2. A source call-site sweep of `src/vm` proving that only the validated-plan
   entry can publish a `core_machine`; `src/vdm` and `tests/` are reported as
   the two non-production classes above.
3. Default-plan equivalence for each of the three profile families, including
   deterministic event order, reset/cancel, IRQ/DRQ, trace/observation and
   validation-failure rollback.
4. A Core/VM architecture review and a coding-rule review recorded in the P
   evidence.  A failed review is a closure blocker, not a later cleanup.

This ledger does not allocate T434 or change its Queue position.
