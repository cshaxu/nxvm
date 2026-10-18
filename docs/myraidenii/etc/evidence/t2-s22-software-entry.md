# T2 S22 Native Software Entry And Return

Baseline `9bcd1348d`, after accepted S21. This consumes V08/V11's ordinary
native software frame/vector/return spine: BRK3, BRK imm8, conditional BRKV,
CHKIND's BRK5 continuation and RETI. It is the existing owner that later
BRKEM/CALLN/RETEM must reuse, not a second mode entry or a claim that 8080
execution has been implemented. Full T2 and the playable game remain open.

## Primary Source And Reproduction

The pinned NEC October 1986 V20/V30 manual remains primary, SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`.
Section 3 defines PC as the following instruction. Section 6 classifies
CHKIND with the software interrupts; printed 12-167--12-170 define the
software frame, vector, IE/BRK clearing and ordinary-native RETI. Rendered
PDF 212/213/215 and section 6 were inspected, not only OCR. The BRK imm8
diagram contains its opcode and selector bytes even though its Bytes row
prints 1; the two-byte encoding, not that inconsistent row, owns decoding.

The manual's positive CHKIND example uses index 87 against bounds 2..80 to
enter BRK5. The new first witness independently checks that outcome and
requires return to the following PC. Before repair, actual execution enters
9000:0200 but stores 0100, not 0104, on the stack. The before log is
`build/myraidenii/t2-s22-before-x64.log`; it fails before the wider matrix.
This witness uses unambiguous positive values and does not depend on the
separate signed-comparison source question.

## Repair And Similar-Issue Sweep

Common ExecFinal previously special-cased native divide's following-PC
software entry but restored CHKIND from the Intel restart-PC checkpoint.
The same existing selection now includes native BR as well as native DE;
the pre-186 Intel divide rule and later Intel fault restart remain unchanged.
There is no new exception dispatcher, native frame/vector builder, return
helper, state, callback, interface, emulator loop or guest-PC special case.
Production is +5/-4/net +1 in the existing instruction unit.

The sweep covers all _SetExcept_BR callers, BOUND/CHKIND admission and
operand read, _e_int3/_e_int_n/_e_into, _ser_int_real, native IRET, ExecFinal,
the S19 divide receiver and the existing Intel exception tests. Explicit
software entry already saves next PC through its original path; ordinary
return already normalizes native PSW/MD correctly. Only the common BR
selection needed repair. Unsupported forms still stop diagnostically;
there is no fabricated native INT6 or added retry/replay.

```text
rg -n '_SetExcept_BR|EXCEPT_BR|next_eip|fault_cpu' src/x86-core/cpu/cpu_instructions.c
rg -n '_e_int3|_e_int_n|_e_into|_ser_int_real|IRET|BOUND_R16' src/x86-core/cpu/cpu_instructions.c
```

## Finite Proof And Explicit Exclusions

The new CPU-owned actual-memory unit has 655,265 successful input cases,
including expected guest entry:

- One original positive bounds/next-PC witness.
- 65,536 immediate BRK cases: every selector against all 256 non-BRK native
  flag combinations. The existing immediate decoder is exercised even for
  selector 3; canonical one-byte BRK3 is separately checked, not aliased to
  a new entry. BRK3 and BRKV each have 256 flag cases; BRKV covers both paths.
- 262,144 bounds cases: every word index against four boundary pairs, using
  the existing signed comparator model without changing or promoting it.
- 131,072 RETI cases: every PSW stack word against two opposing initial flag
  states, including native MD=0 requests, reserved bits and restored BRK.
- 196,000 form/context cases: five operations, absent/four segment overrides,
  four code positions, five odd/even/wrapped stacks, five input/return seeds,
  two flag states and every legal memory ModRM for CHKIND. Index-SP and
  effective-address register aliases, CS/data overlap and old/new return
  segments are consumed through actual fixture memory.

The independent typed plan checks source words, PSW/CS/next-PC writes,
vector reads, order/width/physical addresses/phases and final accepted bytes.
Full CPU/flags, SP/high halves, CS/PC and shadow are checked. Every taken
context executes the actual handler's RETI and must restore the complete
original CPU except following PC. Preview is nonmutating. Each observed
data-read/write, code-fetch and observational-fetch ordinal is refused in
turn: CPU rollback/no guest delivery/shadow and the exact accepted write
prefix are required. The existing fixture/plan is reused unchanged.

This does not qualify undocumented signedness as a silicon fact. The primary
CHKIND page gives a range comparison but does not explicitly settle signed
versus unsigned negative-limit ordering. The systematic word sweep uses
nonnegative bounds, where negative-bit indices reject under either ordering;
code/data-overlap contexts can alter bounds and use the existing signed model
for their expected predicate. The comparator is unchanged and remains
non-eligible for a stronger source claim at V03/V11 until primary evidence or
an approved model disposition. No comparison algorithm is changed to fix an
unambiguous frame PC.

Initial BRK is clear. RETI restores BRK under the existing previous-BRK
sampling model; first-trap boundary/general priority is not retired. Native
MD stays set outside emulation calls according to the RETI footnote. The
different MD-write-enable and mode return of BRKEM/CALLN/RETEM remain V10/V13
receivers, as do 8080 slots, full prefix/LOCK, prefetch/clocks, Machine/board
and game. S8's data-tail L2 grade is unchanged. Entry diagnostics are not
asserted to be physical retirement or timing qualification.

## Qualification And Receiving Releases

Shared repair/test P1 `470391cf0` and NXVM artifact/status P2 `2f227277c`
are pushed. The MyRaidenII evidence P retains the complete admission packet
before coordinator acceptance.

Focused x64 passes all 655,265 inputs plus host refusals in 8.23 seconds after
the central repair. No fixture correction or weakened assertion was needed
to turn the before witness into a pass. The finite new unit has a pre-admitted
90-second containment budget; no old test budget is changed.

Both original Release Ninja/ccache graphs build all targets and first complete
unit suites pass 672/672 (185 X86 Core) in 323.40/136.98 seconds. The new matrix
takes 8.16742/9.66592 seconds. Both specialized targets retain all 498 strict
rows; source/test manifests, Types, dependencies, ownership/negative,
documentation and whitespace checks pass. Existing warning exceptions are
not claimed as removed. The frozen instruction unit SHA-256 is
`8FE6E3184D29C8A1CA939722A6649015B42EECFD99B4335E4AE1FCB00A8AF716`.

All eight current 0564 PC EXEs are rebuilt/deployed through the original
graphs. Each width passes fourteen selected receiving integrations across
four profiles, then one separate restored-default build/identity check.
Both default graphs are restored, and receiving-profile loops recompile zero
CPU objects. These 28 receiving and two post-restoration checks are not a
fresh complete external-integration-suite claim.

[Results](t2-s22-results.json) retain full counts/profile cases and
[artifact identities](t2-s22-artifacts.json) retain fresh all-ten PE/name/
width/size/SHA checks. MyNES 0044, all user INIs, firmware/media and other
source components remain unchanged. All build/test handles have exited.
Source +5/-4/net +1 and tests/registration +258/-0/net +258 give combined
+263/-4/net +259, excluding records/artifacts/manifests. The source/test
manifests together are +3/-2/net +1. Existing fixture/plan reuse avoids the
initially estimated 350--600 test lines; the new unit is 256 lines plus two
registration lines. There is no production helper/interface/state addition.
Receiving NXVM source policy's existing owner-approved embedded-firmware
artifact exception applies; no raw ROM/new protected input is imported.
No physical desktop/RDP, audio, full guest GUI, fresh complete external
integration suite, complete V30 or gameplay qualification is implied.
