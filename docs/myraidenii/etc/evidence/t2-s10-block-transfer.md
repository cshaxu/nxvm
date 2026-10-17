# T2 S10 Native Data-Block Transfers

Baseline `01c5cd23a`. This batch qualifies the six native MOVSB/MOVSW,
LODSB/LODSW and STOSB/STOSW data-block forms in V09, not complete V03/V09,
mode, timing, Machine admission or gameplay.

## Source And Retained Implementation

The pinned [NEC October 1986 manual](../research/references.md), SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`,
printed 12-15--12-20, defines REP ignoring Z, zero-count suppression,
MOVBK source/destination, LDM accumulator and STM destination semantics.
IX/IY move by one/two according to DIR; CW decrements only under REP.
Source defaults to DS0 (Intel DS) and permits segment overrides; the
destination stays DS1 (ES). Flags are unchanged. Section 6 supplies the
interruptible iteration boundary and retained restart prefix contract.

Review found the existing handlers already express these native operations.
The test uses the sole `_m_movs`, `_m_lods`, `_m_stos`, `_kas_move_index`,
logical/physical memory and instruction/interrupt routes; no production
change is required. There is no new executor, callback, interface, state,
helper, mirrored production register image or workaround. Original handler
and trace/table style remains byte-identical.

S8's source-backed L2 data span across FFFF is inherited, not promoted to
original-chip measurement. Carry/F2/LOCK on non-comparison forms, long streams,
mode/8080 and electrical prefetch/timing remain their original receivers.
An ignored segment prefix on STOS is retained as existing deterministic
behavior; it never redirects the ES destination. No undocumented silicon
claim or external implementation import is made.

## Independent Owned Proof

The original ROM-free CPU unit has one-MiB real memory, a separate expected
buffer and a bounded record of actual data transfers. Expected data is computed
bytewise using independently expressed addressing and architectural operations;
it does not call CPU memory/index helpers or use a second decoded interpreter.
Full memory comparisons after every iteration detect missing, excess or
incorrect writes. Full CPU comparisons detect unintended flags, register,
segment, upper-half or count changes. Provider checks reject invalid bus
width/address/odd-word calls instead of hiding them behind modulo storage.

- 960 cases combine six forms, five segment selections, four index starts
  (0, 1, FFFE, FFFF), both DF directions and single/REP-zero/REP-one/REP-three.
  Source/destination parity combinations include split word cycles. Different
  initial CF/ZF/OF values prove REP does not become conditional on Z.
- Pure preview preserves CPU state and causes no data transfers. Each actual
  iteration checks exact data-provider address, width, direction, acceptance,
  final index/count/PC and full expected memory/CPU.
- Six additional forms start both operands at physical FFFFF with CW=FFFF.
  Actual byte/word transfers prove the 20-bit bus tail and sixteen-bit count
  decrement without using a whole-loop time budget as coverage.
- Sixteen overlapping REP MOVS cases cover byte/word, both directions and
  offsets zero through three. The expected buffer advances in architectural
  iteration order, not host memmove semantics or a bulk-copy shortcut.
- Every applicable source/destination callback position of all six forms is
  refused on the first and second REP iteration. Word cases are deliberately
  odd. Architectural checkpoint rollback is checked by full CPU comparison;
  already accepted write bytes are reconstructed from provider records and
  retained, with the failed final request unaccepted and no subsequent replay.
- All six forms take NMI and maskable INT at the actual first REP boundary,
  save IP 0100, execute a real IRET, restore SP and MD, then perform only the
  remaining iteration. Final accumulator, indexes/count, destination data and
  return PC are checked. The three stack reads during IRET are distinguished
  from block data accesses, not counted as repeated block work.

The similar-issue sweep inspected six primary handlers A4/A5/AA/AB/AC/AD,
the three semantic helpers, `_kas_move_index`, `_m_test_string`, repeat
admission/restart and common logical/physical owners. Distinct comparison
and port helpers remain because they have different flag/external side effects.
No duplicate production path was found or created. This is qualification
progress on an existing mechanism, not a fabricated production bug fix.

## Verification And Artifacts

Shared `871fdfe1b` is pushed. Both original Release Ninja/ccache graphs build
all targets. Complete repo-only unit suites pass on their first full run:
x64 663/663 in 84.73 seconds and x86 663/663 in 88.97 seconds, including all
174 X86 Core units and receiving App units. Architectures run sequentially
with four concurrent tests; no retry, budget/timeout change or assertion
removal was needed. The new unit passes in 0.63/1.56 seconds in the complete
suites and directly emits `V30:BLOCK-TRANSFER:OK` in both widths.

Source/test manifests, registered Types/inward-dependency/ownership/negative
gates, documentation, relative links and whitespace pass. New test code builds
under the existing strict warnings, with no new waiver. The historical CPU
instruction-file warning exception is unchanged; no global waiver-free claim
is made. Actual source/test counts, excluding manifests/docs/artifacts:
production +0/-0; tests +317/-0/net +317 (316-line owned unit and one
registration). The test's expected image and transfer records observe an
existing mechanism, not new production state or a test framework.

No production source/build input changed. The retained eight PC 0563 artifacts
match every [S9 identity/hash](t2-s9-pc-artifacts.json); the MyNES pair also
remains byte-identical. No replacement EXE was needed or manufactured.
Both original graphs retain default profile selection and the same persistent
cache. INIs, external ROM/media, snapshots and all other Shared packages remain
unchanged. No external-ROM integration, desktop/RDP, physical audio, original
board/game qualification or MyRaidenII EXE is claimed.

The test SHA-256 is
`1D074BA64306E080869C9ACFD3DF3B836315FAD8D8F84716372ED32F8A02F837`.
Ignored build/unit logs under `build/myraidenii/t2-s10-*` are retained for T2
acceptance. No manual render, trace, archive or protected input was generated.
Coordinator acceptance must reconcile the complete batch and unchanged T2
universe against actual committed changes, not merely these passing tests.
