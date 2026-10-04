# Shared PC Product

Historical T541 design, implemented through S4. The [task ledger](M5-T541-independent-pc-apps.md)
records verification; [Current](../states/CURRENT.md) owns final acceptance.

## Admission And Goal

The owner revises active M5 T541 on 2026-10-04: extract the shared Product
logic of the four existing PC builds into `src/x86/product` before splitting
their Apps. [Current](../states/CURRENT.md) owns admission and progress;
[T541 history](../history/M5-T541-independent-pc-apps.md) preserves the original
request and this scope change. The separate
[four-App split](../proposals/m5-independent-pc-apps.md) is the first queued successor.

The existing XT, AT, Model40 and default builds remain under app-nxvm and keep
their identities, fixed compositions, deployed directories and adjacent INIs.
They use one shared Product implementation, not four copies.

## Ownership And Dependencies

x86/product receives the identical PC command provider, prompt/state notices,
hotkey policy, Debug entry, INI grammar/path resolution and startup/composition.
It consumes existing Lib, Common and public x86 contracts. It neither copies
Common's lifecycle queues/reducers nor owns native presentation, guest chip
state, board wiring, firmware execution or ROM selection.

Each existing build supplies immutable identity, fixed board/CPU facts and
its real construction binding. Shared Product must not include app-nxvm private
headers, select machines by name, grow a per-command callback framework or keep
a parallel parser/runner. S1 audits command/composition/keyboard dependencies
on the existing Machine adapter and determines the smallest real public
construction/operation boundary. Board-specific code stays App-owned.

Lib and Common source/tests are excluded. Existing x86 chips, Core, boards,
Debug and xasm32 implementations are excluded. Shared implementation changes
are restricted to the new Product component. A necessary App removal/binding,
test, build registration or manifest edit must be enumerated in the bounded
S packet and explicitly authorized before editing: extraction cannot be claimed
complete while the original production path remains active.

## Bounded Implementation Sequence

- S1: inventory the complete shared Product capability and dependency class;
  freeze the receiving-owner map and required connection edits. Exit: no
  unmapped Product behavior or reverse dependency; review the implementation
  surface rather than assuming permission.
- Subsequent S: move the cohesive INI/request/startup boundary, then the
  Console/command/hotkey/Debug boundary, then composition and its necessary
  Machine connection. Admit numeric S tasks from S1 evidence; retire each
  old production path in the same accepted batch rather than keeping shims.
- Final S: audit the complete Product ledger, dependency/source gates, coverage,
  four-profile integration and dual-width deployed artifacts.

## Verification And Exit

Keep existing commands, output, debugger and lifecycle behavior; NXVM.ini
continues accepting only 0/1 booleans and resolving media paths from its own
directory. No owner INI, external asset, firmware or MyNES change is included.

Exactly one production Product implementation resides in x86/product and all
four current builds use it. Retire the old App implementation; dependencies
are one-way, with no duplicated state, parser or loop. Preserve original
coverage and all 58 profile/width integration contexts. Each code S runs the
complete required unit suite; T closure runs required units and integration.
Rebuild affected optimized stripped x64/x86 artifacts without changing their
deployment scheme; update applicable manifests and documentation with approved
connection edits.

Do not create new App targets, tools/docs trees, commit prefixes or deployment
paths in T541. Those belong to the queued split and its separate Td prerequisite.
Missing public capability or scope approval is reported before an out-of-scope
edit; Lib/Common are not expanded as a shortcut.
