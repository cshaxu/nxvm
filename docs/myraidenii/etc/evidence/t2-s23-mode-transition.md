# T2 S23 NEC Mode Transition

Baseline `500976cd8`, after accepted S22. V10/V13's mode-transition mechanism
is implemented at the existing CPU owner. This is not a complete 8080, timing,
Machine, board or playable-game claim. V12 remains the next ordinary-opcode
receiver; only actual opcode 00 NOP is used for transition continuation.

## Primary Contract And Baseline

The pinned original NEC October 1986 manual, SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`,
section 8 and printed 12-169/12-176/12-177 define the mechanism. All three
instruction diagrams were rendered and visually inspected, not only OCR.
The section-8 register map distinguishes native SS:SP from 8080 DS:BP.
Mode transition frames use the native stack; they must not substitute BP.

- BRKEM is 0F FF imm8. It saves native PSW/CS/following PC, loads the vector,
  clears MD and enables MD restoration. Its flag table changes MD only.
- CALLN is ED ED imm8 in emulation mode. It saves emulation PSW/CS/following
  PC on the native stack, loads the vector and sets MD. Its flag table also
  changes MD only, unlike ordinary interrupt entry's IE/BRK clearing.
- Native RETI and POP PSW may restore MD while that write enable is active.
  The same instructions outside an invocation retain native MD.
- RETEM is ED FD in emulation mode. It uses the native return frame and then
  disables MD restoration. Its table restores PSW, including MD.
- NMI/accepted INT from emulation save MD=0 and enter native mode; native
  RETI can return to emulation. Reset aborts the invocation. Nested BRKEM
  from a native callback is expressly not supported by hardware.

Printed 12-177's explanatory paragraph says BRKEM when describing write
disable, contradicting its own operation row and 12-169's explicit enable.
The operation rows, section 8 and RETI/POP PSW footnotes agree: BRKEM enables,
RETEM disables. No contradictory OCR sentence is promoted into a second rule.
Undefined nesting receives an explicit unsupported diagnostic, not a claimed
hardware exception or invented successful nested invocation.
The CALLN diagram's SP assignment prints PS-6, but its three immediately
preceding SP-relative frame writes, section 8's native-stack diagram and the
inverse RETI frame establish SP-6, not subtraction from the code segment.
The implementation reuses that existing native push owner rather than
turning the isolated typographical inconsistency into a second stack rule.

The original Intel [8080 Assembly Language Programming Manual](https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf),
printed page 16 and its opcode appendix, is also consulted read-only for the
single-byte NOP continuation. It leaves flags unchanged. No manual payload or
external implementation is imported. The complete ordinary 8080 source/form
inventory still belongs to V12, not this control-only admission.

At the baseline, native NEC FF is not admitted, MD=0 stops before executing
any instruction, and real PSW restore always retains current MD. Actual
BRKEM/CALLN/RETEM bytes therefore have no executable mode path. This feature
batch replaces that diagnostic-only blanket with the admitted transitions;
unfinished ordinary 8080 slots still stop rather than falling through to
Intel/native decoding or silently becoming NOPs.

## Ownership And Actual Change

The architectural MD value remains solely in CPU PSW. One private
`nec_md_write_enabled` lib_bool represents the separately documented hardware
write-enable, reset by the existing CPU reset. It is not a mirrored mode bit,
public API/layout expansion, worker, provider or saved App flag. Full V30
Machine/snapshot eligibility remains closed until its original receivers.

The native NEC format owner admits BRKEM's vector byte for execution and raw
native lexical scanning. Context preview selects the emulation control
encodings from actual PSW; execution uses the same private control-length
owner, within ExecIns, ExecInit/ExecFinal and the existing guest boundary.
The raw byte/profile scanner has no mode context and remains native by
contract. No public enum or mode parameter is added.

Existing `_ser_int_real` retains sole frame/vector/stack publication. Its
private option distinguishes normal IE/BRK clearing from BRKEM/CALLN's
preservation. All five old callers explicitly keep their old behavior.
Accepted real entry sets native MD only after the existing transfers succeed;
BRKEM then clears MD and enables restoration. Existing native `_e_iret` and
POPF/PSW normalization own restores. RETEM disables restoration only after
the existing return succeeds. No latch changes before a fallible frame/vector
or return operation, so the original CPU rollback/accepted-memory-prefix
contract does not require another checkpoint or compensation path.

## Proof And Similar-Issue Sweep

The new CPU-local unit reuses the unchanged actual-memory fixture and typed
transfer-plan checker. Write-enabled contexts are reached by executing real
BRKEM and, for native callbacks, real CALLN, not by setting the private latch.
The independent oracle checks complete CPU/high halves, CS/PC, PSW/MD,
native SP versus untouched BP/DS, all frame/vector transfers, byte order,
actual memory and external begin/commit/cancel phases.

The finite success matrix has 328,192 inputs:

- 131,072 BRKEM/CALLN inputs: each selector against all 256 non-BRK ordinary
  flag combinations. IE, direction and arithmetic flags are retained.
  Operand-space testing includes selectors below 32, which the interrupt
  classification reserves; these synthetic encodings are not a stronger
  hardware/board eligibility claim for reserved vectors.
- 196,608 RETEM/RETI/POP PSW inputs: each operation against every 16-bit
  incoming PSW. Native MD write-enable differs from fixed-bit normalization.
- 480 deep contexts: six operations, four PCs (including code wrap), five
  SPs (odd/wrap) and four target/segment/PSW seeds. Each verifies nonmutating
  context preview and refuses every observed data-read/write, instruction-
  fetch and observational-fetch ordinal. CPU/latch roll back, no guest
  exception is invented, and accepted frame-write prefixes are not replayed.
- Eight actual complete invocations: no interrupt, NMI, INT and CALLN, each
  with IE clear/set, followed by native RETI when applicable and RETEM.
  Exact frames and every asynchronous-entry data-transfer failure are tested;
  failed asynchronous entry preserves the already executed NOP, while failed
  CALLN rolls back its own instruction. Subsequent native POP PSW cannot
  clear MD after RETEM.
- Twenty-four physical aliases: six operations with even/odd SS stack areas
  in the code segment or vector table. Frame writes can replace subsequent
  vector bytes before entry reads them; the independent plan overlays expected
  writes on the initial memory image, rather than copying actual CPU targets.
  Return operands are likewise the actual initially prepared bytes, including
  code/frame overlap. All observed host refusals retain accepted-write prefixes.

Additional non-counted guards reject all 254 unfinished single-byte ordinary
slots and all 254 unsupported ED suffixes, including wrapped code, without
native fallthrough/data writes. Reset is checked from both emulation and a
native CALLN handler. Prohibited nested BRKEM fails without data transfers or
latch mutation. The native full-extension inventory adds only the newly
implemented FF encoding; other old expectations remain unchanged.

The sweep inspects every real entry caller, flags load/image, native IRET,
POPF, private context initialization/reset, NEC metadata/native scan, context
preview, ExecIns/ExecFinal and refresh. Other PSW callers are protected-mode,
task/SMM or fixed native images and retain their existing contracts. No
Lib/Emulator or App interpreter, MD mirror, stack/entry copy, clock or hidden
native fallback is introduced.

Actual-change review also found that context initialization assigns fields
individually rather than zeroing the whole context. Reset already cleared the
new latch, but initialization must explicitly clear it as well. One assignment
in cpu.c and a dirty-context initialization regression close that lifetime
path. This correction is retained, not hidden behind fixture zeroing; final
qualification must use the corrected frozen source/test manifests.

```text
rg -n '_ser_int_real|_e_real_flags_load_16|_e_real_flags_defined_mask|_e_eflags_load|nec_md_write_enabled' src/x86-core/cpu
rg -n 'NEC_FORMAT|emulation_control|NEC_EMULATION|preview_lexeme|ExecIns|ExecFinal|execution_reset' src/x86-core/cpu/cpu_instructions.c
```

General BRK/HLT/priority, ordinary 8080 ALU/memory/ports/stack, full
prefix/LOCK and physical retirement/timing remain original unqualified
receivers. Native MD can now change, but this does not grant V30 Machine
eligibility or pretend Intel instruction timing applies to 8080 bytes.
There is no game ROM execution or runnable MyRaidenII EXE yet.

## Qualification And Receiving Releases

Shared implementation/test P1 `ad55ccf3f` and NXVM receiving artifact/status
P2 `ad47d64f9` are pushed. The product evidence P retains the full S23 packet
before coordinator acceptance.

Corrected production first passed complete dual-width suites and receiving
releases. Exit-condition review then found that wrapped code/stack and GPR
aliases did not directly cover physical code/vector stack aliases. The
twenty-four cases above close that test gap without changing production.
Final complete suites, including those stronger assertions, pass 673/673
per width (186 X86 Core) in 94.51/103.27 seconds. The final mode unit passes
328,192 inputs plus refusals/guards in 0.862028/1.16264 seconds. No old test
budget or assertion is weakened; the finite new unit has its admitted
90-second containment. Both specialized targets retain 498 strict rows;
Types, dependency/ownership/negative, manifests, documentation and whitespace
gates pass. Existing warning exceptions are not claimed as removed.

All eight current 0564 PC EXEs are rebuilt/deployed, with 28 selected
receiving integrations and two restored-default identity checks passed.
DeskPro boot groups finish in 120.85/180.91 seconds, within the unchanged
budget. Receiving-profile loops recompile zero CPU objects. Both original
graphs are restored to default and use the existing persistent cache. Final
test-only strengthening does not change executable inputs or deployed hashes.

[Results](t2-s23-results.json) retain final suite counts and profile cases;
[ten identities](t2-s23-artifacts.json) retain fresh PE/name/width/size/hash
checks. MyNES 0044, all user INIs, firmware/media and other production
components remain unchanged. The existing receiving-PC firmware artifact
exception applies, with no raw ROM/new protected input import. All final
build/test handles have exited. Owned rendered PNGs are removed; the pinned
manual and bounded logs remain for the next CPU receiver.

Production is +105/-24/net +81 across cpu.c, cpu_instructions.c/h. Tests and
registration are +406/-1/net +405, including a 403-line new unit and the
one newly valid native extension-inventory entry. Combined code is
+511/-25/net +486, excluding manifests/records/artifacts; both manifests
together are +6/-5/net +1. This remains within the admission estimates.
The two private production helpers own mode-control lexical sizing and
execution, not forwarding; the sole new state is the documented MD write
enable. The test-local planned-word helper computes alias-dependent source
values; the original transfer/failure checker stays unchanged.

Frozen instruction/context/lifecycle source SHA-256 values are respectively
`B742B413A1FA8AD4939EC771EAF7D6477BA2185D79B33B43DEF9E087D74356D1`,
`48094F81268E18B301FE1C7D887422F9A97D4C206DB2F44EDDA77DDFE74E1816`,
`6D55A10A40805BB1A0955BFC5FD99A9C7EFAC0C0D48EFD21D9F5EA121D8A5BDA`.
No fresh complete external/desktop/RDP/audio suite, full V30 eligibility or
gameplay qualification is implied.
