# T390 S9: Far-Indirection JMP Qualification

`M5:T357:S3:INSTRUCTION-TIMING-LEDGER:OK`

## Exact Rows And Owner

Intel's 1986 [80386 Programmer's Reference Manual](https://datasheets.chipdb.org/Intel/x86/386/manuals/386intel.pdf), `JMP` instruction table (page 318), gives `FF /5 JMP m16:16` as `43+m` in real address mode and `pm=31+m` for the successful protected-mode intersegment route. Its clocks definition (page 248) defines `m` as the component count of the next executed instruction: prefixes and ordinary bytes each count separately, while a complete displacement or immediate counts once.

The sole existing Core control/stack timing owner already obtains that term from the nonpublishing lexical-preview mechanism after successful execution. S9 corrects its private 80386 real-address memory-far-JMP ledger entry from the contradicted historical `17` base to `43`, and admits only the successful protected same-privilege direct-code `FF /5` path at the documented `31+m` row. The protection discriminator retains the pointer selector captured by the instruction and requires it to equal the resulting CS selector. Therefore call-gate, task, privilege-changing, failed and other unproven protected transfers remain source-unallocated and physical-contract-rejected.

## Focused Proof And Sweep

The existing timing-ledger smoke now executes a CS-overridden real-address `FF /5` indirect jump through a project-owned pointer to a known one-component target. It proves physical admission at `43+1 = 44` clocks. Its protected counterpart remains prepublication-faulted with zero executed instructions, elapsed ticks and execution-provider publication. The sweep confirms that the shared `FF /3` far-CALL path remains unmodified and nonphysical in protected mode, that direct far-JMP retains its existing separate row, and that segment prefixes still enter the same source-backed control/stack owner.

## Bounded Replay And Transfer

A rebuilt owner-managed, redacted Model-40 boot-media replay classified the previously terminal successful protected indirect jump at the source-backed `31+m` route (45 clocks in that observed context). It advances from 18,228 to 18,315 successful retirements and then stops at one new normalized source-unallocated terminal. It neither reaches `0:7C00` nor selects physical retirement. No external asset identity, path, hash, provenance, byte record or trace enters Git.

## Verification

Focused timing-ledger smoke, full 283/283 current gates, documentation governance and diff hygiene pass. The S9 developer artifact is `vm-0-5-0390` / `build/output/nxvm_0_5_0390.exe`, SHA-256 `F2946939290ECFE8CA35106FF3299FD7F110AF9798558B952AEA37901E0F2926`. Coordinator actual-change review remains required before acceptance.