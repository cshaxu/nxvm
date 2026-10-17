# MyRaidenII Documentation Guide

MyRaidenII is an independent product in the nxvm repository. It succeeds the
former NXVM queue candidate MyArcade, with RAIDEN II as its sole initial game.
It owns its MTSP sequence, design, Current, Queue, TODO and evidence. Shared
rules remain under [../rules/](../rules/); there is no product-private rules copy.

## Task Reading Set

Before work read this guide, [Current](states/CURRENT.md), its active packet,
[Execution Rules](../rules/EXECUTION.md) and [Contributing](../../CONTRIBUTING.md).
Admission also reads [Queue](states/QUEUE.md) and [Roadmap](design/ROADMAP.md).
Code/build work reads [Architecture](design/ARCHITECTURE.md), [Source Layout](design/CODING.md),
[Architecture Rules](../rules/ARCHITECTURE.md) and [Coding Rules](../rules/CODING.md).
Documentation/closure reads [Documentation Rules](../rules/DOCUMENT.md) and
the authorities being changed. UX work reads [Product UX](design/UI.md).
Research/import/asset work first reads the [source policy](etc/operations/policy/source-policy.md).

## Authorities

- [Goal](design/GOAL.md): product outcome and exclusions.
- [Architecture](design/ARCHITECTURE.md): ownership, dependencies and runtime.
- [Source Layout](design/CODING.md): directories, names and test ownership.
- [Product UX](design/UI.md): monitor/gameplay and failure experience.
- [Roadmap](design/ROADMAP.md): M0 planning and M1 implementation boundaries.
- [Current](states/CURRENT.md): sole active packet/status authority.
- [Queue](states/QUEUE.md): ordered unnumbered candidates; numeric T identifiers
  start at T1 only on implementation admission and remain product-local.
- [TODO](states/TODO.md): unplanned debt, not a second task queue.
- [Supporting index](etc/README.md): sources, detailed design and evidence.

Use commit target `MyRaidenII` for this product and `Shared` for shared mechanisms.
An explicitly admitted cross-target S uses `NXVM` for affected PC artifacts
or the historical queue transfer, not for MyRaidenII source. Receiving artifact
verification follows the shared Execution Rules; this does not authorize
undeclared NXVM source or configuration changes. One commit has one target. M0
governance uses `M0 Td S1 Pn`; M1 implementation uses `M1 Tn Sn Pn`. Do not
inherit NXVM's T564 or MyNES's task/version sequence.
