# T394 S5 P1 Pre-Retirement Eligibility-Key Refresh Evidence

## Scope

This P1 repairs a Core-only observation-order regression found while replaying
the fixed T390 C0 capture corpus through the S4 eligibility key. It does not
select a descriptor entry, change a VM/Core interface, import an asset, or
make a physical, board-clock, firmware, device-timing, cycle-exact, or L3
claim.

## Finding And Repair

The diagnostic callback captures a retirement observation before an instruction
executes. S4 subsequently called that full capture operation again after source
cost classification. For instructions that change architectural state, including
the real-to-protected transition, that second call overwrote the required
pre-retirement mode and size snapshot with post-execution state.

Core now retains the diagnostic pre-retirement snapshot. After successful cost
classification it refreshes only the eligibility-key's derived context: source
timing origin/form, normalized opcode form, ModRM/control/repeat context and
post-execution control-flow result. The key continues to use the retained
pre-retirement profile, CPL, mode, size and prefix fields. The private copied
descriptor and all physical prepublication rejection remain unchanged.

## Verification

- `core-machine-retirement-observation-s3-smoke` now executes `LMSW` with PE
  set by the instruction and verifies both the published observation and its
  eligibility key retain `protected_mode = false`, while the machine has entered
  protected mode.
- `vm-model40-byob-retirement-capture --synthetic-c0-smoke` proves fieldwise
  key aggregation with two equal keys and one distinct key. It emits
  `M5:T394:S5:C0-KEY-MAPPING:OK`.
- Read-only replay using already owner-managed C0 inputs restored the accepted
  capture total: 18,255 classified successful retirements, zero unallocated
  retirements, and the protected-return-C0 terminal. It observed 81 distinct
  normalized keys without reaching the fixed capture limit.
- Focused Core timing, retirement-observation, and both deterministic Model-40
  composition tests pass. The complete current-gates build and documentation
  governance pass. The rebuilt current artifact is
  `build/output/nxvm_0_5_0394.exe`, SHA-256
  `7277DB885277BF54077B100055B2E9481C0DFA629769695AC3764CF8AEA11264`.

## Active Transfer

The accepted C0 ledger has 82 children whereas this replay currently aggregates
81 observed normalized keys. P1 does not treat that difference as a completed
same-key equivalence: S5 remains active until every child is mapped to an
observed key or the collision is proven semantically equivalent. No production
descriptor or physical Model-40 selection can follow from this P1.