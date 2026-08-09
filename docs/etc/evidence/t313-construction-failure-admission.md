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
| S4 | Make RTC/FDC/HDC configuration rollback whole-controller atomic. | First/second registration failures leave no ports, configuration, connect/data/topology, or IRQ state before retry. Stop before VM platform work. |
| S5 | Return a minimal precise status from session resume/start and retain it as a session-owned start outcome; surface it at the existing Console boundary. | Forced `vm_platform_start` failure is observable to the selected session and Console, with no active run handle or false running state. Stop before new CLI or generalized product APIs. |
| S6 | Replace product-to-core debug enum casts with private explicit mappings and unknown-value rejection. | Focused mapping proof covers every known value and unknown rejection. The existing void watch vtable cannot return status; record the retained command-level error policy or stop for a narrowly admitted contract change. |
| S7 | Closure only. | Build `nxvm_0_5_0313.exe`, record SHA-256, pass focused intersections and `current-gates-gcc`; no new behavior. |

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

## S2 RAM Creation Result

`core_machine_create` now resolves zero `memory_bytes` to the default before
memory initialization and calls a private specified-capacity initializer once.
The retained raw `t_ram` initializer continues to allocate its default backing
for direct fixtures, while the create path no longer performs that allocation
before an explicit capacity allocation.

The test-only create seam receives a per-call private allocation observation;
it can fail only the core-owned backing allocation and records attempts. It is
not process-global allocator state and does not change the public memory
interface. The focused `M5:T313:S2:RAM-CREATE:OK` proof covers default and
explicit success, deterministic default and explicit failure, retained
freeze/reset/destroy, and direct raw `t_ram` allocation.

## S3 Port And Device Assembly Result

The one directional port registry now supports a private registration
checkpoint. Typed range registration rolls back every direction and port added
after that checkpoint when an allocation fails; duplicate directional owners
remain rejected without replacement. Create-time legacy device registration
uses the same first-failure status and checkpoint before publishing a machine.
FDC initialization, plus multi-step RTC and HDC configuration, roll back their
own checkpoint on a later registration failure.

The test-only port allocation observation is stored on the individual `t_port`
operation and is passed only through the private create test seam. It has no
process-global allocator or public fault-injection API. The focused
`M5:T313:S3:PORT-ASSEMBLY:OK` proof covers a read/write range mid-failure with
no surviving binding, retry and first-owner preservation, create failure with
a null output, FDC configuration rollback, and fresh default creation after
each injected failure.

## S4 Controller Rollback Result

RTC now registers its index and data providers before it initializes embedded
RTC storage, writes defaults, or publishes its configuration. A first- or
second-registration failure therefore rolls back the complete checkpoint while
the RTC/configuration fields remain their clean stopped-machine values.

The S4 sweep also found that FDC finalization only deasserted DMA/IRQ, and that
HDC rollback left `hdc_topology` copied after controller finalization. FDC
finalization now clears data and connection state; HDC failure clears its
topology after the existing private finalizer clears controller state. The
focused `M5:T313:S4:CONTROLLER-ROLLBACK:OK` proof directly checks RTC first and
second failure, FDC and HDC port/controller/topology clean state, retry, and a
fresh default create after each injected failure.

## S5 Startup Failure Visibility Result

`vm_session_start` and `vm_session_resume` now return the exact
`vm_platform_start` status and store it in a small session-owned outcome. Reset
clears the outcome; every subsequent start or resume rewrites it with its
result. This state is separate from the guest CPU fault diagnostic. A failed
platform launch leaves the run handle inactive and cannot enter the automatic
display-promotion loop.

The existing selected-session Console provider now returns that same status for
`START` and `RESUME`; the retained Console prints `START failed: <status>.` or
`RESUME failed: <status>.` immediately. The dedicated Win32 stage seam is run
for a window-stage and a Console-stage failure. The focused
`M5:T313:S5:START-OUTCOME:OK` proof captures the real Console output and checks
the selected session's exact retained result, inactive handle, and stopped
control state. The retained run-handle smoke proves reset clears the result and
a later successful window start records `TYPE_STATUS_OK`.

## Exclusions

No CPU executor, paging, RETF/IRET, product re-architecture, generic allocator
framework, external-runtime CLI, or M6 capability is admitted. All allocation
failure injection is deterministic test support, never a production global
allocator facade.
