# M5 Core Timing-Input Qualification Closure

## Purpose

Close the construction-time timing qualification boundary before later device
work: Core must accept one frozen, machine-neutral timing plan capable of
representing sourced L3 numbers/formulas and reference-derived L2 proportions;
VM profiles must provide the best qualified value available and may reach the
bounded L1 escape only through an explicit unsupported configuration.

The task does not add a VM scheduler, runtime controller setters, a second
guest clock, device mirrors, or provenance branches in Core.  A profile
materializes data once, Core copies it, and each existing device owner consumes
the copied value through the existing deadline composition route.

## Frozen Coverage Universe

The ledger covers every live timing input consumed by the current Core
scheduler for the four product profile families: IBM 5160 XT, IBM 5170/default
PC/AT, Model 339, and DeskPro Model 40.  The rows are CPU/time-axis and
instruction/transaction timing, DMA/PIT/RTC/KBC/VADP/provider clock ratios,
FDC byte-service conversion, HDC personality service input, and Model-40 D4.
PIC immediate publication is recorded as non-duration work, not converted to a
fictional timer.

For each row, the ledger records its Core input, profile value, primary/manual
or board evidence, corroborating external reference when needed, current code
owner, and one disposition: Manual-L3, Other-L2, explicit L1 fallback, or
non-time/unsupported with a named receiver.  The absence of a manual value is
not permission to manufacture a duration.

## Required Subtasks

1. **S1 - Qualification ledger and source audit.** Read the original sources,
   current Core/profile code, and available 86Box, MAME, PCjs, Bochs and QEMU
   implementations.  Freeze the complete input ledger and List 2 code routes.
   Identify every active built-in L1 route and every product row below L2.
2. **S2 - Normalize the copied Core input seam.** Repair only a demonstrated
   semantic impediment in the existing plan/configuration seam.  A Core field
   that can receive a manual value must not be named or shaped as L2-only;
   provenance remains profile evidence, not runtime behavior.  Add direct
   construction/owner tests without a generic setter layer.
3. **S3 - Product-profile qualification closure.** Supply or validate the
   strongest justified values for every built-in profile.  In particular,
   research default-PC/AT ATA service timing against primary material and
   available external implementations; freeze a proportional L2 input only if
   that evidence supports it.  Otherwise retain the existing immediate owner
   boundary explicitly and transfer the missing source to its earliest receiver.
4. **S4 - Matrix and product closure.** Assert that every supported profile
   reaches no active L1 compatibility disposition over its finite waiting
   matrix, that Standard/Turbo share the sole Core path, and that no profile
   injects guest ticks.  Run complete repository-only unit, external-asset
   integration, governance, and the stripped `nxvm_0_5_0508.exe` release
   build.

## Exit Criteria

The durable ledger exhausts the frozen input universe.  Every product input is
Manual-L3, Other-L2, a truthful non-duration boundary, or an explicit
unsupported transfer; no ordinary built-in profile enters L1 compatibility.
Core has exactly one copied timing-plan route and one time writer, while VM
retains only profile construction and host pacing.  The task reports code-size
change, owner paths, full unit/integration results, and the stripped 0508
artifact.
