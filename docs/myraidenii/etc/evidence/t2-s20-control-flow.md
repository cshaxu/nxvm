# T2 S20 Native Branch, Call And Return

Baseline `55e2a795a`, after accepted S19. This consumes V08's native branch,
call and ordinary return forms, not breakpoint/interrupt return, PSW/mode,
physical clocks, neutral Machine admission or game execution. The original
playable MyRaidenII goal and full T2 ledger remain open.

Shared P1 `9588a8c13` is pushed. This product-local evidence delivery retains
the complete sixteen-field packet before coordinator actual-change review;
no receiving NXVM/MyNES P is required because executable inputs are unchanged.

## Source And Existing Ownership

The T1-pinned NEC V20/V30 October 1986 manual remains primary, SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`.
Section 3 defines next-instruction PC; printed 12-142--12-145 define native
near/far calls and returns, and 12-154--12-166 define jumps, predicates and
counted branches. CALL saves following PC, far CALL saves PS then PC, and
ordinary return restores PC/PS and adjusts SP. Branch/count operations preserve
flags. Timing columns are not newly implemented or certified by result tests.

The existing `_e_jcc`, `_e_loopcc`, `_e_call_near/far`, `_e_jmp_near/far`,
`_e_ret_near/far`, their common real/stack helpers and all primary/FF callers
were reviewed. Relative targets already wrap native PC, loops update low CX,
calls read their source before pushing, and ordinary returns restore their
native words. The independent matrix reproduces no production defect. There
is no source, public interface, persistent state, decoder/executor, provider,
Lib/Emulator, private PC/MyNES or alternate transfer-path change.

The old typed expected-transfer plan/checker is moved out of S19's unit into
the existing CPU-local actual-memory fixture. Both consumers now share its
address/width/order/phase, accepted-prefix and final-byte checks. The arithmetic
oracle, matrix and assertions are retained, not replaced by control-flow tests.
No test framework or test-only production entrance is introduced.

## Complete Admitted Matrix

`cpu_v30_control_flow_smoke.c` executes 463,982 successful input cases:

- 131,072 Jcc cases: all sixteen predicates, all 32 combinations of their
  relevant flags, every eight-bit displacement.
- 131,072 LOOPNZ/LOOPZ/LOOP/JCXZ cases: all four forms, 32 flag combinations,
  CX zero/one/two/FFFF, every eight-bit displacement.
- 256 short JMP and 65,536 each word-relative JMP/CALL displacements.
- 70,500 form/context cases across all fifteen forms: absent/four segment
  overrides, four code positions including FFFC/FFFF, five odd/even/wrapped
  stack positions, five target/cleanup seeds, all sixteen predicates/four
  counted branches and every legal near/far indirect ModRM. Immediate far
  targets and far returns include zero, original, alternate and FFFF segments.
- Ten explicit SS:BP source/stack-overlap near/far calls.

Every form/context case checks pure lexeme preview with no architectural/data
mutation, then full CPU/flags, CS base/selector, target PC, SP/CX aliases,
return addresses and the exact physical data-transfer plan. Calls execute a
real near/far RET at their target and must restore the complete original CPU
except following PC. The harness installs that RET and explicitly invalidates
prefetch; this is a return-mechanism test, not hardware self-modifying-code
coherence qualification.

Each context also refuses every actual code-fetch, data-read, data-write and
observational-fetch ordinal observed on its successful path. Host failures
retain complete CPU rollback/no guest exception or shadow and precisely the
accepted stack-write prefix. There is no retry or replay. Failed preview
remains nonmutating and does not turn into a runtime fault. The native data-tail
model stays S8 L2; physical prefetch/clocks and asynchronous target-fetch faults
are not inferred from these bounded entry/refusal tests.
Input TF/BRK is clear; general step and asynchronous interrupt priority are
not included in the unchanged-flag result contract.

## Initial Test Corrections And Similar-Issue Review

Strict compilation first rejected a signed/unsigned comparison in the moved
plan helper; the expression was corrected without relaxing warning flags.
The first matrix then exposed test setup, not CPU corruption: CALL SP uses SP
as both its input and stack pointer, but the harness initialized guard words
around the pre-alias seed. Setup now initializes the actual aliased SP before
each attempt, preserving the same expected rollback/accepted-prefix checks.

An initial plan also assumed a combined four-byte far-pointer read. The actual
`_m_read_pair` owner intentionally validates the whole operand then performs
two scalar word reads, retaining original bus phases and tail policy. The
coordinator narrowed the packet to that existing contract before full
qualification. The final oracle declares both word reads separately; an odd
pointer therefore has four byte transfers, not an invented merged [1,2,1]
sequence. No production path was changed merely to satisfy the first test.

All primary relative/direct/indirect entry points and FF /2--/5 were swept,
including register SP aliases, DS/SS selection, four overrides, pointer/stack
overlap and code/data/stack wrap. Illegal far-register encodings retain V03's
rejection tests. Intel protected and 32-bit branches retain their existing
units and separate contracts. BRK/IRET/PSW, mode/8080, full prefixes/LOCK and
time remain explicit receivers, not silently marked complete by V08 progress.

## Verification And Delivery

The original x64/x86 Release Ninja/ccache graphs are reused with -j8 and
complete repository units at -j4. The new finite unit has a pre-admitted
90-second containment budget; no existing timeout is changed.

Both first complete unit suites pass 670/670 (183 X86 Core units) in
109.21/114.38 seconds. Control flow takes 2.45062/2.12980 seconds; the retained
S19 multiply/divide matrix passes all 1,796,404 cases in 4.28376/4.75838
seconds. Both specialized targets retain 498 strict rows. Existing warning
exceptions are not claimed as removed. Manifest, Types, inward dependencies,
ownership/negative checks and product documentation/whitespace are verified.

[Results](t2-s20-results.json) retain both widths and full-suite counts;
[artifact identities](t2-s20-artifacts.json) retain fresh PE/width/size/SHA
checks of all ten current releases against S19. Production and every executable
input are unchanged; no artificial rebuild/redeployment or NXVM receiving P
is necessary. Both graphs retain the original default profile and persistent
cache. INIs, media, firmware and MyNES remain unchanged.

Production is +0/-0/net 0. Tests/registration are +380/-74/net +306: the new
unit +302, fixture +67, S19 caller migration +9/-74 and registration +2. The
canonical test manifest is +4/-3/net +1. These figures exclude records and
artifacts. No production helper, state or API is added; the moved private
test helper has two real CPU-test consumers and removes the old duplicate
owner. No physical desktop/RDP, audio, game or fresh full integration-suite
qualification is claimed by this test-only batch.
