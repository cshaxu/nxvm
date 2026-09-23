# M6 T39 S8 IRQ Poll-State Repair

## Repair And Regression

On accepting an IRQ or NMI, Core now marks `irq_poll_i` masked with the CPU I
flag.  A persistent level IRQ can no longer re-enter before the handler fetches
its first instruction.  `mynes.core.interrupt-smoke` now holds a level IRQ
asserted after vector entry and verifies that a second service attempt is
rejected until handler code changes the CPU state.

## Verification

| Check | Result |
| --- | --- |
| x64 focused interrupt regression | Passed. |
| x86 focused interrupt regression | Passed from the fresh ignored x86 configuration. |
| Ignored CPU/IRQ probe | Stack no longer repeats one return address; PC returns to ROM execution and Cartridge IRQ is acknowledged. |
| Ignored synchronized Start/no-input frame probe | Published frame hashes differ and 28 RGB pixels differ; palette values are no longer the all-black baseline. |

This proves the original IRQ re-entry/black-screen receiver is repaired and
Start now causes an observable visual state transition.  It does not by itself
prove an entire game session or every visual scene.  A later acceptance pass
must demonstrate sustained player-controlled motion and nonblack frame change
using the same ignored automation boundary.
