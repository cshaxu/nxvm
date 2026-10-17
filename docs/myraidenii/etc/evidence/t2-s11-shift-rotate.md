# T2 S11 Native Shift And Rotate

Baseline `bf48635bb`. This batch consumes the seven-operation native Group-2
subfamily of V07: ROL/ROR/RCL/RCR/SHL/SHR/SAR, byte/word, one/CL/imm8,
register/memory and relevant failure contexts. It does not retire full V03/V07,
mode/8080, timing, Machine or board/game qualification.

## Source And Cause

The pinned [NEC October 1986 manual](../research/references.md), SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`,
printed 12-100--12-141, gives individual shift/rotate encodings, operations
and flag tables. In particular 12-102--12-105 assigns the complete CL/imm8
value to a temporary and repeats while nonzero; there is no five-bit mask.
The same rule appears for the other rotate/shift variable-count forms.
For SHL AX,32 with AX=1, the old private helper produces zero count and leaves
AX=1, contradicting this operation. The repaired original loop yields AX=0.
Immediate and CL counts use one existing count owner, not separate fixes.

NEC's tables and iterative operations also specify the final CF after long
SHL/SHR. Two inherited Intel undefined-CF markers incorrectly hid that fact
when count reached operand width. Only V30 is excluded from those markers.
One-bit forms define overflow; variable/multi-bit undefined OF/AF values are
not promoted to silicon guarantees. The tests mask those undefined flags,
but check every defined result, unchanged flag and the CF diagnostic metadata.
The existing deterministic value for an undefined flag remains unchanged.

Read-only web discovery reconfirmed the same NEC manual mirror and the NEC
V-series instruction manual U11301EJ5V0UMJ1. No secondary discussion or other
emulator implementation is an implementation authority here, and no external
source, table or binary was imported/copied/translated.

## Sole Production Owner And Retained Paths

The private `_a_shift_rotate_count` now admits full eight-bit count for V30
explicitly. It does not change the public 8086-semantics helper or grant V30
all early-Intel behavior. Existing seven arithmetic helpers, Group-2 handlers,
register decoder, memory RMW and execution/preview remain the sole paths.
The RCL/RCR modulo-9/17 optimization is retained: reducing complete rotations
preserves both value and carry, while the original count still owns flag
classification. It is not the erroneous modulo-32 input truncation.

The three 32-bit rotate branches remain Intel-only and are not reachable by
the native V30 form admission. Intel 8086/8088 full-count and 186/286/386/486
masked-count behavior is unchanged. 486 masked-zero write suppression remains
486-only. Native V30 zero-count memory RMW retains the existing unchanged-value
writeback model; the source does not prove its electrical/device side effects.
This is explicit retained model behavior, not a new hardware-accuracy claim.
The timing selector still returns V30 unallocated before Intel count models;
no timing provenance or Machine admission is silently added.

## Owned Proof And Failure Boundary

The CPU-owned ROM-free unit uses the existing real-storage fixture with local
provider records and refusal controls. A separate bit-at-a-time mathematical
oracle handles the seven operations without the production count helper or
decoder. Arithmetic/result and defined-flag comparisons use full CPU snapshots,
so unrelated register halves, count aliases, segments and control state cannot
silently change. The oracle's count is never masked to five bits.

- 14,336 vectors exhaust every count 0--255, seven operations, byte/word,
  CL/imm8 and both initial carry values on AL/AX.
- 3,780 additional vectors cover one/CL/imm8, all eight low/high-byte or word
  register codes (including CL/CH/CX aliases), five operand patterns, even/odd
  memory operands and all five source segment selections. DS/ES/SS use distinct
  bases; CS uses the real code segment. A wrong segment cannot pass accidentally.
- 2,240 memory vectors cover 0/1/8/16/31/32/33/255 count boundaries, both widths,
  CL/imm8, alignments and all source segments. Actual provider addresses, widths
  and counts are checked, including FFFF word tails under the accepted S8 L2
  model. Pure preview preserves CPU and performs no data transfer.
- 126 failure vectors refuse every applicable read or write part for seven
  operations, all three count forms and both widths. Odd word failures preserve
  the first accepted byte, reject the suffix and restore complete architectural
  CPU state, without a guest exception or replay. Source-read failure causes
  no write. Provider call counts bound accepted-prefix effects.
- Six reserved /6 register forms are rejected consistently by actual V30 execution and
  pure form scanning without a fabricated guest INT6. Five explicit old-Intel
  profile checks retain the differing SHL AX,32 result; complete repo units also
  cover their original count/flag/timing/bus paths.

The sweep also finds that reserved /6 memory encodings reach operand read
before the existing handler diagnoses the unsupported extension. Their memory
side-effect/admission ordering is not qualified by the six register vectors.
This belongs to the complete V03 reserved-form admission audit, which must
reconcile all native ModRM classes rather than add six instruction-specific
workarounds. No native-silicon ordering is inferred for an undocumented opcode.
It remains an explicit T2 receiving obligation, not silently accepted behavior.

The similar-issue sweep inspected `_a_shift_rotate_count`, all seven helpers,
six Group-2 handlers C0/C1/D0/D1/D2/D3 and their byte/word/profile branches,
`_m_write_shift`, RCL/RCR reductions, preview/form
admission and timing-selector guards. Separate 32-bit/486 paths have genuine
profile semantics and remain untouched. No helper, state, API, second ALU,
parser, clock, queue or interpreter is introduced.

## Verification And Delivery

Shared `36bba8907` and NXVM artifacts `b1fd77101` are pushed. Both original
Release Ninja/ccache graphs build all targets. First full repo-only unit runs
pass x64 664/664 in 274.31 seconds and x86 664/664 in 95.00 seconds, including
175 X86 Core units and receiving App units. Architectures run sequentially,
with no retry, budget/timeout adjustment or assertion removal. The new unit
passes in 0.37/0.36 seconds in those suites and directly emits
`V30:SHIFT-ROTATE:OK` in the pre-suite x64 check.

Manifests and registered Types, inward dependency, ownership and negative
gates pass. A separate `-Wall -Wextra -Wpedantic` syntax review completes
with no warning in the changed source regions; the legacy whole-instruction-
file warning exception remains. This is not a waiver-free whole-tree claim.
Governance, relative links, whitespace and actual-change review are recorded
for coordinator acceptance.

Both original graphs link/deploy four PC profiles per architecture, reuse
production objects and restore default. The [artifact inventory](t2-s11-pc-artifacts.json)
records eight real PE widths, sizes and SHA-256 at retained revision 0563;
product-name, version and lowercase-INI tokens were checked in every EXE.
MyNES pair, all INIs/media/snapshots/ROMs and other Shared packages remain
unchanged. No external-ROM integration, desktop/RDP, physical audio, original
board/game run or MyRaidenII EXE is claimed.

Actual `git diff --numstat` excluding manifests/docs/artifacts: production
+4/-3/net +1; tests +281/-0/net +281 (280-line unit and one registration);
combined +285/-3/net +282. Original handlers remain, with no production
helper/state/interface added. New test code owns independent semantic/failure
proof, not another runtime mechanism. Source SHA-256:
`EF9CFBB454EA6D7F7061DFCC7A4828F379D946E48BFFC9365436F917BE25A8B0`.
Test SHA-256:
`CBBF87F5BF145A346799EBFBB92E65803071717CDE3F77DD0A6F8318FEF15E27`.

Ignored build/unit/artifact/warning logs under `build/myraidenii/t2-s11-*`
remain for final T2 acceptance. No new manual render, trace, archive or protected
input was generated. Complete V03 reserved-memory admission, native modes,
timing and real-board requirements remain explicit rather than inferred from
the passing seven-operation family.
