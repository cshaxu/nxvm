# T457 S3: Release Artifact Without Compiler Debug Information

## Correction

S2 incorrectly treated compiler debug information as necessary for NXVM's
runtime debugger.  They are independent: the debugger is product code, while
DWARF/PE debug sections are build metadata.  S3 changes only the current
artifact publication boundary.

`current-gcc` now configures CMake `Release`: its cache records
`CMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG`, with no `-g`.  The current-artifact
target links with `-Wl,--strip-debug`; this is necessary because the admitted
MinGW static-link route can otherwise carry debug sections from linked objects
despite Release compile flags.  The publication guard accepts only `Release`.

## Product debugger retained

The Release artifact has no `.debug*` PE section after `objdump -h` inspection,
but exports `core_machine_debug_step`, `core_product_debug_set_trace`, and
`vm_machine_debug_set_trace`.  The optimized-tree `core-product-debug-target`,
`vm-debug-target`, and `vm-debugger-recording-lifecycle` smokes pass 3/3.
Thus pause, step, break/watch, trace, recording and fault diagnostics remain
product behavior; only compiler metadata and unattended developer trace work
are excluded.

## Publication guard and artifact

The guard rejects `RelWithDebInfo` with `Current NXVM artifacts may be
published to build/output only from Release.` and the rejected check leaves the
artifact hash unchanged.  The rebuilt `build/output/nxvm_0_5_0457.exe` is
SHA-256 `B1DC3B723CC03B19E8C9298D08B9909D5182D9BB8EEEEBE9D8016115F8E67455`.

## Full regression

The Debug `current-gate` passes 293/293 in 105.12 seconds.  Its Debug route
intentionally retains Core trace-provider observability; it cannot publish the
Release artifact.
