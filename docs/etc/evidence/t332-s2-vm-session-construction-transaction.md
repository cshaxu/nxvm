# T332 S2: VM Session Construction Transaction

## Mechanism Map

`vm_session_create` owns allocation, profile/config preparation, session
initialization, late image insertion, optional blank-device creation, boot
selection, reset, and final publication through `out_session`. Its stages and
failure owners are:

| Stage | Failure boundary | Rollback owner |
| --- | --- | --- |
| Allocation, profile selection, and config materialization | No constructed machine exists. | `vm_session_create` frees the private allocation. |
| Core creation, display binding, profile-port discovery, display/DMA/RTC configuration | Storage exists only through the display slot and core machine. | `vm_session_storage_rollback` finalizes the display slot, destroys the core, and clears the sole core pointer. |
| Media registry, profile context, mailbox, debugger context | Storage is fully initialized. | The lifecycle finalizer owns the next-stage rollback. |
| Control/debug activation, provider firmware, controller configuration, media bind/freeze | Control/provider may have begun after full storage initialization. | `vm_session_initialize` calls `vm_session_finalize`, which finalizes control/provider then storage. |
| FDD/HDD image insertion after initialization | The session is not yet published to the caller. | `vm_session_create` calls `vm_session_destroy`; `out_session` remains null. |
| Optional blank FDD/HDD creation, boot preference, reset | These operations have no failure return. | They run only after the preceding transaction succeeds and before publication. |

The two rollback shapes are intentionally distinct. An early storage failure
cannot call the full lifecycle finalizer because control/provider state has not
been initialized; completed storage plus later stages must use the lifecycle
owner so provider/device finalization precedes storage destruction. The common
early shape is now one private `vm_session_storage_rollback` mechanism.

## Focused Failure Evidence

`vm-session-initialization-atomicity-smoke` retains the existing invalid core,
firmware, and FDC-profile cases and adds an HDC-profile failure, so both
controller configuration stages reach the lifecycle rollback boundary. It
checks failed direct initialization has no active session, no core machine, and
no active platform run handle; a subsequent normal creation succeeds.

The same owner smoke creates configurations with individually nonexistent FDD
and HDD image paths. Each late insertion fails with a null output session,
then a fresh normal session succeeds. This proves the outer construction
transaction does not publish the already-initialized session on either late
media path.

The smoke retains the T300/S1 markers and adds
`M5:T332:S2:SESSION-CONSTRUCTION-TRANSACTION:OK`.

Fresh `mingw-gcc-x64` configuration, the focused smoke, all 49 specialized
verifiers, documentation governance, diff check, and the 212-test
`current-gate` CTest selection pass. The isolated T332 developer artifact is
`build/output/nxvm_0_5_0332.exe`, SHA-256
`0429D3ED2D0D55CB8DB7E8BE0F89344B2FF842C1A0DD25E33A07C4FAD7463D5D`.

## Similar-Issue Sweep

The sweep inspected every `vm_session_create`, `vm_session_initialize`,
`vm_session_finalize`, storage/control/provider lifecycle, machine-device
configuration, media bind, and FDD/HDD insertion failure return under
`src/vm/composition/session/`, plus session callers in `src/` and `tests/`.
Only the two storage early-failure branches duplicated the same teardown and
were converged. Lifecycle and outer create rollback remain distinct by the
initialization-state boundary above. No unclassified VM composition failure
return remains in scope.
