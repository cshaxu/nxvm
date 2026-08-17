# T394 S3: Normalized Eligibility-Key Collision Audit

`M5:T394:S3:NORMALIZED-KEY-COLLISION-AUDIT:OK`

## Decision

A physical-retirement descriptor must compare a complete, finite
`core_machine_retirement_eligibility_key`, not an observation, instruction
byte window, PC, firmware identity, external callback or VM state. The key is
Core-owned, value-only and equal only when every field below is equal:

| Dimension | Normalized value | Existing Core producer | Why it is required |
| --- | --- | --- | --- |
| CPU and timing family | CPU profile, timing origin, opaque source-form ID | `core_machine_instruction_cost` and its selected private classifier | Keeps a 80386 source path distinct from a compatibility fallback or a different selected source row without using emitted ticks. |
| Instruction identity | prefix-normalized primary opcode; secondary opcode or a fixed no-escape sentinel | `t_cpuins_data.opcodes` after the existing prefix scan | Separates the semantic instruction from immediate, displacement, port and branch-target bytes. |
| Group and operand shape | ModR/M register-or-memory form; extension or fixed unavailable sentinel | existing private ModR/M extraction | Separates group operations, indirect control forms, segment/control selectors and primary memory/register rows. |
| Dynamic semantic context | control outcome; next-lexeme component count; REP phase | existing observation helpers plus `source_timing_repeat_phase` | Separates conditional/indirect control outcomes and primitive, zero-count, first and continuation string work without recording a target or raw next instruction. |
| Architectural context | CPL, protected mode, virtual-8086 mode, operand size, address size, LOCK and REP kind | CPU/decode state already captured for observation | Prevents a real-mode member from collapsing with a protected/VM86 or prefix-distinct semantic form. |

The key deliberately excludes `point.bytes`, CS/EIP/linear PC, source tick
count, immediate/displacement/port/branch values, ROM/media/asset identity,
observer sequence and elapsed/timeline time. Those values are dynamic inputs,
locations or results; accepting them would make eligibility depend on one
firmware replay rather than the C0 semantic form.

## C0 Collision Reconciliation

T390 S27 retains the complete 82-child C0 batch. Its five owner groups map to
the key as follows; every group consumes the listed dimensions, so no group is
collapsed merely by origin, opaque form or emitted ticks.

| C0 group | Children | Required key dimensions | Collision result |
| --- | --- | --- | --- |
| Control and stack | 1, 13, 23--24, 28, 40, 44, 46--47, 67, 72, 82 | instruction identity, group/shape, control outcome, next-lexeme components, architectural context, timing family | The corrected LOOP split and direct/indirect JMP forms require outcome and `m`; neither PC nor target is needed. |
| Primary data/ALU | 2, 4, 7, 11, 14--19, 21, 27, 30--33, 35--39, 41--43, 45, 48--49, 54, 58--66, 70, 73, 75--76, 80 | instruction identity, group/shape, architectural context, timing family | Opcode, escape and ModR/M/extension distinguish generic primary/group rows; source form/origin prevents classification-family collapse. |
| String and ordinary I/O | 3, 10, 20, 22, 34, 52--53, 77 | instruction identity, REP kind/phase, architectural context, timing family | STOS primitive/continuation and I/O immediate-versus-DX forms remain distinct without retaining port or count values. |
| Privileged | 8, 68--69, 71, 79, 81 | instruction identity, group/shape, architectural context, timing family | `0F 01`, `0F 20` and `0F 22` require escape opcode and ModR/M selector/form; mode distinguishes their protected-mode uses. |
| 80386 fallback | 5--6, 9, 12, 25--26, 29, 50--51, 55--57, 74, 78 | instruction identity, group/shape, control context, architectural context, timing family | Branch variants and SAL/MOV Sreg forms require the same normalized dimensions; literal prefix or immediate bytes are not a selector. |

This is a schema-level collision audit, not yet a descriptor membership proof:
existing C0 evidence did not record the resulting normalized key values. The
next implementation S must emit the key from Core, replay the fixed C0 corpus,
and prove each of the 82 children maps to an explicit descriptor entry or a
recorded same-key semantic equivalent. Duplicate keys are permitted only after
that proof; descriptor cardinality is therefore not assumed to equal 82.

## Producer And Boundary Inventory

`core_machine_instruction_cost` clears and selects the source timing family
before retirement publication. `retirement_observation_interface.c` already
performs the prefix scan, opcode/escape interpretation, supported ModR/M
extraction, control outcome and next-lexeme capture; it also copies mode,
size, LOCK and REP state. These are all Core-owned inputs. Today primary and
secondary opcode are reconstructed only from the copied diagnostic point, so
the next receiver must move that normalization into a private Core key
producer rather than making raw diagnostic bytes contractual.

The future descriptor must be validated and copied synchronously during
`core_machine_create`; it may not retain an entry pointer, provider callback,
VM object, ROM pointer or firmware identifier. Reset clears only per-retirement
pending/key state. It must not alter the copied descriptor; an unallocated
source or a classified key absent from the descriptor faults before elapsed or
device time publication when physical mode is selected. Deterministic mode
continues to publish exactly as it does today. Both Model-40 construction
routes remain explicitly deterministic.

## Similar-Issue Sweep And Transfer

The source sweep covered all `source_timing_*` writes and resets, the complete
classifier dispatch, the retirement observation/context producer, the
prepublication physical rejection, and both Model-40 construction routes. No
VM-to-Core firmware coupling or retained external lifetime was found. The
current observation object is diagnostic-only and cannot be reused as the
eligibility contract because it carries the raw execution point.

The next receiver is a bounded Core implementation S: define the value-only
private key and public immutable descriptor shape, validate/copy it at
construction, emit the normalized key to a test-only/copied observation path,
and add synthetic equality/reset/absent-key nonpublication regressions. It
must then run the fixed C0 corpus to complete the 82-child descriptor mapping
before any Model-40 physical-mode selection. No current profile is
physical-qualified by this audit.