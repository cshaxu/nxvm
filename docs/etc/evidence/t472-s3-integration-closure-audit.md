# T472 S3: Integration Closure Audit

`M5:T472:S3:INTEGRATION-CLOSURE:OK`

## Result

T472 replaces the former fixed Standard HLT delay with one honest pacing
contract. The selected Rev-3 Model-339 uses its immutable 8,000,000 tick/s L2
macro rate; Standard waits only when Core has already progressed ahead of that
budget, and Turbo removes only that wait. Default PC/AT and Model-40 have no
selected rate and run without host pacing. No host input writes Core time.

The runner HLT fallback is gone. A selected pacing interval of less than one
millisecond yields rather than unconditionally sleeping one millisecond; a
larger positive lead may still use bounded one-millisecond host-control slices.
Pause and debugger-control waits remain independent host-control behavior.

## Verification

- `cmake --build --preset current-gates-gcc` passed every current smoke and
  specialized gate, including documentation governance for `vm-0-5-0472`.
- `vm-model-339-clock-contract-smoke` proved Standard macro pacing and Turbo
  no-wait deadline advance on the same selected axis.
- Release configuration and `current-gcc` built target/banner
  `vm-0-5-0472` / `0.5.0472`; CMake's optimized/stripped artifact verifier
  passed before link.
- Artifact: `build/output/nxvm_0_5_0472.exe`, SHA-256
  `C2B9B95B8A124B96106369E4D88D592A6A7A090D16401C176E93B91349E28BDE`.

## Retained Boundary

This is L2 macro pacing, not a claim that the complete CPU retirement,
transaction, controller deadline and oscillator chain is physical time. The
existing profile physical-timebase TODO remains the only receiver for that
separate proof.
