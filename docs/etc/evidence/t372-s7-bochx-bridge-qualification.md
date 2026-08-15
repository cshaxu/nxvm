# T372 S7: Bochx Bridge Qualification

## Decision

The existing optional Bochx/Bochs research gate accepts a bounded,
media-free real-mode CPU experiment manifest.  The local manifest check
validated the external-reference identity fields and a finite instruction,
wall-clock, no-progress and ring-buffer budget.  No firmware, guest media,
whole-machine boot, raw trace, timing scalar or product artifact was used.

The historical paired-step bridge itself is not currently admitted for
execution.  Its project-owned historical glue requires a retired NXVM CPU,
memory, interrupt and port-interface surface that the current source tree no
longer exposes.  Rebuilding that adapter is an optional, separately admitted
developer-tool migration; it must preserve the differential-debug policy's
process, trace and cleanup containment.  It is not a prerequisite for the
current 5170 implementation work.

## Permitted Later Use

If a later task admits that migration, the bridge may compare a bounded CPU
microprobe at explicit checkpoints: selected registers, masked FLAGS,
segment/control state and a declared linear-memory window.  It cannot validate
POST, BIOS, disk boot, PIC/PIT, device initialization, platform behavior or
physical timing.  Any divergence still requires a project-owned regression
probe and a primary-source disposition.

## Containment And Similar-Issue Sweep

The sweep covered default-build isolation, provenance gate, CPU-versus-platform
oracle boundary, firmware/media exclusion, declared budgets, temporary-output
cleanup, trace retention and tracked-file leakage.  The local executable and
object output were removed after the manifest check; no external source,
patch, path, hash, manifest or generated artifact entered Git.  No S1
physical residual, timing value or Model-339 L3 conclusion changes.
