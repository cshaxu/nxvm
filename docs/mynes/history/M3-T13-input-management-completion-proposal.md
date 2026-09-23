# Input And Management Completion

## Outcome

Deliver reliable controller play and complete cooked configuration/debug control: no stuck keys after focus, pause, reset, rebind, source retirement or backend switching.

## Prerequisites And Authority

PPU and clock completion accepted; the vertical's live controller and monitor routes remain functional. See [M3 design](../etc/m3-design.md), [remediation](../etc/m3-remediation.md), [Architecture](../design/ARCHITECTURE.md), [Coding](../design/CODING.md) and [research provenance](../etc/m3-sources.md). This was an unnumbered candidate; implementation and S allocation required separate admission.

## Complete Scope

Complete all 256 pad states, serial/high-strobe/open-bus/disconnected-port behavior, per-source held states, repeats/opposites and bounded resume drain. Finish protocol 8/9 validation, atomic accepted configuration, hotkey conflicts, pending/STARTING/ERROR command policy, startup help and monitor notification/rearm. ERROR retains help/status/quit as required by the accepted UX.

Remediation batch: Close R05/R06; preserve R01/R03/R04 boundaries and R11/R12.

## Verification And Convergence

Final G09-G11. Freeze command x lifecycle/pending state, pad bitmap x strobe/read position and source x transition matrices. Prove invalid requests preserve effective settings; first resumed instruction sees cleared stale input. Exercise production Common paused release suppression, two-source release and 256-callback overflow fault. Full x64/x86 regression plus native key/monitor checks.

The design G ledger and remediation R ledger are the task convergence inventory. At admission expand this package's named batch into finite cases, required contexts, evidence and dispositions; no first-failure chasing or test-count-only closure. Every accepted member needs production-path evidence and a regression owner. Retain all earlier accepted behavior; expose unsupported behavior explicitly.

## Boundaries And Transfer

Do not add player two, persistence, native input code in App/Core or shared changes. Transfer only final conversion and native stress/teardown qualification to Presentation. Lib/Common source and tests remain unchanged. No commercial ROM is committed. Any primary-source contradiction, unsafe lifetime or required shared mutation stops its affected work until resolved in the admitted scope. All applicable owned members must close before acceptance; residuals need the named receiving candidate, and no unresolved required row may disappear at milestone closure.
