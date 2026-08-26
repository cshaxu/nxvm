# T470 S4: Controller And Transaction Settlement

`M5:T470:S4:CONTROLLER-TRANSACTION-SETTLEMENT:OK`

## Result

The existing sole elapsed-time publisher now receives an internal publication
origin instead of an ambiguous `cpu_retired` boolean. It admits physical
progress only from a qualified CPU retirement or a Core-composed deadline.
The origin remains private to the Core machine owner; it creates no public
controller setter, counter, callback or scheduler.

## Complete Non-Retirement Publisher Matrix

| Origin | Owner/lifecycle | Source and conversion disposition | Physical result and receiver |
| --- | --- | --- | --- |
| `EXTERNAL_WAIT` | Deferred CPU external-cycle and BUSRDY branches in `core_machine_run()` | Current transaction contract supplies deterministic board policy only; T470 has no selected physical bus/wait-state chain. | Rejected in physical mode. S5 retains the profile blocker; a future sourced board transaction admission owns any change. |
| `D4_SLOWDOWN` | Model-40 auxiliary-PIT output gate in `core_machine_run()` | Current D4 input/order behavior is retained, but its one-tick slowdown duration has no closed physical conversion. | Rejected in physical mode. S5 retains the Model-40 blocker. |
| `DEADLINE` | `core_machine_advance_to_next_deadline()` after copied observation | T469 derives only PIT next-output and L3-source RTC IRQ boundaries through copied source-rational clock plans. The Core axis supplies the unit only when already marked verified physical. DMA, KBC, FDC and HDC activity invalidate the observation. | Admitted in physical mode only through the existing valid-deadline path; no controller state is exposed. |
| `DETERMINISTIC_ADVANCE` | Explicit `core_machine_advance_time()` test/helper operation | Explicitly deterministic/test-only; it is already invalid under physical retirement. | Rejected in physical mode; no receiver. |

No other production path writes `machine->elapsed_ticks`: reset initializes it,
and the sole publisher advances it. The timing timeline, clock domains,
controller callbacks, VM waiting and host display time are consumers only.

## Retained Boundary

The Model-339 plan's PIT/RTC rules remain source-qualified controller facts,
but its CPU and board chains are not a complete physical profile proof. The
new synthetic Core-only regression therefore does not select a Model-339 rate
or alter its published unavailable observation. It proves only the owner-local
property required by a later complete profile: a valid PIT deadline can pass
the sole publisher when the same Core instance already carries a verified axis.

All current product profiles remain unavailable. The source-qualified deadline
path is not a shortcut around T388 residual CPU forms, transaction waits, D4
slowdown, active DMA/KBC/FDC/HDC blockers or S5 oscillator proof.

## Verification

- Rebuilt the focused Core time, external-wait, instruction-timing,
  instruction-timing-ledger, retirement-observation and Model-339 deadline
  smoke targets under `current-gates-gcc`.
- Five registered current-gate CTest cases pass: Core time, prefetch locality,
  retirement observation, instruction timing and instruction-timing ledger.
- The direct Model-339 contract smoke passes, including its T469 deadline
  marker and the synthetic physical-axis deadline assertion.
- Static sweep covered every `core_machine_publish_elapsed_ticks` caller,
  all `elapsed_ticks` writes, Core deadline producers and VM waiting routes.

The tracked source/test diff is 51 added and 14 removed lines by
`git diff --numstat` excluding documentation: 35 added/14 removed production
lines and 16 direct regression lines. The net growth replaces the ambiguous
boolean at every existing publisher call with the one owner-local origin
classification; it does not add another publication path or state holder.

## Transfer

S5 must prove each profile's oscillator/divider/wait-state chain and the
complete CPU/controller matrix before selecting a nonzero physical time axis.
It may not promote the S4 synthetic proof, a controller clock ratio or a
nominal CPU frequency into that proof.
