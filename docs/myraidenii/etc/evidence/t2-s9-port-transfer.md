# T2 S9 Native Port Transfers

Baseline `337eb47b7`. This batch consumes the V09/V14 native I/O subfamily,
not the complete native instruction, mode, timing, Machine or game domain.

## Source And Contract

The pinned [NEC October 1986 manual](../research/references.md), SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`,
printed 4-3--4-4 and 12-25--12-29, supplies the source facts. I/O uses the
lower sixteen address bits with no segment addition. V30 words at even ports
use one word cycle; odd ports use two byte cycles. Scalar immediate addresses
are eight-bit starting addresses, not an eight-bit mask on the second cycle.
INM/OUTM preserve DW, move IY/IX by width according to DIR, and REP uses CW.
The input destination remains DS1 (Intel ES); output defaults to DS0 (DS) and
can use a segment override. No reference implementation was copied/imported.

The upper 256 port addresses are reserved by NEC. Synthetic FFFF->0000
vectors qualify the existing sixteen-bit provider address model, not real
hardware meaning or a documented peripheral in that area. Input segment
override is prohibited by the manual; the inherited ignored-override behavior
is retained/tested as a deterministic compatibility model, not NEC silicon
acceptance. Carry-repeat port combinations remain diagnostically rejected by
the same format/runtime admission owner. F2 port behavior and long-prefix or
mode interactions are not newly qualified. These retain V05/V09 receivers.

## Sole Owner And Failure Semantics

Only the existing `_p_input` and `_p_output` issue provider port transfers.
For V30 odd words they call their original byte path twice, low then high,
with a sixteen-bit second address. This is bounded to one split level: byte
calls cannot split again. Scalar and string handlers remain unchanged. Each
accepted byte has its existing completion callback and BEGIN/COMMIT phase
before the next byte starts; a refused byte has CANCEL and no completion.
No helper, public API/type/layout, persistent state, queue, transaction,
replay mechanism or second I/O route is added. Old Intel and 486 buffered
paths remain their original routes.

Provider failures remain existing diagnostic stops, not invented guest
exceptions. Accepted external effects stay accepted; registers and current
instruction/index/count roll back through the existing instruction checkpoint.
A complete previous REP iteration remains committed. A partial input word
does not publish half an architectural accumulator; a partial output retains
the first accepted device byte. String input completes I/O before attempting
memory writes; a later failed memory write cannot undo or replay the input.
The host failure contract is not a claim about electrical retry hardware.

## Owned Proof And Similar-Issue Sweep

`test/x86-core/cpu/cpu_v30_port_transfer_smoke.c` uses original synthetic
instructions, real one-MiB memory and sixteen-bit port backing. The fixture
rejects odd-word provider calls and validates real port address, width,
direction, value, acceptance, completion ordering and every port phase.
It is local test data, not a production mirror or a test framework.

- All eight scalar byte/word immediate/DW forms run with even/odd, immediate
  FF->0100 and reserved-tail provider-model addresses. Accumulator width,
  flags, count and DW preservation, pure preview and no preview I/O are checked.
- All four byte/word port strings run single and REP, both directions, both
  memory/port alignments, ordinary and FFFF index starts, five segment choices
  and three iterations. Memory/provider addresses, values, complete counts,
  sixteen-bit index wrap, upper register halves and unchanged flags are checked.
  Input memory begins blank, so stale/pre-seeded output cannot satisfy proof.
- All twelve scalar/string forms refuse each port-cycle position. Every odd
  memory half of word input/output is separately refused: output does not reach
  I/O after failed source read; accepted input cycles and an accepted memory
  prefix are retained after a failed destination write.
- Every string form has zero-count proof and carry-prefix diagnostic proof.
  Actual NMI/INT acknowledgement, native frame, IRET and second REP iteration
  demonstrate exactly-once continuation at the existing one-iteration boundary.
- Every byte/word string fails at each port position in its second REP
  iteration, retaining the first iteration and accepted second-byte prefix
  without another read, duplicate completion or register/index/count advance.

Sweep queries covered `transfer_port|complete_port` throughout CPU C/H,
`_p_input|_p_output|_p_ins|_p_outs`, scalar handlers E4--E7/EC--EF, string
handlers 6C--6F, repeat admission, external-cycle publication and memory
preflight. The only provider issuers are the two existing port owners; their
input/output side-effect ordering is meaningfully distinct. 486 SMM/restart
and buffered-write behavior remains profile-specific, not a V30 fallback.
Mode/8080, electrical bus/prefetch, full timing, LOCK and unqualified prefix
contexts remain in their original complete ledger rows.

## Verification And Delivery

Shared `7a9433dc1` and eight NXVM artifacts `2f0ac6b63` are pushed. Both
original Release Ninja/ccache graphs complete full builds. Final complete
repo-only unit suites pass x64 662/662 in 76.23 seconds and x86 662/662 in
80.14 seconds, including all 173 X86 Core units and receiving App units.
Each first full run passed 661/662 but timed out in the unchanged thirty-second
`x86-nxvm.verifier-negative` gate while both architectures' builds/suites
overlapped: x64 515.53 seconds total, x86 85.53 seconds total. The x86 gate
passed separately in 18.06 seconds. Full suites were then run sequentially
with four concurrent tests, without code, assertion or timeout changes;
the gate passed in 20.47/22.23 seconds. These are rechecks, not first-run
green results or proof of a specific host scheduling root cause.

The new actual-execution unit directly emits `V30:PORT-TRANSFER:OK` in both
widths and passes both complete-suite runs. Manifest, Types, inward dependency,
ownership and negative gates pass in the final suites. A separate syntax check
with `-Wall -Wextra -Wpedantic` reports no warning in changed port regions;
the legacy instruction-file warning exception remains, so no waiver-free
whole-file claim is made.

Both original graphs link/deploy four PC profiles per architecture and restore
default, reusing production objects. The [artifact inventory](t2-s9-pc-artifacts.json)
records actual eight PE widths, sizes and SHA-256 at retained revision 0563.
Product-name/version/lowercase-INI tokens were inspected in each EXE. MyNES
pair remains byte-identical; all INIs, media, snapshots, ROMs and other Shared
source/test packages are unchanged. No MyRaidenII EXE, external-ROM integration,
desktop/RDP, physical audio, original-board or gameplay validation is claimed.

Actual `git diff --numstat` excluding manifests/docs/artifacts: production
+13/-0/net +13; tests +383/-0/net +383 (382-line owned unit and one registration);
combined +396/-0/net +396. No production helper/state/interface is introduced.
The added test records observe existing provider effects; no mirror engine or
per-instruction bus workaround is present. Source SHA-256:
`DC045CEE97B1ED9992C4F17E9A6510E17A5A7B619FE07734C33A495D4C04DFCE`.

Ignored logs under `build/myraidenii/t2-s9-*` retain full build, initial failed
unit runs, isolated gate check, complete rechecks, artifact links and warning
review for T2 acceptance. No new manual render, trace, archive or protected
input was generated. Coordinator closure checks exact artifact hashes,
relative links, governance, whitespace and the complete unchanged T2 domain.
