# Initial Convergence Ledger

All implementation entries below are **planned, unimplemented and unqualified**.
M0 creates ownership and receiving criteria, not gameplay evidence. M1's first
admission must turn each row into finite vectors/checkpoints with fixed inputs,
observations, sources, budgets and dual-width test owners.

| Capability | Owner / reuse decision | Receiving proof |
| --- | --- | --- |
| Board/set and lawful ROM roles | Product inventory + external owner archive | Fixed revision, sizes/hashes, completeness and wrong-set rejection |
| V30 execution | Reviewed CPU owner; existing Intel core is not V30 | NEC instruction/flag/interrupt vectors and actual board paths |
| Memory/banking/interrupts | Core board | Real bus edges, vectors and deterministic boot checkpoints |
| COP/protection and DMA | Core board | Evidenced operations and real game use, no bypass |
| Graphics and encryption | Core video | Pixel/layer/sprite/priority vectors and named game scenes |
| Raster and orientation | Core timing; Machine output conversion | Native timing grade, complete frame and Window orientation |
| Z80 and sound communication | Core sound | CPU, bank, latch, timer and IRQ vectors |
| YM2151 and OKI sound | Core sound; Lib PCM delivery only | Deterministic mixed PCM and actual game command paths |
| Controls | Product mapping; Core input registers | Real host event through Machine to coin/start/game controls |
| Lifecycle/configuration | Existing Emulator + product adapter | All transitions, adjacent INI paths and safe input reset |
| Failure and ownership | Existing contracts, no recovery framework | Bad assets, partial output, startup rollback and teardown |
| Gameplay | Selected real game | Attract, stages/bosses/end, game-over and restart ledger |
| Dual release | Product build/deployment | x64/x86 units/integrations, latest pair PE/version/hash identity |

Debug and snapshots use existing unsupported behavior until their entire
required machine-state/safe-access contracts are implemented. They must not
silently claim success, nor become hidden mandatory dependencies for boot.
