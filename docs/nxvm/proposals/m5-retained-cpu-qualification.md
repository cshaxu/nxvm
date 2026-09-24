# Retained CPU Family Qualification

## Goal

Preserve every existing CPU implementation, profile and selection table while
qualifying the CPU contracts required by XT, AT, DeskPro, default and future PC110. CPU
architecture remains independent of build-fixed board profiles.

## Current Product Context

The completed fixed-product cutover has made XT, AT, DeskPro and default PC/AT four build-selected
products. Each uses external BYOB firmware, one adjacent NXVM.ini runtime-media
route and sole `assets/nxvm/<profile>/` deployment. This qualification keeps
CPU semantics in `core/devices`; it neither restores runtime CPU/profile/YAML
selection nor treats a successful product boot as CPU completeness proof.

## Scope And Batches

1. Inventory actual models, instruction forms, feature/timing tables and tests.
   Current enumeration has 8086, 8088, 80186, 80286 and 80386; 80188 and 486
   must not be advertised as implemented without a real receiver.
2. Reconcile each retained family's manual function/timing and code-gap ledgers,
   including exceptions, FLAGS, privilege, paging, reset and bus differences.
3. Admit the PC110-selected 486 variant as a bounded CPU implementation task
   with its full instruction/state ledger, not an alias to 386. Preserve older
   family regressions. Treat an independent 80188 implementation as a separately
   evidenced missing capability, not required by PC110.
4. Validate timing labels: exact numeric/formula evidence is L3; range choice,
   emulator model or macro ratio is L2; order-only is L1. Report unupgradable
   L1 and corrections to false higher claims under the owner's existing policy.

## Exit

Every model and affected form has a finite disposition and owned regression.
Complete unit tests close an S; changed product execution requires the normal
integration/artifact closure. Do not promise all-family completeness from the
selected products' boot success. Original manuals remain external.
