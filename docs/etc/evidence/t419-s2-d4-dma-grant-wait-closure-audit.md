# T419 S2 D4 DMA Grant Wait Closure Audit

M5:T419:S2:D4-DMA-GRANT-WAIT-CLOSURE:OK

Coordinator actual-diff review covered P1 through P6 (7aa61a6c..7bc46807).
Core owns the configurable DMA grant delay and retains the sole CPU/DMA
transaction path; Model-40 composition alone selects one scheduling quantum.
The direct Core proof confirms no transfer on the first eligible quantum,
exactly one on the second, and reset clearing. The Model-40 topology smoke
emits `M5:T419:S2:D4-DMA-GRANT-WAIT:OK`.

During the required current-gate review, two shared Core correctness defects
were repaired: provider-bounded ROM prefetch routing and CS-limit-respecting
prefetch. Immutable-ROM, entry-plan, and interrupt-entry regressions now pass.
The full current smoke gate passed, documentation governance passed, and the
verified T419 artifact is `nxvm_0_5_0419.exe` SHA-256
`84662DCABC6BDA05992D09C9E394F3BA95ED10809724AB37900D1F5365228A17`.

This closes only the original-source-backed discrete one-quantum bridge. It
does not claim calibrated DCLK duration, HAK/HAKDMA waveform, BUSRDY, refresh
priority, BWAIT, cascade timing, or Model-L3 readiness; those remain receivers
of the DeskPro physical-cycle proposal.