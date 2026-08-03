# M5 T92 S1: NXVM Console Context

## Result

The Console's mutable command state is caller-owned. The retained prompt and
grammar are unchanged; no VDM product exists to migrate.

## Verification

- `nxvm_0_5_0092.exe` consumed `help` then `exit` on standard input and
  emitted the retained banner, prompt, and `NXVM Console Commands` heading.
- SHA-256: `83F76ED7B8076EB13FA21E73ED51F9D09C8E72E90A1FB48B85452E4773D6FDB9`.
