# M5 T359: Complete Instruction-Timing Corpus

## Task Record

T359 extends T357's deliberately finite instruction-cost corpus into a
complete, source-accounted four-profile program.  It retains one successful-
retirement elapsed-tick publisher and never conflates Intel instruction clocks
with physical bus, device, or host timing.

## Accepted Progress

### S1: Four-profile instruction-timing inventory

S1 is accepted at `a6a52001`.  Its
[inventory](../etc/evidence/t359-s1-four-profile-instruction-timing-inventory.md)
mechanically binds the 256-entry primary and 256-entry `0F` dispatch tables to
the metadata and the sole successful-retirement cost publisher.  It records
every implemented timing form as selected, a named T359 mechanism receiver,
rejected, or external; it preserves the existing one-tick unallocated form
result strictly as a transfer policy.

The task artifact is `build/output/nxvm_0_5_0359.exe`, SHA-256
`A9F0D342753C7F3BDC2C0492E08E103F3A0D8C7BB651ACE5ABAD5D75F0C39134`.
The inventory gate, retained five timing-ledger markers, documentation
governance, and 240/240 current-gate tests passed.  S2 next owns the combined
arithmetic, FLAGS, data, and ModRM/EA source matrix; physical timing remains
explicitly outside this task's form-cost owner.
