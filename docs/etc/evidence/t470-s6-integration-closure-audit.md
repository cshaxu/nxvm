# T470 S6: Integration Closure Audit

`M5:T470:S6:INTEGRATION-CLOSURE:OK`

## Reconciliation

| Scope | Closed result |
| --- | --- |
| S1--S4 Core ownership | One Core-owned elapsed axis publishes only qualified retirement or source-qualified deadline progress. VM reads a copied observation. |
| S5 profiles | default PC/AT, Model-339 and Model-40 are all `UNQUALIFIED` at a zero physical rate; no product construction selects verified physical time. |
| Standard/Turbo | On an unavailable HLT axis, Standard alone takes the retained 1 ms host-load backoff; Turbo omits only that wait. Neither branch generates guest ticks. |
| Gate alignment | The T388 inventory now checks the S2 replacement `core_machine_capture_time_observation`, not the deleted pacing-contract accessor. No compatibility accessor was restored. |
| Physical pacing residual | [Profile physical-timebase closure](../../states/TODO.md) is the sole receiver. It requires a complete primary-backed CPU, transaction, controller and oscillator chain before Standard may wait against host budget. |

## Release Evidence

- Source commit: `f5f68aed`.
- Target and banner: `vm-0-5-0470` / `0.5.0470`.
- Stripped Release artifact: `build/output/nxvm_0_5_0470.exe`.
- SHA-256: `8DAC84FE2F05CAF7718E2B8E3F6DAC6248CF758FCA4F9AB956204FE16656E56C`.
- Release build verified the optimized-Release pre-link requirement.

## Verification

- Focused `current.vm-session-speed-policy-smoke` and
  `current.vm-runner-display-cadence-smoke` pass.
- The final current-gate run covers the corrected T388 inventory, registered
  current smoke/specialized gates and documentation governance.
- The direct registered CTest run passes all 295 current tests.

This closure makes no exact wall-clock pacing claim for a current product
profile. It proves the required direction of control: Core advances guest
time; the host may only wait after a future profile supplies a complete,
verified Core axis.
