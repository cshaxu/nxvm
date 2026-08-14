# T363 S1: complete instruction-timing inventory

## Mechanical completeness boundary

The implemented instruction space is exactly the 256 explicit primary-table
assignments and 256 explicit `0F` assignments in
`src/core/machine/cpu_instructions.c`.  T359's retained verifier mechanically
counts both tables, binds their profile/rejection metadata to
`core_machine_cpu_instruction_metadata_get()`, and checks that the one
successful-retirement publisher is `core_machine_instruction_cost()`.

T363 consumes that mechanical boundary and does not invent a second decoder or
timing publisher.  Its publisher order is: string/I-O, 80386 dynamic IMUL,
T362 legacy dynamic arithmetic, 80386 secondary, 80386 privileged, primary
source, control/stack source, then the four profile-local legacy ledgers and
the visible unallocated receiver.  A non-successful instruction never reaches
that publisher.

## Complete form disposition

The following partitions cover every dispatchable slot; ranges include their
documented ModR/M extension restrictions. `allocated` means an existing
T357--T362 source row remains authoritative, never that T363 has inferred a
new clock. `receiver` is an explicit later shared-mechanism owner. `rejected`
has no successful-retirement timing row. `external` is a non-CPU numerical
owner.

| Space/form partition | Profiles and variants | Current disposition and owner | T363 receiver |
| --- | --- | --- | --- |
| Primary `00`--`5F`, `80`--`A9`, `B0`--`D7`, `E0`--`FF`, including group extensions | 8086 baseline; 80186 additions `60`--`62`, `68`--`6F`, `C0/C1`, `C8/C9`; metadata rejects `82`, `D6`, invalid groups | Existing arithmetic/data, control/stack, string/I-O, legacy-dynamic, or profile ledger owner where T357--T362 selected it; all other successful forms are visible receivers. | S2 arithmetic/FLAGS/data/ModR-M/EA, S3 control/stack, or S4 string/I-O by form. |
| Primary prefixes `26/2E/36/3E`, `64/65`, `66/67`, `F0`, `F2/F3` | Segment/repeat on 8086; FS/GS and size prefixes only 80386; LOCK legality is form/state dependent | Prefix has no inherited clock. Existing legal selected combinations stay at their owner; invalid or below-profile forms reject. | S4 owns repeat/segment-string/I-O state; S5 owns legal 80386 width/prefix combinations; unsupported combinations retain receiver. |
| Primary `D8`--`DF` | Optional 8087 escape, otherwise FPU/profile rejection | CPU interface exists but numerical x87 execution is external. | Explicit x87/physical receiver; never instruction-clock allocation. |
| Secondary `0F 00`, `01`, `02`, `03`, `06`, `20`--`26`, `A0/A1/A8/A9`, `B2/B4/B5` | 80286 system forms and 80386 privileged/table/descriptor forms; metadata limits group extensions and profile | Existing selected 80386 successful rows retain S6 owner. 80286 system descriptors and `0F 03 LSL` descriptor-granularity context remain unallocated by T360. | S6 validates source/context capture; unprovable descriptor or delivery state remains explicit transfer. |
| Secondary `0F 80`--`9F`, `A3`--`A5`, `AB`--`AD`, `AF`, `B3/B6/B7`, `BB`--`BF`, `BA /4`--`/7` | 80386 successful secondary integer, legal prefixes/ModR-M only | Existing selected near-Jcc, SETcc, bit, double-shift, IMUL, scan and MOVX rows retain their source owner. | S5 audits source/form/prefix/width matrix; omitted legal context stays receiver. |
| All remaining secondary slots | All profiles | Metadata invalid, below profile, or `UndefinedOpcode`; no successful retirement. | Rejected; no clock row. |

## Source ladder and retained transfers

1. Exact Intel rows already accepted by T357--T360 remain first authority.
2. T362's four immediate-IMUL rows are the only Intel-range-constrained
   same-profile model allocations in this corpus; their private table and
   evidence remain authoritative.
3. No new external scalar is admitted by S1.  If a later S encounters a
   primary range/no-value row, it must use the owner-approved Intel-first,
   same-profile MAME/86Box/PCjs ladder and document accounting equivalence.
4. Bus waits, HOLD/DMA, prefetch/cache, device service, fault/interrupt
   delivery, x87 numerical work, host time, and cycle/pin phase stay with
   their named Queue receivers.

The exact transferred CPU rows at admission are the 80286 `63`, `0F 00 /0`--
`/5`, `0F 01 /0`--`/6`, `0F 02`, `0F 03`, `0F 06` context forms and 80386
`0F 03 LSL` byte/page descriptor distinction.  S6 may allocate only a form
whose context is capturable before retirement; otherwise it preserves the
transfer.  No S may borrow an 80386 value for 80286 or manufacture delivery
cycles.

## Ordered S-unit admission

| Unit | Shared mechanism boundary |
| --- | --- |
| S2 | Primary ALU, FLAGS, data, ModR/M, EA, Group forms, TEST/XCHG, conversion and adjustment. |
| S3 | Calls, returns, interrupts, LOOP/Jcc breadth, stacks, HLT, and real-mode control success. |
| S4 | String primitive/restart/repeat/count/direction and ordinary I/O state. |
| S5 | 80386 secondary integer plus legal `66`/`67`/prefix and operand-address capture. |
| S6 | 80286/80386 privileged/table/selector/control/debug/task/VM86 source-context reconciliation. |
| S7 | Complete allocated-versus-transferred closure audit. |

## Verification record

`verify-t359-instruction-timing-inventory` passes against this unchanged
mechanical source boundary: 256 primary assignments, 256 secondary assignments,
metadata classification, seven private timing owners, one publisher, and the
visible unallocated receiver.  The S1 sweep is:

```text
rg -n "insTable\[0x|insTable_0f\[0x|core_machine_cpu_instruction_metadata_get|CORE_MACHINE_SOURCE_UNALLOCATED_TICKS|core_machine_instruction_cost\(" src/core/machine/cpu_instructions.c src/core/machine/machine.c cmake CMakeLists.txt docs
```

S1 makes no runtime, ABI, source import, or developer-artifact change.  It is
the admission record for later T363 mechanism units, not a physical or
cycle-exact claim.
