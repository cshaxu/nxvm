# Interactive Dual KVM Vertical

## Outcome

Produce one original NROM fixture whose background marker responds to the default controller through both real Window pixels and Core-produced Console text. The same executable loads, runs, pauses, returns to the monitor and switches representation without guest reload.

## Prerequisites And Authority

M3 design acceptance; read m3-design.md and the complete remediation ledger.
See [M3 design](../etc/m3-design.md), [remediation](../etc/m3-remediation.md),
[Architecture](../design/ARCHITECTURE.md), [Coding](../design/CODING.md) and
[research provenance](../etc/m3-sources.md). This was an unnumbered candidate;
implementation and S allocation required separate admission.

## Complete Scope

Repair App/driver and debug metadata boundaries, transaction/lifetime ownership, grammar essentials and build isolation first in this same outcome. Implement the single slot clock, cartridge CHR/mirroring, PPU register/background path sufficient for the fixture, controller serial path, both converters, `mynes.ini` startup configuration and F12/F5 routing. No second CPU engine or placeholder console route.

Remediation batch: R01-R05, R08-R10 close here except R05's complete command matrix and R08's device timing; establish R06/R07 and apply R11/R12.

## Verification And Convergence

G01; initial G02-G04 and G07-G13. Independently calculated background pixels/text cells and scripted guest controller checkpoints must pass through production APIs on x64/x86. Observe both native backends and monitor return. Inject every candidate acquisition and teardown failure. Run complete registered regression and unchanged shared gates.

The design G ledger and remediation R ledger are the task convergence inventory. At admission expand this package's named batch into finite cases, required contexts, evidence and dispositions; no first-failure chasing or test-count-only closure. Every accepted member needs production-path evidence and a regression owner. Retain all earlier accepted behavior; expose unsupported behavior explicitly.

## Boundaries And Transfer

Sprite and exact PPU/NMI/DMA timing go to PPU completion; exhaustive input/state cases to Input; conversion/native qualification to Presentation. Record initial-versus-final G dispositions explicitly; this outcome does not claim full M3. Lib/Common source and tests remain unchanged. No commercial ROM is committed. Any primary-source contradiction, unsafe lifetime or required shared mutation stops its affected work until resolved in the admitted scope. All applicable owned members must close before acceptance; residuals need the named receiving candidate, and no unresolved required row may disappear at milestone closure.
