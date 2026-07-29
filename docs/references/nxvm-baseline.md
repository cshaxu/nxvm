# NXVM Reference Baseline

## Role

NXVM is a secondary reference for C project layout, CPU/memory state design,
device separation, debugger/tracing concepts, and selected x86 behavior. It is
not authority for DOS, BIOS, POST, guest services, or quick-and-dirty devices.

## Local Identity

- Local reference: sibling `../nxvm` checkout.
- Declared license: LGPL-3.0-or-later (`LICENSE`).
- Baseline commit: record before use; do not rely on an unpinned working tree.

## Restrictions

- Never modify the sibling checkout from this project.
- Create a provenance record before copying or closely deriving code.
- Verify CPU behavior with primary x86 documentation and focused tests.
- Treat its BIOS/POST/DOS hacks as leads for experiments, not accepted behavior.
