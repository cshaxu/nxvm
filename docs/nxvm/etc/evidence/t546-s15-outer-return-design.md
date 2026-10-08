# T546 S15 Outer-Return Predicate And Cleanup

## Authority And Scope

The primary sources are Intel's *iAPX 286 Programmer's Reference Manual*,
210253-006, Table 7-4 and section 11.2, SHA-256
`9C6067E777AE694D5F71D8ADE23014558BE4E9D156041C778AED84C1246D8538`,
and Intel's *80386DX Programmer's Reference Manual* (1990), section 6.5.2,
Table 6-2, and section 6.5.3, SHA-256
`9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1`.
The original PDFs remain in the external manual archive; this record stores no
manual copy.

Both manuals require an interlevel return to accept a nonconforming return CS
only when its DPL equals the return selector RPL, and a conforming return CS
when its DPL is no greater than that RPL.  Both also require retained data and
nonconforming-code segment registers whose DPL is more privileged than the
new CPL to become null; conforming code remains usable.  The 286 applies this
to DS/ES and the 386 extends it to FS/GS.

## Current Owner Finding

`_ser_ret_far_outer` and `_ser_iret_protected_outer` rejected every
conforming descriptor and did not reject a nonconforming descriptor with a
DPL different from the selector RPL.  Neither route invalidated retained
DS/ES/FS/GS caches after successful privilege return.  Call-gate admission was
separately inspected: its current target-DPL predicate already implements the
manual's `target DPL <= CPL` rule and is not changed by this S.

## One Owner And Publication Boundary

The existing two outer-return handlers stay the sole return paths.  They each
use the same source-derived predicate, publish CS/SS/EIP/FLAGS only after all
faulting reads and descriptor writes succeed, then call one private cleanup
helper.  Cleanup is infallible and has no lookup, I/O, exception, timing, or
public-contract effect.  It preserves the existing null-cache convention
(`flagValid = false`, selector zero) rather than adding a cache or a second
segment-load route.

## Regression Matrix

`cpu_outer_return_smoke` covers RETF and IRET on 80286 and 80386, 16-bit and
386 32-bit return forms, conforming return descriptors at DPL 0 and 3,
nonconforming DPL mismatch delivery, and DS/ES plus 386 FS/GS invalidation
while retaining a conforming ES/GS cache.  Existing outer-return tests retain
nonpresent, table/type and stack-fault delivery checks.

This is a semantic protection repair.  It allocates no instruction timing and
therefore neither upgrades nor downgrades a timing classification.

## Focused Verification

Both MinGW Release widths built the changed CPU owner and its direct
receivers.  On each width, `x86-test-cpu_outer_return`,
`x86-test-cpu_protected_iret_state` (4,032 flag cases), and
`x86-test-cpu_transfer_boundary` passed.  The `src/x86` and `test/x86`
manifests also verified after their corpus revisions were updated.  The four
fixed PC products' x86/x64 artifact targets were rebuilt; no MyNES artifact,
INI, media master, or snapshot was changed.

The full repository-only unit qualification remains required by the active
packet and is not represented by this focused result.

## Repository-Only Unit Attempt

The project `run-unit-tests` aggregate has a fixed 300-second containment and
timed out on x64 after 243 of 506 tests, all passing to that point.  The same
complete 506-test label was then run in three non-overlapping CTest partitions
(170, 170, and 166) per width.

- x64: all 506 passed at `-j 8`.
- x86: the first 170-test partition passed at `-j 8`.  The two later
  partitions first exposed host-side 15-second containment at `-j 8` for
  `unit.core-machine-cli-sti-s48-smoke` and
  `unit.machine-8086-instruction-timing-ledger-smoke`, despite both printing
  their success markers.  Each exact test passed at `-j 1`; rerunning those
  complete, non-overlapping 170-test and 166-test partitions at `-j 4` then
  passed 170/170 and 166/166.  The final x86 qualification is therefore
  506/506, with every test identity run by CTest in its passing partition.

The fixed `run-unit-tests` aggregate remains unsuitable as a single proof
command here because its global 300-second containment stopped its x64 run
after 243 passing tests.  This S neither changes test timeouts nor claims a
test-runner repair: the complete proof is the bounded, non-overlapping CTest
partition set above.  No CPU assertion failed.
