# T313: Construction Failure Atomicity And Startup Failure Visibility

## Admission

T313 is the next linear task after the accepted T312 withdraw and M5 direct
80386 package audit. It addresses four independent but connected construction
and startup failure boundaries. Existing port ownership and session-initialize
status propagation are retained; this task does not reopen them.

## Ordered Subtasks

| Subtask | Scope | Deterministic proof and stop boundary |
| --- | --- | --- |
| S1 | Freeze contracts, seams, owners, and source inventory only. | Documentation governance and diff check; no runtime code. |
| S2 | Resolve default versus explicit RAM once during `core_machine_create`; add a test-only deterministic allocation-failure seam without changing raw `t_ram` fixture ownership. | Default and explicit allocation each occur once; failure returns exact status and no machine escapes. Stop before ports/devices. |
| S3 | Make typed port range registration and core device assembly transactional on allocation failure. | Mid-registration failure leaves no port owner, no partial binding, and `core_machine_create` returns failure after cleanup. Stop before VM platform work. |
| S4 | Return a minimal precise status from session resume/start and retain it as session fault/start outcome; surface it at the existing Console boundary. | Forced `vm_platform_start` failure is observable to the selected session and Console, with no active run handle or false running state. Stop before new CLI or generalized product APIs. |
| S5 | Replace product-to-core debug enum casts with private explicit mappings and unknown-value rejection. | Focused mapping proof covers every known value and unknown rejection. The existing void watch vtable cannot return status; record the retained command-level error policy or stop for a narrowly admitted contract change. |
| S6 | Closure only. | Build `nxvm_0_5_0313.exe`, record SHA-256, pass focused intersections and `current-gates-gcc`; no new behavior. |

## Contracts

- Machine creation has one resolved RAM byte count and one backing allocation.
  A failure cannot return a live machine without RAM or transiently allocate the
  default backing before an explicit backing.
- A multi-port provider/device installation commits only after every required
  registration allocation succeeds. Failure has no surviving port binding or
  half-created `core_machine`.
- Platform launch failure is a session-owned result, not a silent return from a
  void callback. Existing Console glue remains narrow and displays the retained
  failure outcome.
- Debug product and core enum domains remain separate. A private total mapping
  accepts only named compatible values; unknown values never reach a cast.

## Exclusions

No CPU executor, paging, RETF/IRET, product re-architecture, generic allocator
framework, external-runtime CLI, or M6 capability is admitted. All allocation
failure injection is deterministic test support, never a production global
allocator facade.
