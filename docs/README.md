# Repository Documentation Guide

This is the entry point for the multi-application repository. It owns only
repository-wide governance and orientation; product facts, design, task state,
and evidence belong to the selected product documentation tree.

## Task Reading Set

Before changing a product, read:

1. this guide;
2. the selected product guide: [NXVM](nxvm/README.md) or [MyNES](mynes/README.md);
3. that product's `states/CURRENT.md` packet and its S brief;
4. the shared [Execution Rules](rules/EXECUTION.md) and the authorities named
   by the product guide; and
5. [Contributing](../CONTRIBUTING.md).

Read the selected product's source-and-research policy before importing,
deriving, packaging, or researching source, firmware, media, ROMs, or other
third-party material. A shared change names every affected product and may not
silently make one product's active packet authorize another product behavior.

## Repository Map

- `rules/` — shared governance invariants and execution process.
- `nxvm/` — NXVM design, task queue, evidence, history, and product policy.
- `mynes/` — MyNES design, task queue, evidence, history, and product policy.
- `../src/{lib,common,x86}` and `../test/{lib,common,x86}` — neutral shared
  source/test corpora.
- `../src/app-nxvm`, `../test/app-nxvm`, `../tools/nxvm`, and
  `../assets/binary-nxvm` — NXVM product ownership.
- `../src/app-mynes`, `../test/app-mynes`, `../tools/mynes`, and
  `../assets/binary-mynes` — MyNES product ownership.

## Product Task State

Each product owns its own current packet, queue, TODO ledger, proposal series,
history, and roadmap. Shared code remains neutral, but a shared change is
admitted under one named product and identifies every affected consumer. The
root does not create a second task queue or duplicate current-state authority.

## Supporting Detail

Each product's `etc/README.md` indexes its supporting material. Supporting
records cannot supersede shared rules or product current state.
