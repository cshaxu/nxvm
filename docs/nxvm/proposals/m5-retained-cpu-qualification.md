# Retained CPU Family Qualification

## Goal

Preserve every existing CPU implementation, profile and selection table while
qualifying the CPU contracts required by XT, AT, DeskPro, default and future PC110. CPU
architecture remains independent of build-fixed board profiles.

## Current Product Context

T544 is admitted after the closed T543 four-App split. My5160, My5170,
MyDeskPro386 and NXVM deploy directly to their matching `assets/<app>/` roots.
Their firmware is embedded from external BYOB build inputs; adjacent NXVM.ini
selects runtime media, not CPU identity. The completed prerequisites are
[T539 chips](../history/M5-T539-independent-shared-chips.md),
[T540 boards](../history/M5-T540-shared-ibmpc-integration.md) and
[T543 Apps](../history/M5-T543-four-pc-apps.md).
CPU mechanisms and CPU-only tests already belong to `src/x86/chips/cpu` and
`test/x86/chips/cpu`; neutral execution/time belongs to x86/core. The existing
machine timing/decoder producers belong to test/ibmpc/board-common/composition,
not another Shared CPU executor. The [T544 convergence ledger](../history/M5-T544-retained-cpu-qualification.md)
records the frozen universe and batch dispositions; Current owns admission.
Neither extraction nor qualification restores runtime CPU/profile/YAML
selection or treats a successful product boot as CPU completeness proof.

## Scope And Batches

1. Inventory actual models, instruction forms, feature/timing tables and tests.
   Current enumeration has 8086, 8088, 80186, 80286 and 80386; 80188 and 486
   must not be advertised as implemented without a real receiver.
2. Reconcile each retained family's manual function/timing and code-gap ledgers,
   including exceptions, FLAGS, privilege, paging, reset and bus differences.
3. Inventory 80188 and the PC110-selected 486 prerequisite explicitly. Neither
   is a current implemented CPU. The queued PC110 evidence/component task owns
   selection and implementation of its documented 486 variant with a complete
   ledger, not an alias to 386. An independent 80188 remains a separately
   evidenced missing capability, not required by PC110 or a stub to add here.
4. Validate timing labels: exact numeric/formula evidence is L3; range choice,
   emulator model or macro ratio is L2; order-only is L1. Report unupgradable
   L1 and corrections to false higher claims under the owner's existing policy.

## Exit

Every model and affected form has a finite disposition and owned regression.
Complete unit tests close an S; changed product execution requires the normal
integration/artifact closure. Do not promise all-family completeness from the
selected products' boot success. Original manuals remain external.

## Initial S Breakdown

- S1: reconcile actual model, source, decoder, timing and regression inventories;
  freeze the convergence ledger and collect a fresh complete unit baseline.
- S2: audit shared admission, prefixes, FLAGS, reset, exception/delivery and
  fault-versus-retirement contracts across all five CPUs; propose each coherent
  repair batch before Shared edits.
- S3: reconcile 8086/8088 legal forms, state and timing, including their distinct
  bus/transfer assumptions; preserve one decoder and one timing selector.
- S4: reconcile 80186 additions and range-based L2 rules.
- S5: reconcile 80286 protected forms, transitions and qualified clock formulas.
- S6: reconcile 80386 sizes, paging, VM86, system state and timing formulas.
- S7: reconcile all accepted batches, run complete units/integration and verify
  affected four-App x64/x86 artifacts before T closure.

This is the initial sequence, not pre-admission of seven active packets. A large
family batch is split into later numeric S tasks when its finite inventory
requires it. No unresolved form is hidden by a passing representative smoke.
Lib/Common, MyNES, INI policy, board clocks and new CPU implementation are not
repair targets here. Shared code changes require the owner's concrete review;
read-only inspection and NXVM-owned qualification records can proceed first.
