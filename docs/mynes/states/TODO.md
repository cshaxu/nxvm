# Long-Term Review Ledger

## RDP Short-Input Repair

- Priority: P1, transferred from T556 S10 on the owner's 2026-10-13 closure request.
- Owner: MyNES input adapter/controller sampling. Diagnostic delivery 3dc750be2
  proves a character pulse gives repeated reads `08,00` where held Enter gives
  `08,08`; the tested game rejects the inconsistent pulse. No production fix
  or actual mobile native-event capture has been completed.
- Risk: software-keyboard pulses can be lost in repeated controller polling;
  sustained movement also depends on whether the client supplies held keys.
- Admission: a separately approved MyNES repair must define bounded guest-time
  pulse expiry, held-key overlap, pause/reset/source cleanup and snapshot
  compatibility without ROM-specific rules or a second input path. Any needed
  Shared change requires review first. Qualify both widths and the actual
  Window/RDP route before claiming the reported user problem fixed.
- Evidence: [T556 S10 investigation](../etc/evidence/t556-s10-rdp-controller-investigation.md).

## Governance Debt

### Native Console Pause Qualification

- Priority: P2; observed in NXVM-hosted T562 S3 correction on 2026-10-16.
- Owner: MyNES native-console integration qualification. The x86 complete
  suite failed the existing Escape/pause assertion after 10 seconds; unchanged
  isolated and complete 12-test reruns passed. x64 passed initially.
- Risk: intermittent native input/qualification failure; no established
  production cause, and no assertion or timeout was weakened.
- Admission: separately investigate input delivery, broker takeover and machine
  pause observations if reproduced; preserve the original failure evidence.
- Evidence: [S3 correction](../../nxvm/etc/evidence/t562-s3-correction.md).
  T563 S10 also observed the same assertion on x64; the unchanged complete
  35-case receiving integration rerun passed, including all twelve MyNES cases.
  [S10 evidence](../../nxvm/etc/evidence/t563-s10-shared-import.md) retains the
  failure and final result without claiming a native-input repair.

| Priority | Debt | Admission path |
| --- | --- | --- |
| P1 | Battery `.sav` and snapshot `.mns` saves still truncate their destination before writing; write/close failure can destroy the prior disk copy. S12 propagates errors but cannot provide file-level rollback. Owner explicitly defers atomic replacement; current Storage has no file replacement API (medium_replace transfers a lease only). | Separately approved Shared storage capability, then MyNES adoption for both save paths with write/close/replace failure tests. No native API bypass or product-local storage implementation. |
| P2 | T43 S7 fixes synchronous Common machine request admission/completion. The similar-issue sweep still finds ignored event errors in asynchronous lifecycle/input wakeups and Session queue wakeups; Audio control-completion signals also have no independent failure notification. These are different asynchronous/control contracts, not a second synchronous request implementation. Risk: lost wake or stalled control after native synchronization failure. | Owner-reviewed Shared follow-up covering async admission/failure reporting across Common/Lib; no polling or unapproved public API. Dual native-channel failure/thread-join failure remains outside recovery guarantees. |
| P2 | Expose an executable-image-path capability from Lib so App can retain executable-adjacent `mynes.ini` discovery without importing a Win32 SDK declaration. | A future shared-corpus proposal, reviewed and imported through the Lib workflow. |
| P2 | Extend the neutral state-byte reader with a bounded-length/EOF completion fact so Core can reject physical snapshot trailing bytes before committing restore state. | A future Common/Lib state-stream proposal with both receiving products audited. |
| P2 | Shared `library.kvm_window_modal` intermittently observes modal exit before its freeze check (x86 T43 S6 first suite); three unchanged isolated reruns pass. Cause is not established; desktop interaction may affect this native test. Risk: nondeterministic verification, not a proven production regression. | Owner-reviewed Shared test investigation; preserve the failure assertion and separate host interaction from component retirement before changing the test. |
