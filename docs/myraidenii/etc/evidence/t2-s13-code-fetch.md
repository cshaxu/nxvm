# T2 S13 Native Instruction Fetch Start And Span

Baseline `f96c1b3cb`. This batch consumes the V04/V14 logical native fetch
start/span receiver found by S12, not physical prefetch arbitration, clocks,
all native instruction semantics, mode/8080, Machine or board qualification.

## Primary Source And Reproduction

The pinned [NEC October 1986 manual](../research/references.md), SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`,
printed 3-4--3-5, describes the program prefetch pointer as a sixteen-bit
counter relative to the program segment. Printed 4-1--4-3 supplies the native
memory width and lane context. This does not turn the existing semantic
prefetch cache into a cycle-accurate queue. S8's data-tail behavior retains
its separately sourced L2 grade; instruction offset wrap is a distinct fact.
No external implementation, table, manual payload or protected input is imported.

The new actual CPU test fails against the original production baseline with
`V30:FETCH-FORM:0:0:0:fffe`. The instruction is native NEC INS, `0F 31 C9`,
starting at FFFE: the suffix is at FFFF and its ModRM belongs at offset zero.
NEC admission passed the unnormalized value 10000 to `_s_read_cs`, obtaining
the wrong address/form before its handler. This is the executed reproduction
of S12's source-only finding, not a new game-ROM failure.

The existing `_kma_real_legacy_segment_wrap` already splits an instruction
span which starts inside the sixteen-bit segment and crosses its end. Its
condition deliberately excludes starting offsets beyond FFFF. Native logical
translation also deliberately retains data offsets, including the S8 data-tail
model. Normalizing every data address there would break that distinction.

## Sole Production Owner And Caller Sweep

`_s_read_cs` now normalizes the V30 starting offset before deriving linear and
recorded-instruction offsets or consulting prefetch. Its existing fallback
still supplies instruction-fetch provenance to the same logical span owner;
the physical lane/20-bit split and provider failure owners are untouched.
S12's caller-specific mask is removed. There is no new helper, decoder, cache,
state, public function/type/layout or second memory path. Intel paths execute
their original start/limit/fault order, without this V30 branch.

The sweep uses `rg -n -F '_s_read_cs(' src/x86-core/cpu/cpu_instructions.c`,
the surrounding complete function bodies, code advance and preview, logical
read/span predicates, `ExecInit` and CPU prefetch reservation/advance. All
twenty-six direct call sites are classified:

- `_kdf_code` (one): the common immediate/displacement stream, with the existing
  `_kdf_skip` owning PC advancement. Fetch normalization does not replace PC state.
- NEC 0F (two), BOUND (one), POP group 8F (two), FF groups (two), and actual
  dispatch (two): reuse the corrected fetch owner. Native witnesses exercise
  their relevant admission/read classes without patching the handlers.
- Preview (one): uses the same fetch owner on an observational clone. Its
  older-family caller mask remains for Intel semantics; it is not another
  V30 reader, data route or state owner.
- Intel 0F (two), LOCK's protected/32-bit probe (seven), Intel system/bit groups
  0F00/0F01/0FBA (three), BSWAP (one): retain their genuine Intel-only admission
  and behavior. The native LOCK handler does not run that Intel probe loop.
  Full LOCK/long-prefix semantics remain in V05/V14, not certified here.
- FPU escape (two): still routes through the common owner; original V30 FPU
  remains unqualified/non-eligible. No NEC FPU is fabricated by this fetch fix.

`ExecInit` bounds the initial window by page/segment remainder before using
the existing logical reader. The V30 odd-start five-byte semantic window is
unchanged. Native reservation records bookkeeping only; the separate refill
read in `advance_prefetch_reservation` is 8088-only. Electrical overlap and
prefetch timing retain V14/V15. Data/stack and CS-overridden data continue to
use their existing data provenance; the whole S8 suite remains enabled.
No other V30 instruction-code reader or handler-local fix is introduced.

## Owned Proof And Its Limits

The existing `cpu_v30_memory_transfer_smoke.c` is extended, retaining its real
1-MiB backing storage, lane bounds, records and explicit provider refusal.
There is no duplicate fixture/framework, new test registration or borrowed App
test. A separate contiguous execution is a relational witness for fetch
equivalence, not an independent oracle for every instruction's semantics.

- Thirty-eight encoded streams include one-/multi-byte primary forms,
  immediate/displacement/ModRM, PUSH/POP/ENTER, LEA/BOUND/FF, relative control
  forms, NEC register/immediate/memory/implicit encodings. Five segment choices
  (none/ES/CS/SS/DS), three code bases (0, 80000, FFFF0), eight segment-tail
  starts FFF8--FFFF and odd 0101 produce 5,130 comparison cases plus 570
  contiguous witnesses: 5,700 real executions and observational previews.
- Expected encoded length and recorded bytes are checked directly against the
  input stream. Preview preserves the complete CPU and live cache validity and
  causes no data/phase effects. Actual provider records must stay inside the
  sixteen-bit program window modulo the twenty-bit bus, with legal native lanes.
  Full CPU results normalize only the expected next IP; data/stack/extra-segment
  memory and the CS data operand are compared with the contiguous witness.
  This does not qualify every native result/flag/control form.
- Three-base sequential MOV/NEC TEST/NOP streams retain the existing prefetch
  window for the second instruction, cross FFFF through uncached suffix bytes,
  then refill after PC wrap. The second instruction must not invent another
  initial-prefetch operation. No electrical queue/timing claim follows.
- Every provider fetch and observational-read position of the wrapped NEC
  immediate TEST stream is refused in each base. Tests reject vacuous zero
  counts, bound the accepted prefix, check restored complete CPU/provenance,
  no data/write effects and no fabricated guest exception. Preview failure
  preserves the live CPU/cache and does not publish a diagnostic stop.

While expanding the test, BOUND initially had a valid range only in DS, so the
new segment-prefix witnesses correctly exposed a fixture setup error. Explicit
range data is now supplied in all four selected segments; no production BOUND
behavior or assertion is weakened. This was resolved before complete suites.

## Delivery And Verification

Shared `5c9695356` and NXVM artifacts `c7ab4fdd4` are pushed.
Both original Release Ninja/ccache graphs build all targets. First complete
repo-only unit suites pass x64 665/665 in 290.26 seconds and x86 665/665 in
93.96 seconds, including 176 X86 Core units and receiving App units. The
expanded native memory unit passes in 0.28/0.72 seconds. Architectures run
sequentially; no retry, budget change or assertion removal is used.

Source/test manifests and registered Types, inward dependency, ownership and
negative gates pass. A separate `-Wall -Wextra -Wpedantic` syntax inspection
retains only the original warning classes; the fetch normalization adds no
warning. Existing instruction-file exemptions are not changed or promoted
to a waiver-free whole-tree claim. Governance, links and actual-change review
remain required for final acceptance.

The product guide's M0-era `NXVM` commit-target wording is aligned with the
already approved receiving-artifact process and root Execution Rules. The
active packet names that direct documentation correction. It authorizes no
undeclared NXVM source/configuration change and creates no new target or task.

Actual source +3/-1/net +2 and tests +180/-0/net +180 exclude manifests/docs/
artifacts; combined +183/-1/net +182. Five new test-local helpers share setup,
record checks, execution witnesses, form enumeration and cache/failure proof
within the existing fixture. There is no new production helper or state.
The increase is owned proof, not another interpreter or address implementation.
Source SHA-256:
`C303CEF038F3407A7D64E07A1DC410187CE6F0F7F8649BFFE1A24CF4B2EDD4C4`.
Test SHA-256:
`B04D2607BCD23D7BBAACF7A8E2A485E7247958679DDB6A37527727A112F3D684`.

Both original graphs link/deploy four PC profiles per architecture, reusing
CPU objects and restoring default after each width. The [artifact inventory](t2-s13-pc-artifacts.json)
records eight PE architectures, sizes and hashes at retained revision 0563.
Every EXE also passes product-name, version and lowercase-INI identity checks.
MyNES pair, all INIs/media/snapshots/ROMs and other Shared packages remain
unchanged. No external-ROM integration, desktop/
RDP, physical audio, original board/game execution or MyRaidenII EXE is claimed.
Ignored batch build/unit/warning/artifact logs under `build/myraidenii/t2-s13-*`
are retained for final T2 acceptance. Mode/8080, timing, complete native semantic
and slot qualification, Machine and real-board requirements remain open.
