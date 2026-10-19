# T3 S4: Bounded COP Observation And Receiving Ledger

## Scope And Input Discipline

This is an evidence-only receiver for B07/B08.  It does not add a COP model,
production route, Shared interface, ROM byte, or protected trace payload.
The owner-supplied archive remains outside the repository:

| Input | SHA-256 | Use |
| --- | --- | --- |
| `O:\assets\raidenii\raiden2.zip` | `9FEDC8DDAC2208408C3B05201316E3F2DB43BDFB104551BA17337BB55DB13D71` | Read-only source of the two already-inventoried 512 KiB V30 program lanes. |

The disposable probe copied only the two program lanes into
`build/myraidenii-t3-s4-probe/rom/`, configured the real local Board, and ran
the existing V30/Core execution path.  It emitted only transaction category,
physical address and aggregate count; it did not emit ROM bytes, instruction
bytes, values, a raw trace, screenshots or game output.

## Reproducible Observation

The probe links its private Board build with the existing
`x86-core-machine-observable` target so the already-public copied transaction
trace can be observed.  No source contract was added.  A 5,000,000-instruction
budget ended normally at `9800:0247c`; the stop reason was the requested
budget, not startup completion.  It observed 11,708,516 CPU memory reads and
79,401 writes.  The retained 0x400--0x6ff address summary contains 84 distinct
written addresses and no reads; it fits the S4 64 KiB summary budget.

The current Board has no COP route, so every observed B07/B08 address below
currently falls through to ordinary Core RAM.  This is a reproduced missing
device boundary, not evidence that RAM is correct device behavior.

| Observed address family | Neutral disposition | Evidence grade / next owner |
| --- | --- | --- |
| `0x428`, `0x42a`, `0x42c`, `0x432--0x43c`, `0x444` | COP configuration/program writes reached before the bounded run stopped. | Real-set address observation; register purpose only L2 from pinned MAME reference. S5 owns explicit register decoding; S6 owns each command result. |
| `0x476--0x47e` | COP DMA configuration writes reached. No `0x6fc` or `0x6fe` trigger was observed. | Real-set address observation; DMA semantics/throughput unresolved. S6 owns an explicit no-fallback trigger model and public Core DMA use. |
| `0x4a0--0x4a6`, `0x4c0--0x4c6`, `0x500` | COP register-file and command-write families reached. | Real-set address observation; command value/result meaning unresolved. S5 owns latching/reset and S6 owns finite decoded operations. |
| `0x600--0x68c`, `0x6c0--0x6ce` | Low-board writes outside the COP register families also occurred. | These remain Video/board receiving work; S4 must not claim or absorb their semantics. `0x6cb` is the already-owned program-bank byte. |
| COP result families (`0x580--0x5b5`) | Not reached as reads within the bounded run. | No fabricated read results. Their disposition remains an explicit later B07 receiver. |

## Source Grade And Boundary Decision

The fetched temporary MAME revision is the already-pinned
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff` driver reference:

| Temporary research input | SHA-256 | Retained L2 fact only |
| --- | --- | --- |
| `src/mame/seibu/raiden2.cpp` | `FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A` | Address-family navigation for the observed COP/DMA region. |
| `src/mame/seibu/seibucop.cpp` | `7E9763D7D1026E82473F135AEE26317DFD8E930C5F1AC257780CD813484A75A9` | The reference itself labels portions incomplete/guesswork; it cannot establish hardware-exact command behavior. |
| `src/mame/seibu/seibucop.h` | `69857872F1E68D807442FA6018562A29390ACE4E0D66ECF5D86ABE080829E962` | Header identity only; no declarations or tables are imported. |

S5 therefore has one deliberately small local boundary: a MyRaidenII Core COP
owner reachable through explicit Board replacement routes for the observed COP
families.  It may preserve resettable register values and reject unimplemented
command/result paths, but it must not let those addresses silently become
ordinary RAM.  It must not decode values, synthesize result reads, copy MAME
logic, start a second clock, or implement a DMA transfer until S6 has a finite
operation ledger and separately proved semantics.

## Outcome And Cleanup

The run reached a meaningful pre-COP checkpoint: it configured multiple COP
and DMA register families through the actual V30/Core bus.  It did **not**
reach a complete startup checkpoint, observe a COP result read, or establish
any command result.  That is evidence for the receiving split, not a passing
game boot result.

The temporary program extraction, probe source/output and MAME research files
are disposable.  They are deleted after this record using the repository
cleanup tool; no protected payload, source text or raw trace is retained.  The
reviewed cleanup removed `myraidenii-t3-s4-probe` (130 files, 17,981,969 bytes)
and `myraidenii-t3-s4-research` (3 files, 245,735 bytes).
