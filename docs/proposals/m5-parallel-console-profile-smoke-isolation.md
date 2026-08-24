# M5 Parallel Console Profile-Smoke Isolation

## Purpose

Make current-gate Console profile smokes deterministic under parallel CTest by
giving each test an owned catalog/profile workspace.

## Boundary

The task changes only test/CMake fixture isolation.  It preserves the product
catalog discovery route, Console selection semantics, standard-input
restoration and existing production profile paths.  It does not add a second
catalog implementation or hide either smoke from the current gate.

## Approach And Acceptance

Inventory all fixed profile-file writes and catalog discovery callers in the
affected tests. Route the lifecycle, Model-40, and memory-roundtrip smokes to
unique owned working/profile directories through existing test setup, prove
each retains its expected selection, and run repeated parallel current-gate
replays. A complete result includes a control serial run and no shared
source-working-directory profile residue.

## Stop Condition

Stop for a needed product catalog-path contract change; do not redefine menu
ordering or selection to mask cross-test interference.
