# Shared IBM PC Board Integration

## Goal And Dependency

Admitted as T540 after closed [independent chips](m5-shared-chip-extraction.md).
Extract the neutral x86 executor and the actual common
board mechanisms of XT, AT, DeskPro 386 and default PC/AT into the flat
`src/x86/ibmpc-common`, `src/x86/ibmpc-at` and `src/x86/ibmpc-xt` components.
This is reusable PC assembly, not another chip library or a host executor.

## Ownership

- `x86/chips` is the sole ownership location for each extracted chip's state
  and behavior; S4 removed the retired `x86/devices` source path.
- `x86/ibmpc-common` owns demonstrably shared IBM-PC port routing and signal wiring.
  It integrates chips only through their public contracts. The generic x86
  machine executor -- memory, ports, transactions, guest timeline, CPU bus and
  plan application -- is not IBM-PC code; its required neutral receiver is
  `x86/core`, established before independent Apps stop depending on the current
  App-owned Core interface.
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

S1 inventoried the four construction graphs and retained non-chip files. Its
durable ledger distinguishes common mechanisms,
genuine machine differences and App/host adaptation. Include bus transactions,
interrupt acknowledgement, DMA/refresh, firmware mapping, reset, time and
failure cleanup; similarity of filenames is not proof of shared semantics.

S2-S4 fixed the finite adapter ledger, target layout and chip path. S5 resolves
the generic-Core/IBM-PC cut in its [source-inspected map](../etc/architecture/t540-s5-neutral-core-cut.md)
before S6-S8 move Core ownership. Then extract and
reconnect one complete board dependency batch at a time. Finish with
NXVM using the shared implementation and delete its duplicate common board
paths. Preserve one construction/rollback owner and one reset path. No generic
board inheritance, universal event bus or new per-machine execution loops.

## Verification And Exit

- Shared board code depends on chip public contracts and declared neutral
  capabilities, never an App path; independently built x86 tests prove this.
- Common board contract tests live with their flat receiver under
  `test/x86/ibmpc-common`, `test/x86/ibmpc-at` or `test/x86/ibmpc-xt`; specific machine
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
