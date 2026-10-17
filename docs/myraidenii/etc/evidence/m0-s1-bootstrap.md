# M0 Td S1: Independent Product Bootstrap

## Baseline And Scope

Owner direction on 2026-10-17 selects former NXVM queue entry 10 and renames
MyArcade to MyRaidenII, with independent M0 planning/M1 implementation.
The initial clean HEAD was `da9e84b88`. Read the selected guides/current packet,
shared execution/architecture/coding/document rules and source policy; apply
architecture-governance followed by coding-governance.

Three separately committed targets are necessary: Shared registers the new
product roots/guide/checker; NXVM archives/transfers its candidate; MyRaidenII
owns its design, source policy, current packet, independent queue and three
descriptive source/test/assets root README files. No numeric T is allocated.

## Design And Provenance Review

The design reuses Lib/Emulator host capabilities, not PC or MyNES private code.
V30, Z80, YM2151, OKI and Seibu protection/video remain explicit gaps, not
features supplied merely because another emulator runs an x86 CPU. Research
is pinned in the reference register. No third-party implementation or ROM was
imported. M1 source/set freeze precedes any CPU or board implementation.

## Verification And Delivery

All three product documentation checks passed: nxvm, mynes and myraidenii.
The relative-link sweep passed 155 links in 33 selected Markdown files,
including the original candidate's moved archive and its historical inbound
reference. The packet has all 16 required fields; Queue has seven ordered,
existing proposal links. Current, roadmap and queue agree on independent M0/M1,
no numeric implementation T and no pending MyArcade candidate in NXVM.

Self-review and coordinator actual-change review found only admitted governance,
registration and root README changes. Existing production/test/build inputs,
INI files and root README remain unchanged; all ten deployed EXE hashes match
the T563 S10 artifact record. Git whitespace checks passed. No Shared production
manifest changes are needed: none of those packages changed.

This is governance/design work, so no runtime rebuild, desktop/RDP test or game
qualification was performed or claimed. Shared P1 (`f295a4a6b`) and NXVM P2
(`5fc82badb`) register and
transfer the product; MyRaidenII P3 completes this design delivery. Each is a
separate target and is pushed immediately. M0 awaits owner design acceptance;
M1 implementation remains unadmitted.
