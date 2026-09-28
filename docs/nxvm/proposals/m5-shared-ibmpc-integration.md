# Shared IBM PC Board Integration

## Goal And Dependency

Second ordered migration candidate, unnumbered and not admitted. After
[independent chips](m5-shared-chip-extraction.md), extract the actual common
board mechanisms of XT, AT, DeskPro 386 and default PC/AT into `src/x86/ibmpc`.
This is reusable PC assembly, not another chip library or a host executor.

## Ownership

- `x86/devices` retains sole ownership of each chip's state and behavior.
- `x86/ibmpc` owns the demonstrably shared PC memory/port routing, signal
  wiring, construction/reset sequencing and guest-device scheduling mechanism.
  It integrates chips only through their public contracts.
- Common remains owner of host execution/lifecycle coordination; Lib remains
  owner of platform, file and audio services. There is no second lifecycle
  FIFO, host worker, scheduler time source, renderer or storage backend here.
- Each product composition selects its actual chips, wiring, clock inputs,
  address maps, constraints and immutable firmware roles. Board-unique Compaq
  or XT/AT details stay at their genuine owner; do not force four boards into
  one configuration full of model-name switches.
- Use small typed descriptions or direct assembly where semantics genuinely
  match. Accept resolved external assets through neutral inputs; no product
  INI parser, asset-root lookup or protected ROM bytes enter shared board code.
- Own board-local mutable glue once and define its reset/event lifetime;
  never mirror device registers to make routing convenient. Chip cycles and
  deadlines feed one guest timeline; host elapsed time cannot create ticks.

## Coverage And Work Strategy

The first S inventories the four construction graphs and the first candidate's
retained non-chip files. Its durable ledger distinguishes common mechanisms,
genuine machine differences and App/host adaptation. Include bus transactions,
interrupt acknowledgement, DMA/refresh, firmware mapping, reset, time and
failure cleanup; similarity of filenames is not proof of shared semantics.

Extract and reconnect one complete dependency batch at a time. Finish with
NXVM using the shared implementation and delete its duplicate common board
paths. Preserve one construction/rollback owner and one reset path. No generic
board inheritance, universal event bus or new per-machine execution loops.

## Verification And Exit

- Shared board code depends on chip public contracts and declared neutral
  capabilities, never an App path; independently built x86 tests prove this.
- Common board contract tests live in `test/x86/ibmpc`; specific machine
  composition/firmware tests remain product-local. Units use synthetic owned
  inputs, not external files; integration retains all four real machine sets.
- Every ledger member has a verified shared implementation or a justified
  retained machine-specific owner; no duplicate common implementation remains.
- Required full unit/integration, manifests, static/governance checks and
  affected dual-width artifacts pass. Every existing boot scenario and timing
  classification is preserved; structural extraction makes no new L3 claim.
- The four App split receives a concrete source/test/build owner map, not an
  unresolved catch-all legacy directory.

## Non-goals And Stops

No App split, new hardware, PC110 implementation or asset relocation. Scope is
Shared plus NXVM, with other receivers explicitly admitted if affected; one
target per commit. Stop for a contract/behavior change beyond extraction,
unresolved shared ownership, or a requirement to import protected material.

Next: [four independent PC Apps](m5-independent-pc-apps.md).
