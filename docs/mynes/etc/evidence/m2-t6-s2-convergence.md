# M2 T6 S2 Convergence Evidence

This ledger records the bounded verification sequence for the active
coverage-bearing CPU/bus task.  It supplements, and does not replace, the T6
proposal, its history record, the opcode CSV, or Current's active-packet state.

| Batch | Concrete outcome | Status | Receiving proof |
| --- | --- | --- | --- |
| P1 | Classify every byte at the public execution boundary. | Complete. | `test/core/decode_ledger_smoke.c` executes 0x00 through 0xFF in original in-memory mapper-0 images; legal bytes retire once, excluded bytes trap after one cycle with zero retirement and exact trap PC/opcode. |
| P2 | Prove every official operation's observable semantics and total. | Complete. | The 151-row ledger asserts cycles, PC and reset-state P for every form; focused family fixtures provide differentiated data/status cases. |
| P3 | Prove binary ADC/SBC over the full input space. | Complete. | `test/core/alu_exhaustive_smoke.c` executes 524,288 public machine cases: ADC and SBC over all A/operand/carry combinations, repeated with D clear and set, and verifies A plus C/Z/N/V/D/I/U. |
| P4 | Prove functional control, interrupt and bus behavior. | Complete. | Public fixtures cover IRQ/NMI, reset, branches, stack/control and the M2 RAM/open-bus/reserved-device map, including error-path trap metadata. |
| P5 | Decide T6 closure from complete evidence. | Complete. | Full CTest suites pass for both x64 and verified i686 GCC configurations. The current product artifacts are x64 `36BC7E82BD8439C77F974F2FC29E4E819F284F87DC06E9D8207722CF8E6767D9` and x86 `C00E12661C4C45FB9B14C3A2DD92AA4942B5EC65F91A467D0F073A6E55B97059`. |

The batches are ordered by dependency.  P2--P4 may add or repair the one CPU
engine, but may not introduce a second interpreter, a private state mutation
API, or test-only execution behavior.  A passing batch does not waive a later
row or transfer unfinished T6 functional CPU/bus work to debugger, timing or
reliability packages.

P1 was run in the x64 Release tree: `mynes.core.decode-ledger-smoke` passed.
The complete admitted Core selection (seven tests, including its six retained
regressions) also passed.  P2's new full-ledger cycle/length fixture passed in
the same tree.  P3 passed in 0.89 seconds in that x64 Release tree.  Full
documentation governance cannot yet be claimed:
the pre-existing Current baseline still declares `product-design` despite the
already-admitted product source, and its existing text also fails the
mojibake detector.  This evidence does not repair or waive that separate
governance-state defect.
