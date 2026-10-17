# T2 S7 Native Conditional Repetition

Baseline `56f7df605`. This batch consumes the V05/V09 native comparison-repeat
subfamily: REPC/REPNC and REPE/REPNE on CMPS/SCAS byte/word, with one existing
iteration/safe point, prefix selection, failure and interrupt return. It does
not close full prefix, native, emulation, physical bus or timing qualification.

## Primary Source And Sole Owners

The pinned October 1986 NEC manual, printed 12-14--15 and 6-4 (PDF 59--60,
29), was rendered and inspected. Carry prefixes are 65 REPC and 64 REPNC,
defined for block comparison. Zero CX skips comparison; initial carry before
the first comparison is irrelevant; subsequent continuation uses the new
comparison carry. The existing F3/F2 zero conditions remain unchanged. Native
interrupt restart retains up to three preceding prefixes, including the
manual's excess-prefix example. No external implementation/table was copied.

The execution and observational scan formerly held separate prefix predicates.
One pure private recognizer now serves execution, preview, LOCK look-ahead,
saved-instruction inspection and restart bookkeeping. V30 64/65 mean carry
repeat; Intel 386/486 retain FS/GS. The profile-owned admission recognizes this
overloaded prefix rather than pretending that V30 has Intel 32-bit features.
The profile-independent metadata's Intel minimum is not rewritten to admit
64/65 on early Intel CPUs. Two appended private repeat enum values preserve
old values/layouts; no public API/type/layout change is introduced.

One comparison predicate is shared by preview/runtime carry-target admission
and existing 486 comparison bookkeeping. One repeat-result predicate replaces
twelve copies of the ZF condition and supports the two CF conditions. Existing
CMPS/SCAS arithmetic, register/index updates, memory paths and one-iteration
execution are retained. There is no second string loop, executor, host clock,
queue, worker, persistent latch or synthetic counter. The preview's local
carry-prefix selection tracks the last repeat prefix without new runtime state.
Undocumented carry-prefix targets stop through the existing diagnostic path;
their silicon behavior is not claimed or turned into an invented native trap.

The existing legacy string-interrupt bookkeeping now applies the documented
three-prefix restart to V30 while preserving the original early-Intel one-
prefix and 186/later behavior. Interrupt vectors, stack, acknowledgement and
IRET remain their original owners, not an App or test-only resume path.

## Owned Proof And Sweep

The ROM-free CPU unit runs all four comparison instructions against all four
conditions, counts 0--4, each possible early/normal termination, both DF states
and all initial CF/ZF combinations. Its expected branch result is calculated
from operand values, not copied from the production flags predicate. It checks
CF/ZF, exact CX/indices/PC, zero-read count, no writes and preserved high ECX.
A FFFF counter boundary terminates on the first comparison and proves 16-bit
decrement without an unbounded test. Existing Intel FS/GS are separately run
as segment overrides with no repeat/count change.

Prefix-order cases exercise repeat versus segment order, last repeat wins,
last segment wins and preview/truncation. Undocumented carry-prefix targets
are rejected by execution and preview. NMI and maskable INT are injected at
the first iteration boundary for one, two, three and four prefixes. Tests
inspect the actual stacked return PC, run actual IRET, then execute the final
comparison to its semantic end. More than three prefixes return to the last
three, not the original first prefix. No direct PC patch substitutes for entry.

Provider injection refuses each data-read position in the next comparison
after one accepted iteration, for byte/word CMPS and SCAS. It checks no extra
read/replay, no writes, preserved last accepted CX/indices/flags and original
instruction PC. Prior external read effects are not rolled back. Failure
returns through the original diagnostic/stop request owner.

The similar-issue sweep searches `src/x86-core/cpu` for both old prefix
recognizers, all comparison ZF conditions, every repeat enum use and every
restart consumer. Both old recognizers and all twelve duplicated comparison
conditions are removed. The independent X86 Debug disassembler's predicate
remains: it belongs to its separate parser, and NEC debugger eligibility is not
claimed here. Non-comparison string paths retain count-only REP behavior;
undocumented carry forms are not admitted there. Existing physical bus,
FFFF-start word, prefix-limit/LOCK, 8080/mode and timing rows remain explicit
receivers in the complete T2 ledger.

## Verification And Size

Shared `297ae1637` and eight NXVM artifacts `860fa1282` are pushed. Both
original Release Ninja/ccache graphs finish full builds. Complete repo-only
unit suites pass on the first full run, without retry or budget changes:
x64 660/660 in 291.27 seconds and x86 660/660 in 155.62 seconds. These include
all 171 X86 Core units and every other registered repo-only unit, including
manifest/Types/dependency/ownership/negative gates. The focused conditional
repeat executable passes both widths before the full suites; the final full
build relinks all other CPU consumers against the changed library as well.

All eight PC 0563 artifacts are linked/deployed through the original graphs,
reusing each architecture's production components between profile links. Both
graphs restore default. PE width, product/version/lowercase-INI tokens and
SHA-256 are verified in the [artifact inventory](t2-s7-pc-artifacts.json).
MyNES pair, all INIs/media/snapshots and the other Shared packages remain
unchanged. No external-ROM integration, desktop/RDP, physical sound, game boot
or gameplay validation was performed; no MyRaidenII EXE exists.

Actual source/test deltas use `git diff --numstat`, excluding manifests/docs/
artifacts: production +71/-56/net +15; tests +281/-0/net +281 (280-line owned
unit plus registration); combined +352/-56/net +296. The small production
increase represents two real prefix conditions while deleting duplicate
recognition/conditions. New private predicates own actual shared decisions,
not wrappers or a second state machine. Only private enum values are appended.
Source SHA-256: `6707D57E5BCD4B313881A59EE3D7447ED950F5AC2D814221B8955B660DC3E835`;
private header: `2E94787E01142690FE05304C3D673FA0E425EBF634FBC42A06436C7E7B2A8ACA`.
The instruction file's historical warning exception remains; a separate
`-Wall -Wextra -Wpedantic` syntax inspection reports no warning in the changed
helper/handler regions, not a waiver-free claim for the complete old file.

Build/focused/full-unit/artifact logs remain under ignored
`build/myraidenii/t2-s7-*` for the complete T2 qualification review. Three owned
manual render PNGs were removed after inspection; pinned manual/text remain.
Documentation governance, relative links, manifest identities and whitespace
are checked before delivery. Coordinator acceptance still requires actual
source/test/artifact and full-ledger review, not only these passing reports.
