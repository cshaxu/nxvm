# M5 T252: VM Composition Lifecycle Closure

**Status:** S1 active.

## Goal

Make VM composition the sole owner of one lifecycle sequence: create,
configure, freeze, start, request stop, join, finalize, destroy. It owns the
run handle; core remains the machine owner and platform remains a borrowed
backend. No Console, debugger, boot, or scheduler behavior may change.

## Subtasks

### S1: Contract And Path Inventory

Map every create/reset/resume/stop/finalize/destroy caller and name the unique
creator, stop requester, joiner, and finalizer. Reject any second session,
machine, scheduler, or direct backend teardown path.

### S2: Consolidate Lifecycle Ownership

Factor only the existing composition-owned stop/join/finalize sequence into
explicit helpers. Preserve Console's synchronous join and window's asynchronous
stop behavior while routing both through the same run-handle ownership rule.

### S3: Verify Closure

Add focused lifecycle ordering coverage; run current GCC/CTest, Console,
debugger, DOS, FDD/HDD, graphics, and artifact verification. Produce the next
runnable artifact.

## Stop Conditions

Stop for owner direction if a change needs a new run loop, second machine or
session, product policy, guest-time mutation, or altered NXVM user experience.

**S1 marker:** `M5:T252:S1:COMPOSITION-LIFECYCLE-CONTRACT:OK`.
