# T2 S18 Native Unpacked Decimal And Conversion

Admitted at clean `4bf9de32a` after accepted S17. The exact V07 batch is
ADJBA (37), ADJBS (3F), CVTBD (D4 0A) and CVTDB (D5 0A). This is not a claim
of complete decimal, arithmetic, native CPU or game qualification.

Shared implementation P1 `a40ee9742` is pushed; the following product-local
delivery preserves the complete packet and evidence before coordinator review.

## Source And Boundary

The original NEC V20/V30 October 1986 manual is the T1-pinned local source,
SHA-256 `fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`.
Printed pages 12-61--12-63, PDF pages 106--108, were rendered and visually
checked rather than relying on OCR's ADJ/CVT name and flag-table extraction.
No manual payload or external implementation enters the tracked corpus.

ADJBA/ADJBS adjust AL by six and AH by one independently, then mask AL's high
nibble; CY and AC reflect the adjustment condition, not an Intel 486 AX-wide
carry correction. V/S/Z/P are explicitly undefined. CVTBD divides AL by ten;
CVTDB combines ten times AH plus AL into AL and clears AH. S/Z/P follow the
result byte; V/AC/CY are explicitly undefined. All other CPU fields/flags and
unaffected upper halves must remain unchanged.

The two-byte primary diagrams specify the second byte as 0A, not an arbitrary
radix operand. This batch does not assert NEC silicon behavior for other
second bytes; existing Intel generalized-radix paths remain unchanged.

### Packed Adjust Is Not Silently Certified

The ADJ4A page actually prints the first condition as AL AND 0F < 9, opposite
to the usual decimal correction, and marks V as affected rather than undefined.
The same page's prose does not resolve this. ADJ4S has its own CY/AC ordering
questions on arbitrary non-decimal inputs. Neither uncertainty is turned into
an invented Intel contract, masked defined flag or a guessed L2 implementation.
Packed adjustment remains an explicit V07 source/semantic receiver, requiring
additional primary evidence or a separately reviewed model before acceptance.
The scope is four unambiguous forms; the original T2 universe is not reduced.

## Sole Path And Similar-Issue Review

The existing AAA/AAS/AAM/AAD handlers and common advancement/flag/operand owners
are the production path. DAA/DAS/AAA/AAS/AAM/AAD were reviewed together with
native primary admission and existing legacy ALU tests. Intel 486 AX-wide
adjustment and generalized-radix/zero-divisor cases retain their distinct Intel
semantics and regressions. No profile alias, new handler, public contract,
state, decoder, timing model or test-only production entry is introduced.

Queries:

```text
rg -n '^static void (DAA|DAS|AAA|AAS|AAM|AAD)' src/x86-core/cpu/cpu_instructions.c
rg -n 'AAM|AAD|DAA|DAS|AAA|AAS' test/x86-core/cpu
```

## Independent Proof

The one new CPU-owned unit reuses S17's CPU/probe restart and actual one-MiB
memory provider. It seeds every instruction byte, complete input AX and control
flags before execution. The oracle uses direct nibble/radix arithmetic, byte
rounding, independent parity and complete expected CPU images, not production
helpers, live decoder output or another emulator's results.

The 817,152 success vectors comprise:

- ADJBA/ADJBS: every 65,536 AX value with all four input AC/CY combinations,
  524,288 executions, including low-byte carry/borrow and AH wrap aliases.
- CVTBD/CVTDB: every AX value with zero/all arithmetic flag seeds,
  262,144 executions, including discarded input AH and byte-result wrap.
- Every form, five absent/segment-prefix contexts, all 64 arithmetic flag
  seeds, twelve boundary AX images, and ordinary/FFFF fetch starts: 30,720.

Every context in the last matrix also receives pure preview and refusal at
each applicable actual/observational fetch position. Failures must preserve
the entire unmasked pre-instruction CPU, issue no data read/write/phase,
publish no guest exception or interrupt shadow, and honor the existing host
failure result. Preview cannot mutate CPU or use non-observational accesses.
One successful execution receives one entry observation and the exact next IP.
FFFF fetch starts cross the accepted native fetch-wrap owner, not a modulo
fixture or second instruction-read path.

The initial focused harness incorrectly required no entry diagnostic on
failed fetch. Actual source places record_instruction in entry preparation,
including failure; it is not a retirement callback. The harness now requires
the existing single entry observation and separately proves unchanged full
CPU, host stop/fault, no guest exception and no data effects. This correction
changes no production semantics or assertion budget. The complete focused
x64 matrix subsequently passes in 2.16 seconds. The initial failed run is not
reported as a production defect or omitted from the chronology.

## Verification And Releases

Dual-width whole-repository units and specialized gates run in the original
Release Ninja/ccache graphs, with -j8 builds and -j4 units. Both complete builds
pass. First complete unit runs pass 668/668 per width, including 181 X86 Core
units, in 106.94/118.68 seconds; the new matrix takes 1.49427/1.80934 seconds.
[Results](t2-s18-results.json) retain wall time separately from JUnit's rounded
suite time. No complete-suite retry, deadline extension or assertion removal
was necessary after the focused harness correction.

Production +0/-0/net 0; tests/registration +163/-0/net +163 exclude manifests,
documents and artifacts: one 162-line CPU unit and one registration line.
No fixture change, production helper, API, state, execution or memory path is
added. The increase supplies independent input/result/failure proof missing
from existing native qualification; it is not a duplicate CPU implementation.

Fresh checks of all ten accepted releases' PE signatures/widths, sizes and
SHA-256 match T564's [identities](t2-s18-artifacts.json). All executable inputs,
source manifests, INIs, ROMs and media remain byte-identical to the admitted
baseline. The shared test manifest alone is refreshed. No receiving product
needs a new relink/deployment or manufactured binary commit. PC remains 0564,
MyNES 0044; MyRaidenII still has no runnable EXE.

No external-ROM integration, manual desktop/RDP, physical audio or gameplay
verification is inferred from this ROM-free test. Machine/board eligibility,
full prefix/LOCK/mode/8080, entry/bus/prefetch clocks and remaining V07 members
retain their original receivers. Lib/Emulator, other Apps, user INI/media/ROMs
and all public interfaces remain outside this batch.

Existing warning exemptions are unchanged, not claimed as eliminated.
Both specialized targets pass, including 498/498 retained strict-compilation
rows. Full suites include manifests, Types, inward dependency, ownership and
negative verifiers. Direct test manifest, documentation governance and
whitespace checks also pass. Actual-change review covers the original source
form/flag decisions, complete new oracle/matrix/failures, registration and
manifest, retained fixture consumers, baseline and release identities.
Integration suites are not rerun for this unchanged-production batch; retained
release hashes do not count as a new integration, desktop or game run.
