# M5 T360: Four-Profile Intel Timing Source Reconciliation

## Task Record

T360 consumes the explicit source-conflict and range transfers from T359. It
reconciles primary Intel timing authority by processor profile without
inventing an average, borrowing a later-profile row, or turning a secondary
emulator into a specification. It is a prerequisite for later selected-profile
cycle work, not a physical-timing or CPU-semantics task.

## Accepted Progress

### S1: Four-profile source-authority and consumer inventory

S1 is accepted at `219043bd`. Its
[source-authority/consumer inventory](../etc/evidence/t360-s1-four-profile-source-authority-consumer-inventory.md)
binds the existing ten timing consumers to the 8086 Tables 2-20/2-21, 80186
Table 1-16, 80286 Appendix B, and 80386 PRM section 17.2.2.3 sources. It
records the formal 80286 NOP table `3` versus prose `2` conflict without
changing the provisional table-based consumer, and distinguishes source
ranges, following-byte/EA context, 80386 `m`/mode context, delivery, physical
service, x87, and VME/PVI boundaries.

The inventory establishes the non-overlapping later units: S2 authority
edition/NOP precedence, S3 8086/80186 range forms, S4 80286/80386 contextual
normalization, and S5 closure audit. It adds only a static source-inventory
verifier; no runtime timing value, artifact, ABI, or CPU behavior changed.
Documentation governance, the verifier, and 245/245 current-gate tests
passed.

### S2: 80286 NOP authority precedence

S2 is accepted at `b29b5fbf`. Its [NOP authority-precedence
record](../etc/evidence/t360-s2-80286-nop-authority-precedence.md) applies the
owner's source-selection rule to the 210498-005 conflict: the formal Appendix
B opcode-table row remains the sole current selected allocation, `NOP = 3`.
The source-ledger entry, classifier routes, direct 80286 proof, generic timing
proof, and downstream scheduler/device records have no prose-derived two-clock
consumer. The unresolved prose statement is retained as a precise
`TODO(Medium)` with an edition/page/context review and complete consumer sweep
as its only admission route.

S2 changes no runtime value, CMake target, artifact, ABI, or CPU behavior.
Documentation governance, the retained T360 source-inventory verifier, and
diff check passed.

### S3: 8086/80186 range-form disposition

S3 is accepted at `63152b80`. Its [range-form disposition
record](../etc/evidence/t360-s3-8086-80186-range-form-disposition.md) confirms
that the selected 8086 `MUL`/`IMUL` and 80186 `MUL`/`IMUL`/`DIV`/`IDIV` plus
immediate-`IMUL` table rows are ranges without a primary operand-to-clock
formula. Both profile-local source routes retain the visible unallocated
successful-retirement transfer; no minimum, maximum, midpoint, later-profile,
emulator, or benchmark approximation is admitted. The exact future admission
is one `TODO(Medium)` requiring a source-backed formula or separately approved
observation contract plus profile-local capture, rollback, and full consumer
sweep.

S3 changes no instruction semantics, runtime timing number, CMake target,
artifact, ABI, or test logic. The retained 8086/80186 timing-ledger smokes,
T360 inventory verifier, documentation governance, diff check, and 245/245
current-gate tests passed.

### S4: 80286/80386 contextual source-row normalization

S4 is accepted at `614751b0`. Its [contextual source-row disposition](../etc/evidence/t360-s4-80286-80386-contextual-source-disposition.md) audits every T359-selected 80286 and 80386 timing owner for following-byte, EA/odd-word, mode/privilege, prefix, `m`, and descriptor-granularity conditions. The selected rows retain one post-refresh publisher; no source-row mismatch or second publisher was reproduced. The unallocated 80286 system/descriptor forms and 80386 descriptor-granular `LSL` remain exact transfers to the later complete instruction-timing corpus rather than borrowing a 80386 number or guessing a capture rule. Fault, delivery, bus, device, and cycle-exact conditions remain separate receivers.

S4 changes no runtime timing value, execution semantics, CMake target, artifact,
or ABI. Documentation governance, the retained T360 source-inventory verifier,
and 245/245 current-gate tests passed.
