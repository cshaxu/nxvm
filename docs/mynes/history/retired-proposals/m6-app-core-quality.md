# M6 App/Core Quality

## Outcome

Make the delivered MyNes App and Core paths smaller, explicit and durable. The
work audits only product-owned App/Core code and its direct tests, then repairs
confirmed defects by consolidating state ownership rather than adding parallel
control paths.

## Scope

- App command, configuration and composition lifecycle policy.
- Core machine, driver, cartridge/media and snapshot ownership boundaries.
- Direct App/Core and integration tests that prove the repaired behavior.

## Method

Trace each public App/Core transition from command or driver entry to its
observable completion. Prefer one authoritative state field and one transition
path per behavior. Identify wrappers, callbacks and state carriers that only
forward data without isolating an ownership, thread or ABI boundary; fold them
into the real behavior owner. Replace native-layout persistence with explicit
typed fields where a serialized image crosses builds. Remove redundant code
only where the remaining contract stays directly legible.

## Execution Sequence

1. Completed in S3: compare the four imported shared roots with their SoftPC
   committed counterparts and establish whether an exact re-import is safe.
2. Completed in S3: reconcile all four shared roots to SoftPC commit
   `6251f896`, remove the unneeded atomic-replacement extension and import the
   committed Common nonrunning-window gate.
3. Replace Core's native-layout snapshot image with a compact typed codec and
   direct rollback tests.
4. Rebuild App snapshot dispatch around SoftPC's command-effect and
   runtime-completion pattern, with one bounded byte accumulator.
5. Sweep App/Core again for duplicate state, forwarding-only paths, lifecycle
   ordering and direct-test gaps; close only if no unresolved product-owned
   defect remains.

## Acceptance

1. Every confirmed finding has a severity, owner, evidence and disposition.
2. Snapshot bytes are independent of C layout, padding and pointer width; bad
   input never partially changes a machine.
3. Console success text is emitted only when its claimed machine state has
   actually completed.
4. The repair reduces avoidable indirection and duplicate state instead of
   adding another forwarding layer.
5. The dual-architecture repository suite passes after each repaired S, and
   the T37 developer artifacts are rebuilt when runnable product code changes.

## Exclusions

No Lib/Common audit or modification, new emulator features, mapper expansion,
presenter redesign, ROM corpus changes, or new user commands.
