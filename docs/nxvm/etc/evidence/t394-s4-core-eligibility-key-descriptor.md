# T394 S4 Core Eligibility-Key And Copied Descriptor Evidence

## Scope

S4 implements only the generic Core enforcement mechanism selected by T394 S3.
It does not select any T390 C0 member, change a VM or Model-40 interface, or
make a physical board-clock or L3 claim.

## Ownership And Lifetime

`core_machine_retirement_eligibility_key` is a value-only Core semantic identity:
CPU profile; timing origin/form; prefix-normalized opcode form; ModRM/control/
repeat context; and execution mode/size/lock/repeat qualifiers. It excludes
addresses, literal operands, firmware/assets, VM state and elapsed time.

`core_machine_retirement_qualification_descriptor` is read only synchronously
by `core_machine_create`. Core rejects null entries, zero entries and counts
above its fixed capacity, then copies entries into private machine storage. No
caller pointer is retained. Cold reset clears the current key validity but
retains the copied construction descriptor.

## Publication Boundary

After a successful instruction cost is classified, Core captures the key and
constructs an optional copied observation. In physical contract mode, Core
faults before elapsed/timeline/execution-provider publication when either the
source is unallocated or the classified key is absent from the copied
descriptor. The comparison is fieldwise, not bytewise, so struct padding cannot
change membership. The mechanism has no VM callback or model-specific branch.

## Regression Evidence

- `core-machine-instruction-timing-smoke` proves valid NOP/Jcc descriptor
  entries, reset reuse, caller-side descriptor mutation after creation, invalid
  descriptor rejection, classified-but-absent key rejection, and unallocated
  rejection. It emits `M5:T394:S4:ELIGIBILITY-KEY:OK` and
  `M5:T394:S4:PHYSICAL-ABSENT-KEY:OK`.
- `core-machine-instruction-timing-ledger-smoke` now supplies a deterministically
  captured key for each physical success and verifies direct physical paths
  with no descriptor fault before time publication.
- `core-machine-retirement-observation-s3-smoke`, both Model-40 deterministic
  composition tests, and the full source build pass.
- Current artifact: `build/output/nxvm_0_5_0394.exe`, SHA-256
  `2215B0DD8E0BF2B85E34DB705230A6ECF9A7D872A946952FD7469087A0C59CBE`.

## Transfer

The finite T390 C0-to-key descriptor mapping is still required before any
profile can select physical retirement. This S4 mechanism deliberately has no
production descriptor contents; Model-40 remains deterministic.