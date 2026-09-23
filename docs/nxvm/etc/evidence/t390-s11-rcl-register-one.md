# T390 S11: Operand-Size RCL Qualification

`M5:T357:S3:INSTRUCTION-TIMING-LEDGER:OK`

## Exact Row And Owner

Intel's 1986 [80386 Programmer's Reference Manual](https://datasheets.chipdb.org/Intel/x86/386/manuals/386intel.pdf), `RCL/RCR/ROL/ROR` table (printed page 372), gives `D1 /2 RCL r/m32,1` as `9/10` clocks. S11 admits only the register half of that row: the observed real-address operand-size-prefixed `66 D1 /2` register form receives nine Core ticks. The memory half remains explicitly source-unallocated and physical-contract-rejected.

The sole existing private Core 80386 source-timing owner retains prefix, opcode and ModRM classification. It accepts exactly one operand-size prefix followed by `D1 /2` with a register ModRM operand. No public interface, decoder behavior, profile selection or board-time owner changes.

## Focused Proof And Sweep

The timing-ledger smoke proves the qualified form both in the normal deterministic path and under the physical retirement contract at nine ticks. It also proves physical-contract rejection with no elapsed-time/provider publication for the native-size register form and for the operand-size memory form.

The S11 sweep used `rg -n -C 5 "case 0xd0u|case 0xd1u|SAL_REGISTER_ONE|RCL|prefixes != 0u" src/core/machine/machine.c tests/machine/core_machine_instruction_timing_ledger_smoke.c`. The 80386 `D0 /4` register SAL qualification remains its separate three-tick row. The 80286 `D0/D1` family remains in its own profile owner and outside this 80386-only S. All other 80386 rotate, count, memory, prefix and mode shapes stay nonphysical until a later corpus-backed exact-row admission.

## Bounded Replay And Transfer

After rebuilding the owner-managed redacted Model-40 capture against S11, the retained terminal advances from 18,315 to 116,637 successful retirements and then stops at one new normalized source-unallocated success. It neither selects physical retirement nor establishes board timing or L3. No external asset identity, path, hash, byte record or trace enters Git.

## Verification

The focused timing-ledger smoke and the full 283/283 current gate pass, along with diff hygiene. The S11 developer artifact is `vm-0-5-0390` / `build/output/nxvm_0_5_0390.exe`, SHA-256 `9758F3696EDFDFDDFD30D86ABA28E101766B85A31D0AE15A2B0D9878CB24AAEF`. Coordinator actual-change review and documentation-governance verification remain required before acceptance.
## Coordinator Acceptance

Coordinator actual-change review of pushed P1 confirms one private Core 80386 timing owner, an Intel-primary exact register row, and no new public or profile path. The focused regression independently passes, records the qualified nine-tick register form and preserves no-publication rejection for the native-size and memory variants. The full 283/283 current gate, documentation governance and diff hygiene passed before P1; repository and remote are synchronized. S11 is accepted. The later normalized successful terminal remains the next bounded T390 receiver; no physical profile, board-time or L3 claim is accepted.