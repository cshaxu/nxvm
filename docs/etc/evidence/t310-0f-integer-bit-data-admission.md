# M5 T310 S2: 80386 `0F` Integer Bit/Data Family Admission Audit

## Scope And Authority

This admission freezes only the requested 80386 `0F` integer bit/data forms:
`SETcc`; `BT`, `BTS`, `BTR`, and `BTC`, including `0F BA /4`--`/7`;
`SHLD`/`SHRD`; `BSF`/`BSR`; `MOVZX`/`MOVSX`; and the already decoded two-
operand `IMUL` (`0F AF`). It does not admit paging, debug/test registers,
task/V86/system forms, later CPUs, a new memory route, or a guest-image
fixture.

Intel's *80386 Programmer's Reference Manual*, Chapter 4 instruction
descriptions and Chapter 6 protection rules, is the semantic authority. The
read-only comparison identities are Bochs 2.6 `cpu/fetchdecode.cc`,
`cpu/bit16.cc`, `cpu/bit32.cc`, `cpu/data_xfer16.cc`, and
`cpu/data_xfer32.cc`; and PCjs 2.00.0
`machines/pcx86/modules/v2/x86ops.js`, `x86op0f.js`, and `x86help.js`.
They are behavior cross-checks only; no source is copied. The preceding
[T309 form audit](t309-80386-form-audit.md) established why this reachable,
unproven family ranks before paging and system candidates.

## Decoder And Mode Boundary

`core_machine_cpu_instruction_metadata_get` marks every admitted row below as
valid from the 80386 profile. `ExecIns` applies the primary metadata/profile
gate, and `INS_0F` repeats the `0F` form gate before its one dispatch table.
Thus pre-80386 execution must finish as architectural `#UD` before a source
read, destination write, or flag change. `0F BA /0`--`/3` is invalid in that
same route and must likewise be `#UD` without an operand access.

The forms are ordinary data instructions in real mode and non-V86 protected
mode. Operand size is the CS default XOR `66h`; address size is independently
the CS default XOR `67h`. `67h` changes only memory effective-address
decoding. It cannot widen an operand or change the register half selected by
an operand-size form. V86 is deliberately outside this packet, so no claim
about its additional execution checks is made.

All future proof uses one new focused prepared-state probe. It must obtain
guest instruction/data accesses through the retained core executor and checked
memory route, record the first diagnostic, and assert no publication on an
access/decode/profile failure. It must not use an image, host callback, or a
second CPU/memory owner.

## Frozen Form Matrix

| Family and encodings | Form, prefix, and location contract | Flags and commit contract | S2 implementation disposition |
| --- | --- | --- | --- |
| `SETcc`, `0F 90`--`9F /r` | All 16 conditions write `r/m8`; ModRM register and memory forms are legal. `67h` selects only the memory address form. `66h` does not change the byte destination. | Reads the named condition from the entry EFLAGS and leaves EFLAGS unchanged. Decode/read/limit failure leaves destination and flags unchanged. | Reachable but unproven; admit the complete condition table as one batch. |
| `BT`, `BTS`, `BTR`, `BTC`, `0F A3`/`AB`/`B3`/`BB /r` | `r/m16, r16` or `r/m32, r32` by operand size; both register and memory destinations are legal. Register destinations reduce the bit index modulo 16 or 32. For a memory bit string, the signed register bit offset selects the containing 16- or 32-bit element before its in-element bit is selected. `67h` applies to the starting effective address. | `CF` receives the old selected bit. The modifying forms write the resulting element only after the read and arithmetic have succeeded; `BT` is read-only. Architecturally undefined non-CF flags are not stable-probe assertions. | One coupled batch because all four share decode, bit-string element selection, CF, and write-commit logic. |
| `BT`, `BTS`, `BTR`, `BTC`, `0F BA /4`--`/7 ib` | The same `r/m16`/`r/m32` rules apply, with an unsigned immediate bit offset. The immediate may select a later memory element; register destinations use the in-width bit. `/4` is read-only; `/5`--`/7` modify. `/0`--`/3` remain `#UD`. | Same CF and write/atomicity rules as register-index forms. The `#UD` groups must not read the ModRM operand or immediate-derived memory. | Coupled with the register-index batch; it is not separable from bit selection semantics. |
| `SHLD`, `0F A4`/`A5`; `SHRD`, `0F AC`/`AD` | `r/m16, r16, imm8/CL` or `r/m32, r32, imm8/CL` by operand size. ModRM destination can be register or memory; `67h` affects only its address. The effective count is masked to five bits. | Count zero preserves destination and all flags. For defined nonzero counts, future proof asserts Intel-defined result/CF and only defined flags; it must not turn architecturally undefined result/flags into a contract. A failed source/destination access cannot publish the destination or flags. | One coupled double-shift batch because immediate/CL and both directions share count masking and flag boundaries. |
| `BSF`, `0F BC`; `BSR`, `0F BD` | `r16, r/m16` or `r32, r/m32` by operand size; source is register or memory and `67h` affects only memory addressing. | A nonzero source clears ZF and commits the first/last index. A zero source sets ZF; the destination is architecturally undefined and therefore is not used as a positive portability assertion. Other undefined flags are not frozen. Read failure cannot change destination or flags. | One scan batch; the two direction variants share width, zero, and commit boundaries. |
| `MOVZX`, `0F B6`/`B7`; `MOVSX`, `0F BE`/`BF` | `r16/r32, r/m8` for B6/BE and `r16/r32, r/m16` for B7/BF, selected by operand size. Sources may be register or memory; `67h` affects only memory. | No flags change. Source read/decode/limit failure must leave the destination and EFLAGS unchanged. | Reachable candidate: the B7/BF handlers currently hard-code a 32-bit destination while their B6/BE peers use operand size. S2 must first prove or refute the 16-bit B7/BF form before making a minimal correction. |
| Two-operand `IMUL`, `0F AF /r` | `r16, r/m16` or `r32, r/m32` by operand size; source can be register or memory, with `67h` affecting only memory. | Signed truncated product commits to the register destination. CF and OF report whether it fits the destination width; other arithmetic flags are undefined. Source failure cannot publish a destination or defined flags. | Independent signed-multiply batch after the shared operand/address retained probes are green. |

The static read of `_d_bit_rmimm` confirms that the existing path already
models signed register-index memory element selection, including negative
offsets, and masks in-element indexes. That is an implementation observation,
not proof. The focused bit batch must exercise positive and negative register
indexes, immediate indexes, 16/32-bit elements, and failures before treating
that behavior as admitted.

## Independent Implementation Batches

| Batch | Exact scope | Focused prepared-state proof | Retained intersections and stop boundary |
| --- | --- | --- | --- |
| S2 | `SETcc` only | All 16 predicates, register and memory `r/m8`, `67h` memory form, unchanged EFLAGS, profile `#UD`, and destination nonpublication on checked-memory failure. | Retain operand/address, checked-memory, and profile-gate probes. Stop if a condition requires an unadmitted flags producer change. |
| S3 | `MOVZX`/`MOVSX` only | B6/B7/BE/BF with 16/32 destination widths, signed/zero extension, register/memory source, `66h`/`67h`, profile `#UD`, and failed-read nonmutation. This batch resolves the B7/BF candidate before any broader claim. | Retain operand/address, real-mode 386 address, checked-memory, and profile-gate probes. Stop if a result requires a public register-layout change. |
| S4 | `BT`/`BTS`/`BTR`/`BTC`, register-index and `0F BA /4`--`/7` | Register and memory bit strings, 16/32 elements, signed index and immediate element selection, CF, write versus read-only forms, `/0`--`/3 #UD`, and failed current-element nonpublication. | Retain operand/address, checked-memory, and profile-gate probes. Stop if the first failure belongs to paging or a different fault-delivery policy. |
| S5 | `SHLD`/`SHRD` only | 16/32 widths, immediate and CL count, count zero, masked counts, only defined flags/results, register/memory destination, and read/write failure boundaries. | Retain operand/address, checked-memory, and profile-gate probes. Stop if an undefined Intel result would be needed as a test oracle. |
| S6 | `BSF`/`BSR` only | 16/32 widths, register/memory source, first/last index, zero/ZF behavior, defined-flag discipline, profile `#UD`, and failed-read nonmutation. | Retain operand/address, checked-memory, and profile-gate probes. Stop if an observation depends on undefined destination state. |
| S7 | `0F AF` two-operand `IMUL` only | 16/32 signed products, fit/overflow CF+OF, register/memory source, `66h`/`67h`, profile `#UD`, and failed-read nonpublication. | Retain operand/address, checked-memory, and profile-gate probes. Stop if expansion would include one-operand or immediate IMUL forms. |

Each later batch may extend the same focused synthetic test executable, but
must retain its own marker and assertions so a later family cannot silently
weaken an earlier one. No batch implies correctness of primary arithmetic or
other metadata-valid `0F` forms.

## Similar-Issue Sweep And Deferred Boundary

S2 swept the requested metadata rows, `INS_0F`, the initialized `0F` table,
`_GetOperandSize`, `_GetAddressSize`, ModRM/decode and checked-memory entry
points, `_d_bit_rmimm`, flag helpers, and existing focused smoke registration.
No second executor, alternate RAM path, or profile bypass was found. The
future batches must repeat that sweep for changed helpers and inspect all
production callers before a shared helper changes.

Deferred without inference are debug/test-register forms, later `0F` forms,
paging policy and faults, task/V86/system behavior, and product observation.
This S2 audit makes no artifact, CMake, Queue, CPU, ABI, or product-path
change.
