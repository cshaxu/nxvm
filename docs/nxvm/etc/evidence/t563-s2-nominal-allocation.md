# T563 S2 Nominal Form Allocation

## Scope And Qualification Boundary

Baseline `a6ac6377e`. S2 consumes R2 from the S1 audit and the complete C15
F01--F20 universe. This record qualifies the S2 nominal batch, not complete C16
or permission to open C17. Existing staged calculators remain disconnected
from qualified runtime publication until S6.

Original Intel 1992 PRM SHA-256:
`D86207AAEBD9B10464F2058EFC61559E6141D3B072DB100E89BFE17C8187E143`.
The external original was rehashed. PDF 815/816/819, printed E-3/E-4/E-7,
were rendered and visually inspected for MOV/stack, integer ALU and conditional
transfer clock columns. Extracted text is only a locator. T560 S9 preserves
the other table/source and numerical-model qualifications; they must be
reconciled before S2 acceptance, not silently assumed complete.

## Complete Family Census

All implementations below live at `src/x86-core/cpu/cpu_timing_model.c`;
real decoder/access facts live in `cpu_instructions.c`. Direct proof belongs
to `test/x86-core/cpu`, never an App boot.

| C15 family | Current nominal owner / remaining allocation proof |
| --- | --- |
| F01 ALU | Primary 00--3D forms and Group 1. New 864-execution matrix covers all eight operations, byte/word/dword, both code/address widths, register/memory and immediate/82/83 forms. All pass x64; original E-4 CMP memory exception preserved. |
| F02 decimal/XLAT | Nominal switch covers DAA/DAS/AAA/AAS/AAM/AAD/XLAT. Existing execution cases; malformed decimal semantic L2 is not an exact-silicon timing claim. |
| F03 unary/multiply/divide | Nominal NOT/NEG/MUL/DIV/IDIV; IMUL remains the approved L2 model. `nominal_execution`, `nominal_unsigned_multiply` and `arithmetic_model_multiply` retain source-derived width/operand/memory vectors; E-5/E-14 and PRM 26-218 provenance is retained from T560 S9. |
| F04 INC/DEC | Short and FE/FF nominal forms; existing cases, source E-4. Invalid group fields must remain event/fault forms, not successful nominal allocation. |
| F05 Group 2 | Fixed/simple rotates/shifts in nominal; variable RCL/RCR remains the approved L2 model. `arithmetic_model_rotate` retains masked/effective-zero numerical cases; the encoding census includes selected /6. Source range/formula conflict remains explicitly L2, not silently resolved. |
| F06 double shifts | Existing SHLD/SHRD nominal forms. Undefined result ranges do not authorize invented timing or a new instruction path. |
| F07 bit/scans | Existing BT/change and BSF/BSR formulas. Immediate-memory BT really has the E-9 three-clock nominal row; do not replace it with the register-index memory row. |
| F08 moves/address/exchange | Existing canonical MOV/extend/LEA/XCHG forms. Selected UMOV 0F10--13 now has an explicit L2 MOV-alias model, disjoint from original-table rows; semantic alias is not by itself a timing source. |
| F09 stack | Existing short/long PUSH/POP, all-register and ENTER/LEAVE formulas. Dynamic stack/multiple-access costs remain S3, separate from cache-hit nominal proof. |
| F10 flags/sign/NOP | Existing sign extension, flags/NOP/XCHG arms. SALC D6 now has the explicit reference-model L2 allocation below, not an original-table timing claim. |
| F11 near control | Existing Jcc/SETcc/LOOP/near CALL/JMP/RET. New 4096-execution matrix covers all sixteen Jcc/SETcc predicates, all five relevant flag combinations, both code widths and short/near/register/memory forms; taken EIP checked as well as nominal ticks. |
| F12 far/task control | Existing nominal far/task helpers use actual gate/parameter/task facts. `nominal_protected_transfers`, outer-return and task-switch tests retain E-8/E-13/E-14 columns including gate arguments, TSS widths and VM86 entry. Dynamic descriptors/stack and target fetch remain S3/S4. |
| F13 memory strings | Existing primitive/REP nominal and continuation helpers. Full REP lifetime/decode and actual dynamic accesses remain S4/S3. |
| F14 ports | Existing four-mode nominal table and REP port setup/iteration. Denied/provider-failed paths are not successful nominal retirements. |
| F15 bounds/queries/far data | Existing BOUND/ARPL/LAR/LSL/VERR/VERW/far-data rows retain E-3/E-5/E-12 values. New `nominal_protection_queries` directly checks ARPL/LAR/LSL/VERR/VERW, valid/null selector outcomes, CPL0--3, both code widths and register/memory forms. Faults remain rejected and retain the event receiver. |
| F16 descriptor/task tables | Existing group 00/01 nominal rows plus shared task helper. New `nominal_system_forms` directly checks SLDT/STR, null LLDT, available-TSS LTR, LGDT/LIDT/LMSW/CLTS through real execution at both widths. Illegal/privileged/faulting forms retain event classification. |
| F17 flags/interrupt/control | Existing PUSHF/POPF/CLI/STI/INT/INTO/IRET/HLT rows. INT1 source trap and system event accounting remains explicit S5 receiver, not guessed INT3 retirement. |
| F18 system | Existing CR/DR/TR/CLTS/cache/INVLPG/CPUID rows. RSM/SMM and power costs require S5; source-limited forms cannot be silently classified as NOP. |
| F19 late integer | Existing BSWAP/XADD/CMPXCHG nominal rows retain E-6 values; `nominal_execution` and `nominal_modes` retain register/width cases. New `nominal_exchange_memory` directly checks both CMPXCHG memory results at all three operand widths and both code widths. No semantic or timing-grade change. |
| F20 syntax/invalid/absent FPU | Existing C15 negative/length/prefix/LOCK/ESC proofs remain. Successful WAIT uses the explicit three-clock L2 upper-end model below; PRM chapter 26 prints a 1--3 range, not an unconditional exact value. Faulting ESC/WAIT/invalid forms belong to event timing, not a successful instruction subtotal. |

The source universe still includes all 256 escaped slots and all primary/group
forms from T560 S8; this family census does not replace the original legal/
illegal inventory. The reconciliation retains the original nominal tests and
T560 S9 source-conflict decisions rather than replacing their numerical proof
with the new encoding census. INT1/RSM, asynchronous faults, dynamic costs and
final selection keep their named later-S receivers; none is called complete
CPU timing by this nominal allocation.

The final code review also identified that a full encoding census is not a
numerical witness for every result branch. It adds 160 protection-query,
12 memory CMPXCHG and 26 successful system-form witnesses to the existing
nominal smoke. The existing cache smoke, not a duplicate cache fixture,
now checks INVD/WBINVD's four/five-clock nominal rows during its 64 actual
cache-operation cases. External maintenance costs remain outside that subtotal.

## Current Change And Verification

### Special-Form Model Decision

Under the owner's standing accuracy-upgrade permission, extend the existing
private L2 calculator, renaming it from arithmetic to instruction cost rather
than creating a parallel model entry. This does not alter a qualified exact
value: these three successful forms currently have no selected allocation.

- SALC: one nominal clock, L2 reference-model value. Read-only 86Box revision
  `4fef696a4eead1d55a28d6ac0e5bd2864e5454da` selects `timing_rr=1` for
  Intel SX/SL-enhanced SX in `src/cpu/cpu.c`; its `opSETALC` consumes that value.
  The reference hash for `x86_ops_misc.h` is
  `7CCA59F327985989EF2B3CA5E530DFEA2EE0D012261E6D4023BF9BF69B28E441`.
- UMOV: one nominal clock, L2 canonical-MOV alias model. Collins'
  [original UMOV description](https://www.rcollins.org/secrets/opcodes/UMOV.html)
  supports the ordinary-user MOV alias and operand encoding, not its measured
  clocks. The model deliberately assigns E-3's MOV cache-hit subtotal; actual
  memory penalties remain distinct and must be reconciled in S3.
- Successful WAIT: three nominal clocks, L2 deterministic upper-end choice
  from the original PRM 26-280 range 1--3 (PDF 778, visually inspected).
  It is not an assertion that every SX WAIT physically takes three clocks.
  PCjs' no-FPU route also consumes three, while 86Box's generic WAIT consumes
  four; these are reference models, not authority to override the original
  range. TS+MP faults still reject nominal success and use event timing.

No external code is copied or transliterated. Existing IMUL/carry-rotate
models and their original grades remain unchanged. INT1 and RSM retain their
named S5 event/system receiver; do not invent a successful opcode subtotal
that duplicates their entry/exit elapsed time. S6 must publish the explicit
L2 grade for every accepted model, not original-table qualification.

The existing private L2 calculator now owns these three models and is renamed
from `l2_arithmetic_cost` to `l2_instruction_cost`; the old name is removed.
Its existing IMUL and carry-rotate formulas are unchanged. The added matrices
reuse real instruction fixtures and existing calculators; they introduce no
test-only production entry, public type or persistent state. The later
publication changes only the PC version and executable artifacts; INIs remain
unchanged.

The special-form matrix performs 960 successful executions across real,
protected CPL0--3 and VM86 contexts, both code widths, operand/address
overrides, carry values and register/memory forms. Four real fault/provider
failure cases prove rejection without changing the caller's output. A
262,144-case primary/escaped opcode-and-suffix census checks that each
successful sampled execution has exactly one nominal/model owner. This is
not exhaustive proof over operand values or protected/system contexts;
the existing targeted mode, task, port and numerical tests remain necessary.

During fixture development, protected memory initially faulted because its
DS had not been prepared; it now uses the existing valid data segment setup.
The census initially used a fixture without port callbacks and crashed on
INS. It now uses the existing port fixture. Neither failure required changing
CPU semantics or weakening a production check.

A later numerical-test draft incorrectly placed INVD/WBINVD in the ordinary
instruction fixture, which has no external cache-maintenance provider. That
draft correctly failed and was removed; the four/five-clock assertions now
extend the existing cache-operation fixture above. The first full x64 run
observed that test failure (630 other tests passed). Its log is retained as
`build/t563-s2-x64-units-initial.log`; it is not counted as a passing full run.
Source/test changes are now frozen for a fresh complete dual-width run.

The frozen x64 full run executes all 631 units: 630 pass and the unrelated
`x86-nxvm.verifier-negative` reaches its existing 30-second timeout. Its
isolated repeat passes in 29.09 seconds with no code, budget or system change.
This is a full inventory with a passing repeat, not an uninterrupted green
run. Both changed CPU tests pass in the frozen run. The frozen x86 run passes
631/631 in 111.34 seconds, including all 165 X86 Core tests. Its complete log
is `build/t563-s2-x86-units.log`; x64's full run and isolated retry are reported
separately rather than presented as an uninterrupted pass.

Before the additional query/system/cache assertions, the nominal test passed
on x64 (3.27 seconds) and the complete X86 Core group passed 165/165 on each
width. The later full-unit results above supersede that focused snapshot.
Publication is recorded below. These results do not qualify the disconnected
final runtime selector.

Focused nominal timing test passes after both matrices on x64 (0.20 s) and
x86 (0.25 s). Both source/test manifest, dependency, Types and test-ownership
gates pass on each graph (five checks each); documentation governance and
`git diff --check` also pass. An initial standalone manifest command used a
nonexistent test-local script; it was corrected to the registered source-owned
verifier above, without changing validation semantics. This is not a fresh
full-suite or desktop qualification.
The final S2 delivery below records both widths, actual diff and the retained
later-S receivers.

## Actual-Change Review

Counted with `git diff --numstat a6ac6377e -- src/x86-core/cpu test/x86-core/cpu`:
production two files +17/-5 (net +12); tests two files +400/-13 (net +387).
Manifests, version/build declarations, evidence and EXEs are excluded from
these code counts. The positive test delta is direct numerical/mode/failure
coverage, not additional fixtures or a test framework.

The retained owner is the existing CPU timing-model file and its private
header. The arithmetic-only query name is removed at all callers; no alias
or second query path remains. Original nominal rows and IMUL/rotate models
are unchanged. Failed queries preserve output; successful model queries do
not mutate CPU/decoder/execution state. The source sweep is the F01--F20 map,
the actual opcode switch and all `80486_nominal_instruction_cost` callers,
including outer-return, TLB, task-switch, REP and cache tests. Tests use the
existing CPU fixtures at their owner; no Board/App-private dependency appears.

No public API, enum, thread, queue, mutable state or runtime selector is added.
No Lib, Emulator, Debug/IBM-PC product, MyNES or sibling source is changed.
The only X86 NXVM source change needed for delivery is the shared PC version
0563; matching root preset and PC artifact-target strings consume it.
The final eight artifacts must be verified before replacing the 0562 pair in
each App directory. User INI/media/ROM/snapshot inputs remain untouched.

## Remaining T Gates

S2 resolves the nominal ownership holes, not the complete CPU. INT1/RSM and
failed/event-only paths remain S5; memory/translation S3; fetch/decode/REP S4;
sole selection/preview/budget transport S6; production admission S7; full
C01--C17 and external qualification S8. These retain their original required
exits and source restrictions. No manual RDP, physical-machine equivalence,
PC110 boot or complete Windows GUI qualification is claimed here.

## Final Delivery Verification

Shared implementation is `4fb3d5238`. The NXVM publication commit containing
this record supplies the matching PC artifact target, evidence and eight EXEs.

| Gate | x64 | x86 |
| --- | --- | --- |
| Full repository units | 631 executed; 630 passed first run, one verifier timeout passed isolated repeat | 631/631 passed |
| Selected four-PC integrations | 16/16 passed | 16/16 passed |
| Published four-PC CLI/INI identities | 4/4 passed | 4/4 passed |
| Specialized aggregate | 113 target steps passed | 113 target steps passed |
| Direct compilation matrix within aggregate | 497 retained strict rows, zero deferred | 497 retained strict rows, zero deferred |

The integration selection is each profile's boot, INI identity and session
status, plus AT/DeskPro CMOS and Console lifecycle receivers. It is not a
fresh complete external-media or MyNES integration qualification. The full
unit inventory includes Shared, Core-family, all four PC Apps and MyNES;
desktop-labelled automated rows are distinct from unperformed manual/RDP work.
Existing warning-policy exceptions outside this diff are not removed by the
direct-compilation result.

Both original Release Ninja/ccache graphs are restored to default after the
four-profile runs. Eight 0563 EXEs are deployed, with final hashes in
[artifact evidence](t563-s2-artifacts.json). Each App retains exactly two EXEs.
The final default rebuild during specialized checks refreshed its build stamp;
the recorded hashes were therefore rechecked after those checks and published
CLI tests, not taken from the earlier intermediate deployment. Old 0562 EXEs
are removed only after new identities passed and remain recoverable from Git.
MyNES and all tracked owner INIs are unchanged. Manifest, dependency, Types,
documentation checks and `git diff --check` pass.

The executor actual-change review finds no new public boundary, second timing
owner or unrecorded source-grade change. The full S2 cohort and its retained
receivers are explicit above; this delivery does not open C17 or close T563.
