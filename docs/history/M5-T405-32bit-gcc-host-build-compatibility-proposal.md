# 32-Bit GCC Host-Build Compatibility Verification

## Purpose

After the DeskPro 386 Model 40 L3 audit and before the distinct 8088/PC/XT
program begins, establish whether the current NXVM product can be built and
run as a 32-bit host executable with GCC. This is a host-portability and
integer/pointer-boundary check: it does not change the bitness of an emulated
CPU or redefine any selected machine's L3 result.

## Required scope

Use an isolated x86 GCC toolchain and an isolated build tree. Record the exact
compiler identity, target triple, CMake generator, dependency architecture,
configuration command, and executable architecture. Build the current source,
run the current-gate suite, and run bounded product checks for every then-closed
baseline profile (at minimum profile loading, session creation, reset, a
short controlled execution interval, and clean shutdown). Preserve the 64-bit
build and its artifact untouched.

Inspect every failure for an incorrect host-width assumption: pointer or
`size_t` narrowing, `uintptr_t` conversion, file-size/offset handling, memory
capacity conversion/allocation, integer formatting, ABI-layout assumptions,
and third-party dependency architecture. A configuration value representable
by the product's 64-bit configuration type but not allocatable by a 32-bit
host must fail explicitly at the allocation/validation boundary, never wrap or
silently truncate.

The candidate ends with a source-backed compatibility decision: pass with
recorded x86 evidence, or a complete bounded defect/limitation ledger with
each item either fixed, explicitly rejected at its owner boundary, or
transferred to a separately admitted repair task. Any necessary portability
repair follows the normal owner-first rule and may not be hidden by reducing a
test configuration.

## Non-goals and stop conditions

Do not add a 32-bit release promise, alter guest CPU semantics, alter profile
selection, change current L3 contracts, import or commit toolchains/libraries,
ROMs or guest media, or make the x86 toolchain a runtime dependency. Do not
replace the native-width source model with 32-bit types merely to compile.

Stop and transfer if a required dependency has no legal x86 build, if the
result depends on a protected or machine-local asset, if a repair changes a
closed baseline's L3 contract, or if the environment cannot provide a
reproducible GCC x86 target. The evidence must distinguish an unavailable
toolchain from a product portability failure.

## Evidence standard

Require clean-tree build provenance, compiler and binary target proof,
configuration/build/test commands and results, x86 current-gate result,
profile smoke records, a 64-bit-width similar-issue sweep, and a retained
pass/fail decision. A successful compilation alone is not compatibility
evidence; an emulator or a guest's 32-bit execution is not a substitute for a
32-bit host-process result.