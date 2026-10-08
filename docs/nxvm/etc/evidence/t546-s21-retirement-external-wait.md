# T546 S21 Retirement And External Wait Accounting

## Scope

S21 closes the remaining Core-owned publication gap between an external
memory cycle and fault delivery.  It changes neither board timing, host
pacing nor a public interface.  The existing Core retirement-wait owner now
also records whether a pending qualified wait ends in an instruction
retirement or in a completed non-retiring delivery boundary.

## Defect And Single-Owner Repair

`core_machine_cpu_execution_refresh()` can commit an external read before
the instruction later faults.  Previously, the fault-delivery path paused
immediately and discarded that already-qualified external-cycle duration.
The successful path used the retirement wait owner, so the two outcomes did
not partition the same committed work consistently.

The private `cpu_retirement_wait_retires` bit distinguishes the two existing
uses of the one wait state:

- a successful instruction consumes the wait and publishes one retirement;
- fault delivery consumes the same qualified wait but publishes no
  retirement, then returns at the delivery boundary before executing a
  handler in that run call.

Reset and processor reset clear the complete private wait state.  A halted
CPU remains interrupt-waiting only for a retiring wait; a completed
non-retiring wait cannot become a synthetic instruction or compatibility
progress path.

## Direct Proof And Similar-Path Sweep

The extended Core prefetch-locality smoke uses 80386 address-size `MOVSB`.
Its source read at physical `0100h` commits one external wait; its ES:EDI
destination then exceeds the real-mode segment limit and faults.  With a
one-tick execution budget it proves all of the following:

- the instruction does not retire;
- exactly one elapsed/external tick is published;
- the wait is fully consumed and marked non-retiring;
- exception-handler execution does not leak into the same call.

The existing successful delayed-retirement contract remains in the same
smoke.  A source sweep covered every Core writer/consumer of
`cpu_retirement_wait_*`, `external_cycle_round_ticks`, reset, shutdown and
delivery transitions.  They now either start/consume this owner or clear its
entire private state; no second time publisher or delivery-local clock was
introduced.

## Verification

- Focused `core-machine-prefetch-locality` smoke: pass on x64 and x86.
- Complete repository-only `unit` suite: **506/506** on x64 (34.92 s) and
  **506/506** on x86 (36.51 s).
- `src/x86` and `test/x86` manifests: pass.
- Eight 0.5.0546 PC Release artifacts: rebuilt from this revision.

No integration suite was rerun: the change is Core-local and has direct
owner coverage plus the complete repository-only dual-width unit suite.  It
does not claim new firmware/media integration evidence.

## Receiving Artifacts

| App | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| My5160 | `79CEF859C627745CBBAE769AB0DED9AAEB2CA036ABE124B3017E2E59C846E2FE` | `69946021AC149E6BDC784145C6DD3982BA92F7270D5A36020BA92974F12A629F` |
| My5170 | `B1D0D10EB01849ED5F3887416D37E3928691C4A1AA57DF4FC64D0754035230E8` | `2F7AED7C724F065AAF729AB8B02FF7A0443C5AF221E3C0E40786044D479B4298` |
| MyDeskPro386 | `DD00BF1A2EE1E46CC89E3891FA4D63E89DBBFC1D2D3744D9D8BDC25D732F970B` | `5948DFB319ED3087D7C0A71F93572E301EF908D47DB74163658F37AF72C3EF8C` |
| NXVM | `64DA434B7AF7F16ABFABCC60D02838F8141BFFDDF41E8042737DCD80AB47685E` | `04E9591D8EDF0BAF4497FF18822A6872E23C4F0B469565B3BCCD6318A3E64427` |

