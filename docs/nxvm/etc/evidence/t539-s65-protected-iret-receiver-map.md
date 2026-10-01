# T539 S65 protected-IRET receiver map

`machine_protected_iret_smoke.c` remains a Core-machine board receiver.  It
constructs protected-mode descriptor tables and IRET frames through the public
machine contract, so it cannot honestly be recast as a CPU-local fixture.

`core_machine_iret_s51_smoke.c` deliberately includes that receiver to reuse
the same board setup for its existing IRET composition coverage.  The source
rename and CMake target make this ownership explicit; no production API,
instruction algorithm, timing rule, or second setup path is added.

The receiver emits `M5:T539:S65:PROTECTED-IRET:OK` in addition to its retained
historical IRET marker.

The focused x64 and x86 receiver/includer pair each passed.  Complete
repository-only unit suites then passed 426/426 on each width, with the named
protected-IRET marker present in both.  On both widths the T332 lifecycle,
T344 registration and historical-shape, VM lifecycle, and Core CPU/PIC
authority gates passed; documentation governance and `git diff --check` also
passed.  This is a test/CMake/documentation rename: no production/API, Shared,
firmware, asset, INI or EXE input changed, so no EXE rebuild is required.

The scoped code/test/build paths add six lines and remove six lines; the
separate receiver-map evidence is 26 lines.  The similar-issue sweep covers both
direct protected-IRET consumers.  The adjacent INT-entry and VM86 groups have
different guest-frame ownership and remain assigned to S66 and S67.
