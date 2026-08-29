# T499 S12 Full-Gate Restoration

## Rule

T499 cannot close or advance past this recovery unit until the configured gate
is completely green.  A failure belongs to its actual production owner; it is
not transferable merely because that owner is CPU, firmware, display, or test
infrastructure rather than a scheduler controller.

## Fresh Baseline

After a clean single-worker rebuild, the complete configured CTest suite ran
302 checks.  The baseline was 277 passed and 25 failed.  The first common
defect was Core physical-memory routing: an architected reset-ROM overlay was
selected after A20 wrapping, so the 80386 reset alias could resolve to the
ordinary low ROM.

The Core memory owner now resolves an explicit raw overlay before the ordinary
A20-wrapped route and passes the physical address that selected that provider
to the provider.  This preserves one Core memory route and does not add a
firmware-side alias or profile exception.  The focused reset-ROM,
default-PC/AT ROM materialization, IBM 5170 composition/CGA/FDC, and Model-40
FDD checks pass after the repair.

The next complete-suite run is 282 passed and 20 failed.  Its exact remaining
inventory is:

| Common owner | Failing checks |
| --- | --- |
| CPU reset/prefetch/timing | `timing-checkpoint`, `prefetch-locality`, `instruction-timing`, `instruction-timing-ledger`, `t359-s3-timing` |
| CPU real-mode flags, exception and interrupt delivery | `inc-dec`, `legacy-alu-s2`, `pushf-popf-s47`, `software-int-s50`, `iret-s51`, `imul-immediate-s56`, `tf-db-s60`, `bound-s54`, `real-exception-final-s1`, `real-ud-delivery-s1`, `hardware-delivery-s3`, `interrupt-return-composition-s4` |
| Firmware/Core lifecycle | `firmware-capability` |
| Core VADP registration transaction | `ega-registration-transaction` |
| DOS keyboard/display observation | `vm-dos-keyboard` |

The DOS keyboard check passed on its immediate focused rerun, so S12 treats it
as a repeatability defect until repeated runs either reproduce a production
race or establish that its observation is deterministic.  It is not removed
from the exit gate.

## Required Completion

For each owner group, reduce the shared root cause to one owner-local repair,
then rerun its focused checks.  Finally rebuild and run all 302 configured
checks serially.  The only accepted result is `100% tests passed`; no label,
test, profile, or media row may be excluded.

## Completion evidence

After the owner-local reset-ROM routing, profile FLAGS image, retirement
identity, and paused-display publication repairs, a fresh complete rebuild and
the configured CTest gate completed with **302/302 passed** (`100%`). The
recorded run is `Testing/t499-s12-final2-ctest.log`: 240.30 seconds wall time,
`current-gate` 1329.36 sec*proc, including all 15 media checks. No test,
profile, or media row was excluded.

## FLAGS reserved-bit audit follow-up

The same recovery unit found one over-specified test contract: it asserted that
the 80286 undefined FLAGS bit 15 read back as zero. The Intel 80286 reference
manual labels that bit undefined; the 8086 manual explicitly says that its
undefined PUSHF-image bits, including the high four, must be masked rather
than assigned a value. The 80386 manual specifies bit 15 and the 12--14
retained fields. The Core retains one profile-aware load/image
canonicalization boundary; tests now assert only the manual-defined image bits
and mask the 8086/8088, 80186/80188 and 80286 values for which the retained
primary source does not define a precise image. PUSHF/POPF, software-INT and real exception
frame checks pass, followed by a fresh configured run with 302 test records,
302 passed records and zero failed records.  No test was removed or excluded.

The follow-up manual reread corrected an overstatement in that record: the
8086 manual does **not** specify a high-four-bit `PUSHF` image. Its Flag-Images
section requires software to mask undefined image bits. The common Core
canonicalization now also rejects IOPL/NT on 80186/80188 loads, while retaining
them for 80286/80386 and leaving 80286 bit 15 unasserted. The focused
`pushf-popf`, five-profile `pushf-popf-s47`, `software-int-s50`, and `iret-s51`
checks pass after this correction. A fresh eight-worker configured current-gate
then completed **302/302 passed** (`100%`), with no excluded test, profile or
media row.

After the selected D4 reset-decode correction, the complete configured gate
was rerun serially: **302/302 passed** (`100%`), in 165.17 seconds. This
replay includes the Model-40/D4, controller, five-CPU ledger, DOS/media and
static checks; it excludes no test. It proves the current configured test
suite only. The separate owner-authorized Model-40 external-ROM semantic boot
replay remains open until it reaches its required FDD/boot terminal.

A fresh eight-worker replay after the D4 setup-decode receiver and the
per-generation FLAGS audit also completed **302/302 passed** (`100%`). The
latest CTest record contains no failed or not-run test; no test, media row, or
profile was excluded. CTest retains an older `LastTestsFailed.log` file after
a successful run, so that stale file is not evidence of a current failure.

## HLT no-deadline runner follow-up

An active L1 causal blocker or an otherwise idle halted CPU has no
source-qualified Core deadline.  The VM runner now yields the host only when
Core reports that it did not advance to a deadline.  It neither publishes guest
time nor restores a fixed-delay polling path; a subsequent sourced deadline or
external input remains the only way to progress guest state.  Scheduler, HLT,
competition, speed-policy and runner-cadence checks pass.  The following fresh
complete configured CTest run records 302 passed tests and zero failures in
70.70 seconds, with no excluded check.

## Execution-provider seam follow-up

The obsolete execution-provider `refresh` ABI member had no production caller
after scheduler convergence, yet kept a second-looking progression route in
the interface and test initializers.  It was removed rather than wrapped or
left dormant; the provider now has only its optional reset and completed-delta
`advance_time` callbacks.  The scheduler structural gate rejects a restored
`refresh` member or caller.  A fresh serial configured CTest replay completed
**302/302 passed** with zero failed or skipped checks in 313.29 seconds; no
profile, media row, or test was excluded.

## Public time-boundary follow-up

`core_machine_advance_time()` had no production caller, but its public-header
comment still suggested that composition could submit arbitrary source ticks.
The declaration is now Core-private and retained only for deterministic Core
tests; the public boundary exposes CPU execution and Core-selected deadline
advancement, not a VM/profile time-writer path.  The scheduler structural gate
rejects a restored public declaration.  After this boundary change, a fresh
serial configured CTest replay completed **302/302 passed** with zero failed
or skipped checks in 298.17 seconds.

## Cross-generation FLAGS canonicalization follow-up

The preceding FLAGS recovery was tightened so that the one Core-private
defined-field mask is explicit: `0FD5h` for 8086/8088 and 80186/80188,
`7FD5h` for 80286, and `FFD5h` for 80386. The same profile rule governs
PUSHF/interrupt-frame publication and POPF/IRET loading; an undefined bit is
only Core's deterministic zero canonicalization, never a hardware readback
claim. The direct regression proves the full known field set through a
PUSHF/POPF round trip for every profile while deliberately not asserting
undefined bits.

The earlier 87.31-second replay was discovered to have used binaries that
predated the final IRET expectation repairs and is withdrawn as completion
evidence. After relinking the current tree, a serial configured CTest run
completed **302/302 passed** with zero failed or skipped checks in 93.35
seconds; no test, profile, or media row was excluded.

The task artifact is the optimized stripped Release
`build/output/nxvm_0_5_0499.exe` (1,232,838 bytes, SHA-256
`CFC66FE9D74176CC0B9F98D4657BC779B155A71DCA6E893AFB97920F8B6957D5`). It
retains the runtime debugger and has no compiler debug section.
