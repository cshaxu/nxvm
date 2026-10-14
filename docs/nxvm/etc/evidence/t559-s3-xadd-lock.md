# T559 S3 XADD And Inherited LOCK

## Scope And Source

This executor delivery consumes I2 and the ordinary-opcode portion of I9
from the [S1 ledger](t559-s1-instruction-inventory.md). Coordinator acceptance
is recorded separately against the committed actual changes.
The pinned Intel 486 family programmer's reference, pages 26-282--283 and
section 3.11.6, defines source exchange followed by destination sum and the
six ADD flags. S1's EM/TS source-conflict disposition remains applicable.
CR0.WP, alignment checking and full 486 timing remain system-receiver gates;
no machine production admission changes.

## Implemented Owner And Failure Boundary

CPU metadata and the existing 0F dispatch now admit C0/C1 for the selected
486SX profile. The scanner consumes their ModRM/address bytes. The byte and
operand-sized handlers reuse existing ModRM, ADD flags, register-reference
writes and logical-memory writes. The source write precedes the destination
sum, preserving identical-register and overlapping high/low-byte semantics.
No public interface, persistent state, alternate executor or Shared change.

Memory XADD first uses the existing write-access preflight, before loading its
operand. This preserves segment-write denial before paging/data access and
reports a missing target page as a write fault. The page regression asserts
CPL3 error 6 for not-present and 7 for read-only; combined nonwritable segment
and missing page yields GP without changing CR2. The prior read-first test
expected error 4 and was insufficient despite green whole-suite results.
The correction adds no helper: `_m_test_access()` already owns this check.
The pinned Intel XADD exception contract is the primary source; Bochs 2.6
`cpu/arith32.cc` XADD and `cpu/access32.cc` RMW write-permission handling were
read-only design corroboration, not imported or transliterated source.
The inspected access file SHA-256 is
`69955FF82947BB9317807BAF49DE108DA08B3E8C606C5E7DFF4AED412D8E54B8`.
Inherited ALU read-first paths are explicitly transferred to the Core RMW
audit entry in TODO; they are not silently claimed repaired by this S.

Existing instruction exception finalization restores the entry CPU snapshot
on failure, including the flags and source changed before a failing store.
The bus-injection regression proves that behavior and successful restart.
It does not claim rollback of arbitrary external device side effects: the
bus provider remains responsible for its accepted-write boundary. A failed
provider operation is reported as the existing Core execution failure, not
invented as a guest page fault.

## LOCK Reproducer And Sweep

Before the production edit, `F0 01 16 00 01` (ADD [0100h], DX) failed the
existing CPU-owned LOCK test on x64. Ordinary ALU/XCHG cases fell into FE's
group-extension check, rejecting source register encodings above one.
Separating those cases with the existing lock flag and a break fixes the
single owner. The extended test covers ADD/OR/ADC/SBB/AND/SUB/XOR/XCHG,
byte/word and every source-register encoding. Pre-386 transparent LOCK
and the existing invalid-register regression remain.

Static sweep: `rg -n 'flagLock|PREFIX_LOCK|_GetModRM_REG'` in the CPU
instruction implementation. FE/FF, F6/F7, immediate ALU and immediate bit
groups retain their actual opcode-extension restrictions. `_d_modrm()`
remains the sole locked-register rejection. New XADD memory forms are added
to the existing extended-opcode list; CMPXCHG remains S4. The preview's
existing refusal to preview LOCK is not execution illegality and is unchanged.

## Direct Proof

- All 8x8 source/destination register combinations at byte/word/dword widths,
  in 16- and 32-bit code segments; high-byte and identical-register aliases;
  arithmetic carry/overflow/parity/sign/zero/auxiliary flags and untouched state.
- 16/32-bit direct addressing, every source register, all operand widths,
  with/without LOCK; full lexeme length and every truncated prefix rejected.
- Both new opcodes rejected on the retained 386 and locked register forms
  rejected on 486; full XADD metadata/preview admission excluded for earlier
  profiles. Early CPUs may still decode their original one-byte POP CS.
- Protected CPL0/CPL3 and VM86 successful SIB access with the exchanged source
  also supplying both address base and index; the decoded effective address
  remains stable across the register write.
- Required instruction-byte and data read/write provider failures preserve
  CPU and target memory, followed by successful retry without duplicate writes.
- Real execution of protected-mode nonwritable DS, DS/SS limit faults, CPL3
  not-present and read-only pages; valid GP/SS/PF gates, original saved flags,
  restart EIP, page error code/CR2 and unchanged source/target verified.

Initial dual-width focused tests passed. Later protection, code-segment and
expanded LOCK matrix additions passed x64 focused tests; final dual-width
whole-repository qualification and eight artifact deliveries remain pending.
An initial test draft referenced an unavailable WP constant; it was corrected
to test the existing CPL3 page-permission contract, not to add system behavior.
The first complete x64 suite passed 593/594: the newly extended negative
preview assertion incorrectly rejected the retained early-CPU POP CS decode.
It now rejects only full XADD admission, preserving that old instruction.
The SIB fixture initially used null protected-mode selectors; assigning valid
cached selectors corrected the fixture without changing production code.
Those interim results precede the write-preflight correction described above.

## Final Qualification And Actual Change Review

Against the final write-preflight implementation, complete repository units
pass 594/594 on x64 (217.25 seconds) and x86 (43.70 seconds). Logs are
`build/t559-s3-x64-unit-qualified.log` and its x86 counterpart. They include
Lib, Emulator, shared x86, Core and all five Apps, including three registered
desktop-unit cases per width. This is not manual desktop/RDP or complete
external guest boot qualification; external boot remains a T exit gate.

All eight real PC products were rebuilt in the existing Ninja/ccache graphs
and passed their product/INI identity integration once each. Both graphs were
restored to default NXVM. PE widths and the embedded 0.5.0559 string were
checked in every deployed EXE. INIs, external inputs, Shared-six and MyNES
are unchanged. No PC110/486 production profile is admitted.

Production C changes are +44/-4 (net +40), confined to CPU instructions.
Test C/CMake changes are +426/-12 (net +414), including the new 395-line
CPU test. Manifests, documents and binaries are excluded from those counts.
The extra test size covers the complete form/failure matrix rather than new
production machinery. No public API, helper framework or persistent state
was added. Review covers metadata/preview, execution dispatch, alias order,
write preflight, inherited LOCK classification, fixture ownership and all
changed assertions. The finite inherited RMW residual is recorded in TODO.

| Product artifact under `assets/` | SHA-256 |
| --- | --- |
| `my5160/my5160_0_5_0559_x64.exe` | `D547AC58BA6A374FA5155406A8E9EDC3E988C174F638F1CA5804B1093CCB4371` |
| `my5160/my5160_0_5_0559_x86.exe` | `6E6169BE7ED69FC8376281DBCE0A9CC0678FDC6FE7DA2DEFA0A55CEB6BA65E93` |
| `my5170/my5170_0_5_0559_x64.exe` | `1F4CBE8E1A54282050BED34114F148E0AB1671029A3376AB4372B3936243CAC2` |
| `my5170/my5170_0_5_0559_x86.exe` | `429AF16FD76078BCEC3F9C79D8D0A2225A887679622B7082ADA39CCB64940E26` |
| `mydeskpro386/mydeskpro386_0_5_0559_x64.exe` | `119E95AB5A69C9CC567416B33BC53D648B3A89CD9220C80EDB67F1113B039197` |
| `mydeskpro386/mydeskpro386_0_5_0559_x86.exe` | `E158E79E0A86EA7833B5119DBF50525244EF82AA0243C7A5129E50D31D4595C0` |
| `nxvm/nxvm_0_5_0559_x64.exe` | `0D05235C77D5FEDC9BA5C2E255CCF6E79E67A37E15A01078E0D1DE7107F57D64` |
| `nxvm/nxvm_0_5_0559_x86.exe` | `A8DBAB623535B6C22AB35DC660794251C65229C6B0401D3187CA518ABD64D241` |

Efficiency retrospective: finish the entire matrix and review before final
qualification; use only affected CPU targets while developing. The owner-
approved proposal now records this order and permits related simple batches
to merge at admission. No required proof is removed to shorten execution.
