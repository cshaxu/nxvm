# M5 Architecture-Boundary Debt Closure

## Purpose

Close every open architecture and coding finding recorded by the manual
repository audits Td S125--S130, plus the T446 recorder test-build exception
introduced while repairing its lifecycle. This is one coverage-bearing
candidate: it does not claim general repository perfection. Its finite
universe is exactly the findings and scope extensions enumerated below.

The result must reduce, rather than rearrange, mutable-layout exposure, raw
object escape, duplicate production routes, reverse dependencies, forwarding
layers, and test-only production interfaces. Each retained route requires a
real host, layout, lifetime, or semantic distinction and recorded proof.

## Coverage Universe And Completion Rule

The task ledger consists of the following audit records and their named rows:

- Td S125 A--I: machine coordinator; session raw object; VM media, profile and
  platform layouts; VDM forwarding; test coupling; and raw-borrow verifier.
- Td S126 A--E: Core platform state, machine collaborators and plan endpoints,
  debugger context, and host backing resource.
- Td S127 A--D: VM CMake native links, duplicate source ownership, duplicate
  session command authority, and cross-owner tests.
- Td S128 A--C, Td S129 A, and Td S130 A: the stated scope extensions for VM
  product/catalog, Model-40 construction, reverse adapter direction, mutable
  session payloads, keyboard scan-set vocabulary, and unused private includes.
- T446's recorded direct-production-source recorder failure test exception.

The T closes only after every row is accepted with code and focused proof, or
is transferred to a separately approved earlier owner with a concrete
proposal, queue position, and reason that makes completion here unsafe. A
passing structural gate alone is never an accepted disposition.

| Audit row | Receiver | Required repair outcome |
| --- | --- | --- |
| Td S125 A | S10 | Cohesive Core coordinator with distinct mechanisms at their owners. |
| Td S125 B; Td S129 A; Td S125 I | S2 | No raw selected-session pointer or mutable argument payload; verifier covers the eliminated shape. |
| Td S125 C; Td S128 A/C; Td S130 A | S7 | Minimal VM contracts, no mutable media/product layouts, reverse adapter, private KBC constants, or unused private includes. |
| Td S125 D; Td S128 B | S7 | Private profile state and one Model-40 materialization path. |
| Td S125 E; Td S126 A | S5 | Opaque Core/VM platform lifecycle state with bounded host operations. |
| Td S125 F | S9 | No policy-free VDM forwarding layer. |
| Td S125 G | S11 | Retain the already corrected audit-scope record and verify that this T makes no broader unsupported claim. |
| Td S125 H; Td S127 D | S8 | Cross-owner tests use declared operations and copied observations. |
| Td S126 B/C | S6 | Core collaborator and plan endpoint state is not a VM-owned public layout. |
| Td S126 D | S3 | One opaque Core debugger capability and interpreter owner. |
| Td S126 E; T446 exception | S4 | One host backing-resource route; no recorder-specific direct production-source test build. |
| Td S127 A/B | S1 | Host-native links and each production source have one owner target. |
| Td S127 C | S2 | One `SESSION` grammar/authority and VM-specific policy seam. |

## Required Subtasks

### S1 -- Freeze the closure ledger and build ownership

Create the durable per-row ledger, establish pre-change caller/source-target
inventories, then repair the Td S127 CMake findings: native Win32 libraries
belong to their adapter/selected runnable target, `machine_info.c` has one
production target owner, and `VM_RUNTIME_SOURCES` is retired. Add narrow
source-to-target proof. Do not fork composition by host or use linker order to
hide duplicate compilation.

### S2 -- Make product session commands and selected-session access single-owner

Unify `SESSION LIST/OPEN/SELECT/CLOSE` under one product authority while
keeping VM profile policy at VM's explicit seam. Replace the public untyped
selected-session borrow and mutable command/open argument array with a bounded
typed capability or manager-dispatched operation with explicit borrow/copy,
close, and failure semantics. Delete the unused parser and every VM cast of a
raw session object. Extend the raw-borrow verifier with positive and negative
self-tests; do not add aliases, test-only accessors, or a second parser.

### S3 -- Seal the Core product debugger boundary

Make debugger interpreter state owner-private and expose only the smallest
session creation, command, observation, and destruction operations. Migrate VM
composition and focused tests without changing retained debugger UX. Preserve
the Core-to-VM dependency direction and one command interpreter; do not make a
generic console framework.

### S4 -- Converge host backing-resource ownership

Select one owned synchronous backing-resource route for FDD, HDD, media save,
Model-40 BYOB, session catalog, and debugger recording, preserving existing
atomic media-save and source-policy behavior. Retire or narrow the currently
unconsumed parallel resource contract. As part of the recorder migration,
remove the T446 direct-source test-build exception without adding a test-only
public production seam.

### S5 -- Encapsulate Core platform state and VM platform lifecycle state

Make Core backing-resource, input-source, and presentation-mailbox instances
opaque or owner-allocated; migrate VM consumers to bounded create/close,
submit/publish/capture operations and copied frames. Then seal VM platform run,
host-surface, request transport, execution, and virtual-time state behind the
minimal composition-to-platform contract. Preserve the distinct Linux and
Win32 resource lifetimes repaired by T445; do not add a generic host framework
or move host policy into Core.

### S6 -- Close Core machine collaborator and plan endpoints

Move mutable media-registry, display-provider, memory-device, and topology
endpoint state behind Core-owned registrations or private storage. Keep plans
declarative and copied, retain atomic create/reset/destroy behavior, and
migrate default PC/AT, Model-339, and Model-40 together. Do not create a
second machine object, generic plugin framework, or mutable post-create plan.

### S7 -- Normalize VM session, media, profile, product, and composition contracts

Replace cross-owner mutable VM layouts with opaque handles, immutable copied
selection data, and bounded observations. This includes retiring public
`t_fdd`/`t_hdd` storage layouts, console/catalog parser state, profile topology
and Model-40 D4/Core exposure, the public duplicate Model-40 test constructor,
the reverse composition-to-product adapter dependency, private keyboard
scan-set access, and the five unused private includes. Retain exactly one
Model-40 construction/materialization route with copied fixture input; do not
use typedef renames, compatibility aliases, or generic profile machinery.

### S8 -- Repair cross-owner tests and direct-build exceptions

Classify every audited test as same-owner white-box setup or cross-owner
behavioral proof. Move only legitimate setup fixtures to their owner; convert
product/platform/VM integration tests to declared operations and copied
observations, retaining each assertion. Reconcile all exceptions introduced by
S1--S7 with the T345 ledger; no new production source is directly compiled by
a test unless the task records an unavoidable, narrow exception and an
earliest removal receiver. Do not widen production APIs for tests.

### S9 -- Dispose of the VDM forwarding layer

Either remove `vdm/composition/session.c` and consume the machine boundary
directly, or give it one real mantle/DOS assembly responsibility with explicit
lifetime and failure semantics. Preserve the non-runnable VDM scope. No
compatibility wrapper or product promotion is allowed.

### S10 -- Decompose the Core machine coordinator by existing ownership

Reduce `core/machine/machine.c` to assembly/lifecycle coordination by moving
the already distinct timing, validation, scheduling, firmware, board-device,
and fault mechanisms to their existing or one necessary owner-local boundary.
Preserve exactly one Core execution and transaction path, all rollback
semantics, and public behavior. This is not a line-count split and may not
duplicate machine state or introduce a generic framework.

### S11 -- Independent closure audit

Re-run the frozen ledger against actual code, build graph, public headers, and
tests; inspect every changed path; prove each original row's accepted or
transferred disposition; run focused regressions, current smokes, specialized
gates, documentation governance, and a clean artifact build. Report per-S
added/removed/net tracked code and explain any positive net change.

## Shared Constraints

- The architecture and coding rules are hard constraints: one mutable-state
  owner, one production path, dependencies toward Core, minimal public
  contracts, and no forwarding wrappers.
- Migrate all callers of a replaced mechanism in the same S; delete obsolete
  production and retained-path tests instead of preserving compatibility paths.
- Preserve guest behavior, deterministic Core time, media formats, firmware
  provenance boundaries, and existing host-specific lifetime semantics.
- Stop and transfer a row only when an earlier owner or missing authority makes
  the stated repair unsafe; a convenience deferral is not a disposition.

## Verification

Each S records its exact caller inventory, before/after ownership graph,
focused positive and failure regressions, source/build static checks, and code
size accounting. S11 additionally performs the complete ledger review and
current project gates. No generated artifact, owner-supplied firmware, guest
media, or machine-local path may be committed.
