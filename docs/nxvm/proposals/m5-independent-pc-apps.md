# Four Independent PC Apps

## Goal And Dependencies

Third ordered migration candidate, unnumbered and not admitted. After
[shared chips](m5-shared-chip-extraction.md) and
[shared PC integration](../history/M5-T540-shared-ibmpc-integration-proposal.md), replace the four-machine
NXVM product shell with four top-level products:

- `src/app-mypcxt`: existing IBM 5160 XT.
- `src/app-mypcat`: existing IBM 5170 AT.
- `src/app-mypcdeskpro386`: existing DeskPro 386 Model 40.
- `src/app-nxvm`: existing default 386 PC/AT only.

Later `src/app-mypc110` is the receiver of the separate
[PC110 proposal](m6-pc110-evidence-and-implementation.md), not a stub fifth App.

## Design

Each App owns its entry/configuration/CLI composition, board-specific assembly,
asset roles and Common Machine binding. Apps consume `x86/devices` and
`x86/ibmpc`; no App imports another App's headers, source, private state or
executable. Common and Lib retain their existing neutral responsibilities.

Do not copy the former whole NXVM product four times. Reuse the existing
Common session/UI/machine contracts and the proven shared PC mechanisms.
Common PC adapter behavior belongs in the appropriate shared PC contract,
not four copied runners. Keep actual product policies local and avoid a new
universal App shell or pass-through facade solely to hide duplication.

Each EXE still has one build-fixed board. Preserve current INI option meanings,
media modes, debugger, lifecycle and firmware-driven boot; do not reintroduce
YAML, runtime machine selection or a BIOS compatibility bypass. Protected
firmware/media stay in the existing external archive; directory renaming alone
does not authorize changing owner INIs or moving external masters.

## Coverage And Cutover

The first S freezes a four-row product migration map and a complete file/test/
tool/build/artifact/document map. Each original integration scenario receives
one named destination and unchanged acceptance predicate. CPU/device and
common-board tests stay in x86, not duplicated per App.

Tests mirror `test/app-mypcxt`, `test/app-mypcat`,
`test/app-mypcdeskpro386` and `test/app-nxvm`, with separate integration trees.
Each product receives its own docs guide, design, states, proposals/history,
tools and CMake entry using shared build mechanics. Preserve provenance of
existing NXVM task history; move/link records rather than renumbering it.

Before any new-target commit, admit a separate governance Td to extend the
current rules' NXVM/MyNES/Shared target vocabulary and executable deployment
mapping for the new Apps. Freeze product names, INI compatibility, dual-width
EXE names, version continuity and `assets/<product>/` destinations there.
Do not change docs/rules from an implementation S or silently invent commit
prefixes. Current `assets/nxvm/<profile>/` remains authoritative until cutover.

## Verification And Exit

- All four products independently configure/build with only their own and
  declared shared inputs; product-private edits do not affect sibling Apps.
- No old multi-machine selector, cross-App include, duplicated chip/board
  implementation or orphaned build/test/document reference survives.
- Preserve all existing test coverage and qualify every current machine on
  both x64 and x86 with real external assets and its actual adjacent INI.
- Full required units/integration, shared manifests, dependency/static and
  all affected documentation gates pass. Deliver verified optimized stripped
  dual EXEs for each App; retire old EXEs only after verified replacements.
- All four owner maps and the docs/queue handoffs are complete. MyNES remains
  unchanged except explicitly admitted Shared receiver requirements. PC110
  remains future work, not falsely marked runnable.

## Stops

Missing scope/deployment governance, lost capability, incompatible INI/asset
behavior, unresolved duplicated ownership or new hardware requirements block
the affected cutover; obtain approval rather than weakening the baseline.
