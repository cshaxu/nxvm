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

**S1 inventory:** `vm_session_create()` allocates the one `vm_session` and
`vm_session_storage_initialize()` creates its one `core_machine`.
`vm_session_destroy()` is the sole public destructor. Only
`vm_session_resume()` starts a platform run handle. The remaining historical
duplication is local to `lifecycle.c`: reset, window stop, synchronous Console
resume, and finalization each spell some portion of join/finalize. T252 S2
will make composition-private request-stop and join/finalize helpers the only
backend teardown calls. Platform backend code only finalizes a handle when
composition calls the generic operation.

**S1 marker:** `M5:T252:S1:COMPOSITION-LIFECYCLE-CONTRACT:OK`.

### S2: Consolidate Lifecycle Ownership

Factor only the existing composition-owned stop/join/finalize sequence into
explicit helpers. Preserve Console's synchronous join and window's asynchronous
stop behavior while routing both through the same run-handle ownership rule.

**S2 result:** `lifecycle.c` owns the two private operations
`vm_session_platform_request_stop()` and
`vm_session_platform_join_and_finalize()`. Reset joins a stopped handle;
window stop requests, joins, and finalizes; Console resume remains the
synchronous joiner; session finalization requests stop then joins/finalizes.
No platform backend, Console, debugger, or test acquired a direct teardown
path.

**S2 marker:** `M5:T252:S2:COMPOSITION-LIFECYCLE-MIGRATED:OK`.

### S3: Verify Closure

Add focused lifecycle ordering coverage; run current GCC/CTest, Console,
debugger, DOS, FDD/HDD, graphics, and artifact verification. Produce the next
runnable artifact.

## Stop Conditions

Stop for owner direction if a change needs a new run loop, second machine or
session, product policy, guest-time mutation, or altered NXVM user experience.

**S1 marker:** `M5:T252:S1:COMPOSITION-LIFECYCLE-CONTRACT:OK`.
