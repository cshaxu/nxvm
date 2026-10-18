# T2 S25 Native Comparison Strings

Admitted from clean `105526ecb`, consuming the V09 architectural comparison
residual, not the entire V30/game domain. The full sixteen-field packet is
in Current. This record is in progress, not acceptance.

## Primary Source And Sole Path

The already pinned NEC October 1986 V20/V30 manual, SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`,
printed 12-14--15 and 12-17--18 (PDF 59, 60, 62 and 63), was rendered and inspected.
The retained source identity is verified before delivery; no external source,
protected bytes or new manual payload enters the corpus.

CMPBKB/CMPBKW compare source DS/override against fixed ES; CMPMB/CMPMW
compare AL/AX against fixed ES and leave SI alone. Both update native
V/S/Z/AC/P/CY through subtraction, with native auxiliary borrow, not 8080's
different nibble-carry rule. DIR selects forward/backward index movement.
Non-repeat performs one comparison without consuming CX. Repeat first checks
CX, so zero count preserves flags/indices; subsequent continuation uses the
new comparison's carry/zero, not the pre-instruction flags.

The existing `_a_cmps/_a_scas`, CMPS/SCAS handlers, repeat predicate, bus and
entry/return/rollback remain the sole production path. Initial audit/focused
proof has not found a production defect. No source API/state or generic test
framework is added. Existing native data-tail and prefix restart model grades
are retained; semantic tests do not qualify electrical prefetch or clocks.

## Replaced And Retained Test Responsibilities

The old modulo-memory CY/ZF-only operand matrix, shallow native maximum-count
check and separate read-failure provider are replaced by one CPU-local real
one-MiB memory/typed-plan fixture and independent direct flag/CPU oracle.
The independent oracle uses unsigned/nibble subtraction, parity and sign-bit
relations, not native ALU helpers or another emulator. Distinct old prefix
order, malformed carry-prefix, Intel FS/GS isolation and excess-prefix actual
interrupt/RETI restart tests remain. Their fixed low-memory witnesses require
no 16-bit/physical wrap and are not used to claim that boundary coverage.

The finite result matrix has 942,920 inputs: complete byte pairs in all four
forms; every word against five carry/sign/zero boundary values; all source
prefix/direction/count/termination classes, wrap/odd lanes, actual CS-data and
ES operand overlaps, maximum-CX first termination and every status-flag seed.
Deep cases refuse each observed read/fetch/observation position at every
reached iteration, comparing full CPU and exact accepted read prefix/phases.
No compare path may write memory. Preview checks byte/component counts and
permits observational reads only.

Another 800 actual memory cases cover all four forms, absent/four repeat
prefixes, five source choices, both directions, odd/even code/native stacks,
and NMI/INT. The plan checks comparison reads, real frame writes and vector
reads. Actual RETI must restore the complete comparison-result CPU, including
all arithmetic flags, then repeated forms stop at their second comparison
with CX still one. Old excess-prefix restart witnesses now additionally
assert comparison-generated saved/restored PSW.

## Initial Corrections And Pending Verification

The first strict compile found a signed integer-promotion warning in the
test's little-endian read helper. Its high-byte shift now uses lib_u32 before
conversion, rather than suppressing the warning. No production change.

An initial interrupt fixture used CS as source at FFFF and also placed the
instruction there, but still assumed the second source byte was the generated
value 2. The independent RAM oracle correctly found source byte 2E and a
continuing REPNC predicate (A6/64/2E, forward, odd, INT); the fixed two-step
termination expectation was invalid. IRQ cases now keep their data disjoint
from odd/even instruction bytes. The separate deep matrix retains actual
code/data overlap and derives its predicate from real RAM. No CPU fix or
weakened predicate/flag assertion was used to turn this fixture error green.

## Final Verification And Actual Size

Both widths pass all five focused comparison/ALU/mode/block/emulation units.
The original Release Ninja/ccache graphs build all targets and complete full
repository unit suites, 674/674 per width (187 X86 Core), in 105.33/112.57
seconds. The final comparison unit takes 2.31609/2.72066 seconds within its
unchanged 30-second budget. Its 942,920 result inputs, 800 actual IRQ/RETI
routes, every deep refusal and retained legacy controls pass. Both specialized
targets retain 498 strict rows; Types, inward dependency, ownership, negative,
manifests, documentation and whitespace gates pass. Historical CPU warning
exceptions are unchanged, not claimed as removed.

[Results](t2-s25-results.json) and [retained ten identities](t2-s25-artifacts.json)
record exact proof. All production/build/configuration inputs and all assets
are byte-identical to `105526ecb`. The ten current S24 PE/name/width/size/hash
identities match, so no product relink, manufactured artifact diff or receiving
integration rerun is required. Previous S24 integration proof is retained, not
presented as a new execution. No full external, manual desktop/RDP, physical
timing, Machine/board or gameplay qualification is claimed.

Counted test C change is +349/-131/net +218 by
`git diff --numstat 105526ecb -- test/x86-core/cpu/cpu_v30_conditional_repeat_smoke.c`.
Production and build/registration are +0/-0; test manifest separately adds
one/removes one. Records/artifacts are excluded. The increase adds complete
observable state/flags/bus/failure/interruption proof while deleting the weaker
operand matrix, duplicate maximum-count check and separate failure callback.
The slightly smaller-than-estimated diff retains distinct prefix/Intel/restart
checks rather than rewriting them. No production helper, public API, persistent
state or test framework is introduced. Raw/staged changed Shared bytes are
identical LF; the manifest hash pins
`A6E6F663A5B2568A26172E0BF31F7D2CF6E5509219B2ED99C5DA3D6436E324B2`.

## Similar-Issue Sweep And Review

Reviewed `_a_cmps/_a_scas`, CMPSB/W and SCASB/W, `_e_compare_repeat_continues`,
legacy interrupt restart, shared logical-memory/lane and rollback owners.
All four forms already use the correct native result/flags and repeat owner;
no production repair was necessary. Queries for `v30_repeat_case`,
`v30_repeat_failure_fixture`, `v30_repeat_read` and `v30_repeat_failures` in
the changed test return no obsolete shallow operand/failure path. The old
maximum-count-only loop is removed; its Intel half retains the precise name
`v30_repeat_intel_isolation`. Every retained old helper has its distinct prefix,
Intel-family or excess-prefix actual interruption responsibility.

The new oracle/plan consumes actual RAM, including overlap, rather than
assuming generated operands survive aliasing. IRQ setup deliberately avoids
code/data alias so its exact two-step stop is real; the separate deep alias
matrix still exercises that boundary. Existing assertions are strengthened,
not relaxed. Production remains frozen and no source outside the CPU-owned
test/manifest changes. Executor self-review is complete; scoped delivery and
coordinator actual-change acceptance follow before S25 closure.

Shared P1 `1bd6d334b` is pushed. The MyRaidenII evidence P retains the complete
sixteen-field active packet, source/failure chronology, final full proof and
ten unchanged current identities before separate coordinator acceptance.
