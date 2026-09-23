# M5 T30 S3 Cartridge Coverage Verification

## Purpose

This record verifies the descriptor and media implementation committed in
`4f287be`. It does not claim Mapper 2, Mapper 3, or extended Mapper 4 runtime
behavior: their declared profiles are deliberately admitted by format parsing
and rejected by construction until their respective M5 tasks deliver hardware.

## Added Coverage

`test/core/cartridge_contract_smoke.c` now proves the finite edges consumed by
the T30 ledger:

| Receiver | Evidence |
| --- | --- |
| CNROM | Each admitted 1--4 CHR-bank header is accepted by the descriptor path and receives the explicit `LIB_STATUS_UNSUPPORTED` execution disposition. |
| MMC3 | The 32 PRG-bank / 32 CHR-bank upper boundary is descriptor-admitted and receives the explicit unsupported disposition until T33. |
| Legacy suffix | Exact payload, 127-byte suffix and 128-byte suffix normalize to the payload; a 126-byte surplus is rejected. |
| Bounds | The published maximum includes header, profiled maximum payload and permitted legacy suffix. |
| Media transaction | An admitted but unimplemented UxROM image fails replacement without replacing the accepted machine. |

Existing checks retain mapper-0, MMC1, supplied M4-size profile, malformed
headers, unsupported header fields, payload mismatch and storage-failure
rollback coverage.

## Verification

- Focused `mynes.core.cartridge-contract-smoke` and
  `mynes.core.media-failure-contract-smoke`: passed on x64 and x86 after fresh
  target builds.
- Full x64 CTest suite: 112/112 passed. The first attempt exposed a transient
  unrelated `library.kvm_window_modal` timing failure; its three-attempt retry
  passed, followed by the full successful run.
- Full x86 CTest suite: 112/112 passed serially in 324.94 seconds. Isolation
  was necessary after overlapping exploratory CTest invocations were stopped;
  the recorded run is one serial, clean process set.
- Documentation governance state gate and `git diff --check`: passed before
  this evidence update. The closure audit reruns the final documentation gate.

## Rebuilt Delivery Artifacts

The source build refreshed the tracked executable pair. Their SHA-256 values
at this verification point are:

| Artifact | SHA-256 |
| --- | --- |
| `assets/binary/mynes_0_1_0011_x64.exe` | `93270DE0F9E5A0DF2EC47736D551ACD69ED061F611147C85B68D26479B951646` |
| `assets/binary/mynes_0_1_0011_x86.exe` | `8A91C544E657E47D1AE971F69F364C3EA80705B8FAE8DD56E9753085B856E3CB` |

Owner ROMs remain ignored and were neither read as a test fixture nor added to
the repository.
