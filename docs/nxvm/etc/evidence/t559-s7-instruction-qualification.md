# T559 S7 Whole Instruction-Delta Qualification

## Baseline And Scope

Implementation baseline is `e590a0dde`; admission is `51584e161`.
This is the executor delivery; coordinator acceptance is recorded against
the committed actual diff. The
[finite ledger](t559-s1-instruction-inventory.md) defines the coverage universe.
Shared six trees are unchanged. The batch repairs one confirmed CPU decoder
bypass, described below.

## Whole-Ledger Reconciliation

| Rows | Direct evidence and current disposition |
| --- | --- |
| I1 | S2 `cpu_bswap_smoke.c`: eight registers, prefixes, undefined word-form policy, old profiles and fetch rollback; identity/decode evidence remains applicable. |
| I2 | S3 XADD owner test: widths, aliases, memory/address forms, flags, LOCK and failure boundaries; unchanged implementation. |
| I3 | S4 CMPXCHG owner test: both comparison outcomes, aliases, write preflight/writeback and restart; unchanged implementation. |
| I4--I6 | S5 retained-transfer and negative tests prove only their recorded boundary. Cache/TLB instructions and test-register commands still require the complete system receiver; current UD/passive behavior is not accepted 486 semantics. |
| I7--I8 | S6 CPUID/flags and extended IRET/task tests prove the named candidate contract, not exact PC110 silicon, VME execution or alignment exceptions. |
| I9 | Expanded existing LOCK test covers retained 386 and candidate 486 primary ALU/XCHG and group forms. Both widths pass after the single decoder repair; no new handler or test framework. |
| I10 | Existing profile gate, decode, FPU and timing tests pass. Exact comparison review below preserves distinct earlier-family contracts. All eight final external groups pass. |
| I11 | SMM/RSM, CR0/CR3/CR4, descriptor accessed writes, WP and AC remain mandatory system work, not silently omitted. |

## Test Gap And Batch Repair

Authority remains Intel's 1992 i486 Programmer's Reference Manual section
13.1, pinned with edition/hash in the [T558 source record](t558-s2-source-availability.md).
LOCK permits the listed memory-destination operations, not register targets;
this correction does not invent a new instruction or earlier-family policy.

The prior legacy LOCK positive matrix covered byte/word register-source forms
only on 386. The whole sweep adds dword and 486, all eight source encodings,
and the immediate ALU, INC/DEC, NOT/NEG and bit-operation groups. Each group
crosses ModRM extension, memory/register destination and applicable widths.
Illegal forms must deliver UD. Legal forms compare full CPU state and the
memory result against the same ordinary operation, replacing LOCK with the
default DS override to preserve instruction length. Primary ALU/XCHG also
retain explicit arithmetic-result expectations; equivalence does not replace
the independent arithmetic semantics tests.

The first run failed on register-destination immediate BTS. The diagnostic
batch then collected all 108 failing combinations: immediate BTS/BTR/BTC and
register-index BTS/BTR/BTC, word/dword on both 386 and 486. All other tested
group forms passed. `_d_bit_rmimm` called `_kdf_modrm` directly, bypassing the
existing `_d_modrm` locked-register rejection. Replacing that one call with
`_d_modrm` restores the existing single check before immediate/operand access.
No duplicated LOCK validation or new state is needed. Both focused host-width
tests, complete repository units and final external qualification pass.

This stays in `test/core/chips/cpu/cpu_legacy_lock_smoke.c`, reusing the existing
CPU fixture. XADD and CMPXCHG retain their separate owner tests because their
alias, flags and write-failure contracts are distinct. No per-opcode executable,
public interface, persistent state or duplicate interpreter is added.

## Profile And Ownership Sweep

Searches cover `PROFILE_80386` equality/bounds in `cpu.c`,
`cpu_instructions.c`, `cpu_timing.c`, `cpu_timing_model.c` and
`src/core/x86/machine.c`, together with `PREFIX_LOCK` execution admission.

- The Core machine constructor still rejects profiles above 386. Direct CPU
  tests selecting 486 do not make a board runnable with it.
- The optional `cpu_80386_cr_mov_ignores_mod` configuration remains 386-only;
  this is a named retained-silicon policy, not generic 486 instruction admission.
- Reset-vector provenance has distinct 286/386 paths. Complete 486 reset
  provenance remains with the system receiver; do not widen the 286 bus width.
- The scanner explicitly permits the candidate 486. Preview rejecting LOCK
  remains separate from execution legality; unavailable preview is not UD.
- 287/387 eligibility remains explicit. SX does not acquire an integrated FPU
  through enum ordering.
- LSL's 386 comparison collects timing observations, not descriptor-limit
  results. It stays 386-specific.
- Timing dispatch retains its source-qualified profile checks, including the
  exact-386 fallback. No inherited nominal cost is presented as qualified
  486 timing. The system receiver must settle tables, penalties, reset and
  maximum scheduling budgets before production admission.
- The previously recorded inherited RMW read-first defect class remains in
  NXVM TODO; this LOCK coverage does not repair or conceal that separate class.

The similar-issue query `_kdf_modrm(context` now finds only `_d_modrm` itself,
segment-register and effective-address decoders, and debug/test-register
decoders. Their instruction families are excluded by the existing LOCK opcode
whitelist, or deliberately only form an address; none is another admitted
locked bit-operation bypass. They are retained with their distinct contract.

## Final Verification

The original two Ninja/ccache graphs run one sequential external integration
qualification per profile/width. Passed groups are not restarted. Existing
aggregate registration also includes unchanged MyNES integration tests;
that does not authorize MyNES source or artifact changes.

Before the decoder correction, all eight external groups passed: 15, 17, 17
and 35 cases for XT, AT, DeskPro and default respectively on each width.
These are pre-repair baseline results, not final-source qualification. The
final dual-width repository units pass 597/597 each: x64 in 222.88 seconds,
x86 in 43.67 seconds. Logs are `build/t559-s7-x64-unit.log` and
`build/t559-s7-x86-unit.log`. They include Lib, Emulator, shared x86, Core and
all five Apps, with three desktop-labelled unit cases per width. These are
automated tests, not manual RDP validation.

Final publication and external requalification pass sequentially under
`build/t559-s7-final-<architecture>-<profile>-integration.log`:

| Profile | Cases per width | x64 seconds | x86 seconds |
| --- | --- | --- | --- |
| IBM 5160 | 15 | 33.34 | 35.63 |
| IBM 5170 | 17 | 40.61 | 53.54 |
| DeskPro Model 40 | 17 | 128.81 | 166.75 |
| Default NXVM | 35 | 50.92 | 63.93 |

Each aggregate includes the same twelve MyNES integration cases; these are
repeated aggregate executions, not distinct new coverage. Every final group
passes once. Existing boot predicates and budgets remain unchanged. The real
product INI identity route executes each rebuilt product. All eight deployed
EXEs pass PE width and embedded `0.5.0559` checks. Both original Ninja graphs
finish on default NXVM. No INI, master media, ROM, Shared-six or MyNES file is
changed. No manual desktop/RDP claim accompanies the automated desktop-labelled
tests.

Current actual C changes are production +1/-1 (net zero), test +107/-16
(net +91), measured with `git diff --numstat` on the two C paths. Extra test
code covers the previously absent whole-form matrix; production retains one
LOCK admission and one generic register-destination check. Core source/test
manifests and documentation governance pass at this stage.

## Final Artifact Identity

The implementation commit containing this record identifies the source and
eight published artifacts. Paths below are relative to `assets/`.

| Artifact | SHA-256 |
| --- | --- |
| `my5160/my5160_0_5_0559_x64.exe` | `57BA193119D5EA136DE2497D8334EAA5591EEFD89AED8F345D44C8B39507C1D5` |
| `my5160/my5160_0_5_0559_x86.exe` | `E5F7DCA2734901F6FB13959922A690055DBCBCF7EDB08C9870629C5FC1964498` |
| `my5170/my5170_0_5_0559_x64.exe` | `0EB3237426E21D24ECF2DB6704835A6DDCE199E7D1CE270CA5D8B2835146FC12` |
| `my5170/my5170_0_5_0559_x86.exe` | `61F4E02A026E269C5D4947388F8B9C805391F6F806C46DAF9EFAAA3067A8AD33` |
| `mydeskpro386/mydeskpro386_0_5_0559_x64.exe` | `04FF324408088501F6F315F2DD9EB8C67E2E83CC08E397DF8CDB2A99CE11F015` |
| `mydeskpro386/mydeskpro386_0_5_0559_x86.exe` | `0CE00963381E81C2E21276B92A889CE71BCA2FE15432DA32C6FC64AD40BB9D02` |
| `nxvm/nxvm_0_5_0559_x64.exe` | `97368C168629CD50705CA97C7B36147DE61BC322AE89E7C88D95B97600F02E65` |
| `nxvm/nxvm_0_5_0559_x86.exe` | `46F27FE2C546F15D19D98302510E5AAF582D60207C292C505B71F6F530D962A5` |

## Review And Remaining Boundary

Actual-change review covers both C files, the two manifest hash/revision
updates, evidence/status/history and all artifact identities. The new matrix
exercises 384 primary ALU/XCHG source combinations and 672 grouped forms
inside one existing test. The production change restores the checked decoder
instead of duplicating its rejection. Existing arithmetic, memory-bit and
failure tests remain in the full unit run. The earlier successful integrations
were not reused as final evidence after changing production code.

I4--I6 and I11 remain incomplete real 486 semantics with their complete
[system receiver](../../history/M5-T560-complete-486sx-cpu-proposal.md). AC/ID storage and
candidate CPUID encoding do not prove AC/VME effects or exact selected silicon.
The board admission gate remains closed. This delivery completes S7's bounded
instruction qualification, not a runnable 486 or PC110, and does not declare
the persistent 486 implementation objective achieved.
