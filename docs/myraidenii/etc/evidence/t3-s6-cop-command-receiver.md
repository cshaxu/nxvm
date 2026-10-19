# M1 T3 S6: COP Command, Result And DMA Receiver

## Scope And Probe

S6 is evidence-only. It reuses the S4-selected external archive and verifies
its SHA-256 before use. A disposable archive of accepted pre-S5 commit
`77110cf04` restores the exact ordinary-RAM fallback baseline, allowing the
real V30 path to continue past the S5 deliberate command rejection without
changing tracked production code. The temporary probe uses the existing
observable X86 Core transaction interface and records only access-family
counts and first event sequences. It never writes ROM bytes, instruction
bytes, command values, register values, raw traces or screenshots.

The two external program lanes match the committed inventory hashes. The
probe has two fixed budgets; each ends through the requested instruction
budget, not a claimed startup checkpoint.

| Budget | Executed | `0x500..0x505` writes | `0x580..0x5b5` reads | `0x6fc/0x6fe` writes | First command sequence |
| --- | ---: | ---: | ---: | ---: | ---: |
| 5,000,000 | 5,000,000 | 654 | 0 | 0 | 1,125,956 |
| 20,000,000 | 20,000,000 | 654 | 0 | 0 | 1,125,956 |

The repeated command count and unchanged first sequence show that merely
allowing the old RAM fallback does not produce a new observed result or DMA
family during the additional 15 million instructions. They do **not** identify
a command word, command result, operation algorithm, hardware timing or a
cause for the later stalled execution.

## Finite B07 Disposition

| Family | S6 disposition | Receiving owner |
| --- | --- | --- |
| `0x476..0x47e` COP/DMA configuration | Directly observed by S4; no trigger follows in either S6 budget. | S7 evidence/model ledger. |
| `0x4a0..0x4a6`, `0x4c0..0x4c6` register files | Directly observed by S4; their semantics remain unknown. | S7 evidence/model ledger. |
| `0x500..0x505` command | Directly observed writes; only family/count/order fact accepted. | S7 evidence/model ledger. |
| `0x580..0x5b5` results | Not reached as reads in either fixed budget. | S7 evidence/model ledger. |
| `0x6fc`, `0x6fe` DMA triggers | Not reached as writes in either fixed budget. | S7 evidence/model ledger. |

S7 must not treat the zero observations as a command result, no-op command or
DMA specification. It first needs a source-grade operation universe and an
independently reproducible model/receiving proof. If that evidence is absent,
the correct outcome is an explicit deferred ledger entry, not a scripted return
or a larger unbounded runtime budget.

## Cleanup

The governed cleanup tool first reviewed every media/configuration artifact in
the temporary archive and then deleted `build/myraidenii-t3-s6-probe`: 4,146
files and 58,710,039 bytes. This included the two extracted lane copies, the
baseline archive and all generated build output. No temporary payload remains.
