# T444 S1 Current-Gate Regression Restoration

## Frozen Failure Set And Root Cause

The admission baseline at `2f8d12a9` had one T344 static failure (71 expected
direct constructors versus 75 found) and these 20 failures from
`current-fast-smokes-gcc`:

- `planar-parity-nmi-s3`, instruction-timing, instruction-timing-ledger, and
  `t359-s2` through `t359-s6`;
- 8086, 80186, and 80286 instruction-timing ledgers, legacy timing
  normalization, 80386 protected I/O timing, real-mode corpus, real-mode 386
  address, and real-mode 386 REP CMPS;
- control transfer, D4 platform, RTC/CMOS, and DMA/RTC authority.

The fast failures reproduce from a fresh `build/t444-clean` tree. They share
one test-fixture defect: each registered a 64-byte mapping from physical
`0xfffffff0`, a range that crosses the 32-bit physical-address limit. T442's
Core validation correctly rejects that request. The reset vector has exactly
16 addressable bytes before wrap, and every affected fixture's reset stub fits
that window; each test now declares 16 bytes. Core validation and all timing
values remain unchanged.

T344's four extra constructors are the 8086, 80186, 80286, and 80386 timing
manifest result producers. They are not historical fixtures, but are now
explicitly classified alongside the retained 71 historical entries. The
verifier rejects any unclassified direct constructor.

## Full-Gate Sweep

The first complete gate reached previously hidden stale static assertions.
The actual production route is unchanged and singular: VM publishes typed
display, DMA/RTC, FDC, and HDC declarations into its Core plan; Core's one
plan-materialization function calls the corresponding configuration owners.
The EGA and T296 verifiers now prove those two adjacent halves rather than
requiring an obsolete VM-to-Core direct call. T359's privileged classifier is
checked within its 80386 selection tail, T388's LSL checker matches the
already-passing `21,25,22,26` source/smoke values, and T345's generated strict
inventory is fixed at the observed 173 targets / 176 owner tests.

The T382 process-tree self-test could expire before its child recorded a PID
under concurrent gate load. Its deadline is five seconds; the test still
requires a deadline, marker, and dead child process, so cleanup coverage is
not relaxed.

## Verification

- Fresh GCC/Ninja `build/t444-clean` `run-current-fast-smokes`: 277/277 pass.
- Normal GCC preset `current-fast-smokes-gcc`: 277/277 pass.
- Normal GCC preset `current-gates-gcc`: all 69 specialized gates pass,
  including T344, T345, T359, T388, T382, artifact truth, and documentation
  governance.
- Artifact: `build/output/nxvm_0_5_0444.exe`, SHA-256
  `93A5307AD2A27CFEFDEAB491872FB7D918921B541CE73171F004F822E51EF230`.

## Similar-Issue Sweep And Minimalism

`rg` across tracked machine fixtures found every reset mapping at
`0xfffffff0`; all 64-byte forms were the frozen 20 failures and are repaired.
All other reset mappings already use 16 bytes or a non-overflowing range.
T344 enumerates all 75 direct constructors, while T345 consumes its generated
matrix exactly. No public ABI or production execution path changed.

The counted non-documentation change has 32 tracked build/test/tool paths:
114 added lines, 59 removed, net +55. Twenty fixture edits replace only
`64u` with `16u`; the positive lines make existing static checks name the
already-existing single plan-materialization owner and explicit 4-runner
classification. No wrapper, fallback, duplicate construction path, or
suppression was added.
