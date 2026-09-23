# T368 S7: 80286 Successful-Retirement Closure Audit

## Decision

T368 closes the 80286 **CPU successful-retirement source ledger** transferred
by T366. It does not close an 80287 implementation, physical bus occupancy,
device service timing, IBM PC/AT 5170 model-L3, or physical cycle accuracy.
Every residual path from S4 now either reaches a source-backed owner or has one
visible, bounded transfer; no residual success is allowed to inherit a claimed
8 MHz clock.

## Final Residual Matrix

| Residual class | Actual route and proof | Disposition |
| --- | --- | --- |
| Unprefixed `LLDT/LTR`, table registers and `CLTS` | S5's protected/real successful fixtures and Intel 210498-005 Appendix B | Exact `17/19`, `11/12`, and `2` source rows at the sole publisher. |
| Defined string/REP and already-admitted segment forms | Existing string, primary and control/stack owners; S6 regressions | Retain their existing source rows; no second prefix charge. |
| Other successful prefix compatibility contexts | S6's source and execution sweep | `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` is an explicit bus/phase transfer, not a scalar claim. |
| `WAIT` and ESC with the selected Model 339 FPU profile `NONE` | The profile descriptor selects `NONE`; S7 ledger proof executes `9B` and `DB E3` successfully and each publishes only the explicit unallocated value. | Transfer to the existing **Broaden present x87** TODO admission. The selected 5170 has no 80287 and no x87 timing model. |
| Optional 8087 experiment and unimplemented 80287/80387 profiles | `core-machine-fpu-8087-smoke` and FPU interface regressions prove their current provider/fault boundaries. | Outside the selected 5170 profile and outside T368. A future coprocessor task must add the processor/profile/operation timing matrix before allocating any source row. |
| `#NM`, `#MF`, `#UD`, profile rejection, invalid ModR/M, only-prefix and other delivered synchronous paths | FPU escape/interface and focused 80286 boundary regressions; the run-loop delivery boundary is before the source-tick publisher. | Non-retiring: zero CPU/device publication for the faulting round. |
| Final classifier fallback | S4 route inventory plus the retained T366 static publisher verifier. | No accepted 80286 primary/control route reaches an unnoticed lookup fallback; known unallocated paths remain named above. |

`FPU_ESCAPE` checks `CR0.EM/TS` before provider dispatch; `WAIT` checks the
`TS|MP` and pending-unmasked-exception paths. The run loop consumes delivered
synchronous faults before calling the sole source-cost publisher. Thus neither
the delivery path nor the handler acquires an instruction-clock policy.

## Verification

- `core-machine-80286-instruction-timing-ledger-smoke` passed with
  `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`; its S7 vectors prove the
  successful no-FPU ESC/WAIT transfer and existing zero-tick negative forms.
- `core-machine-fpu-escape-smoke` passed with `M5:T156:S1:FPU-ESC:OK`;
  `core-machine-fpu-interface-s65-smoke` passed with
  `M5:T316:S65:FPU-INTERFACE:OK`; and `core-machine-fpu-8087-smoke` passed
  with `M5:T262:S3:FPU-8087:OK`.
- The T366 80286 unallocated-publisher verifier, documentation governance and
  `git diff --check` are required for acceptance. The known dormant
  `vm-request-bridge-smoke` source drift continues to block a whole-gate claim;
  it is unrelated recorded TODO debt, not evidence of a passing full build.

## T368 Closure And Transfers

T368 has completed its CPU-ledger prerequisite for the queued bus-timed PC/AT
operation. The remaining source-undefined prefix and ESC/WAIT interface
occupancy move only to the source-backed, profile-specific bus/phase and x87
admissions named above. CPU memory/I/O wait, READY, HOLD/DMA, device service,
prefetch and physical phase remain the later Queue receivers established by
T366. The required 5170 L3 audit, DeskPro 386, 8088/5150/XT and pre-Windows
L3 sequence remain unchanged.
