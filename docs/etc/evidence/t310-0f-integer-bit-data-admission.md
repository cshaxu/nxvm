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
| S3 | `SETcc` only | All 16 predicates, register and memory `r/m8`, `67h` memory form, unchanged EFLAGS, profile `#UD`, and destination nonpublication on checked-memory failure. | Retain operand/address, checked-memory, and profile-gate probes. Stop if a condition requires an unadmitted flags producer change. |
| S4 | `MOVZX`/`MOVSX` only | B6/B7/BE/BF with 16/32 destination widths, signed/zero extension, register/memory source, `66h`/`67h`, profile `#UD`, and failed-read nonmutation. This batch resolves the B7/BF candidate before any broader claim. | Retain operand/address, real-mode 386 address, checked-memory, and profile-gate probes. Stop if a result requires a public register-layout change. |
| S5 | `BT`/`BTS`/`BTR`/`BTC`, register-index and `0F BA /4`--`/7` | Register and memory bit strings, 16/32 elements, signed index and immediate element selection, CF, write versus read-only forms, `/0`--`/3 #UD`, and failed current-element nonpublication. | Retain operand/address, checked-memory, and profile-gate probes. Stop if the first failure belongs to paging or a different fault-delivery policy. |
| S6 | `SHLD`/`SHRD` only | 16/32 widths, immediate and CL count, count zero, masked counts, only defined flags/results, register/memory destination, and read/write failure boundaries. | Retain operand/address, checked-memory, and profile-gate probes. Stop if an undefined Intel result would be needed as a test oracle. |
| S7 | `BSF`/`BSR` only | 16/32 widths, register/memory source, first/last index, zero/ZF behavior, defined-flag discipline, profile `#UD`, and failed-read nonmutation. | Retain operand/address, checked-memory, and profile-gate probes. Stop if an observation depends on undefined destination state. |
| S8 | `0F AF` two-operand `IMUL` only | 16/32 signed products, fit/overflow CF+OF, register/memory source, `66h`/`67h`, profile `#UD`, and failed-read nonpublication. | Retain operand/address, checked-memory, and profile-gate probes. Stop if expansion would include one-operand or immediate IMUL forms. |

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

## S3 SETcc Evidence

S3 corrects the planned implementation identities without changing their
scope: SETcc is S3, MOVZX/MOVSX is S4, bit test/modify is S5, SHLD/SHRD is
S6, BSF/BSR is S7, and two-operand IMUL is S8. This preserves the original
independent batch boundaries after the committed T310 S2 audit occupied its
former first implementation identity.

Intel 80386 PRM Chapter 4 defines SETcc as a byte `r/m8` destination selected
from the entry condition flags without changing EFLAGS. The S2 read-only
Bochs 2.6 and PCjs 2.00.0 paths remain the comparison identities for this
family. The production sweep covered every `SETO_RM8` through `SETG_RM8`
table entry, `_m_setcc_rm`, the `INS_0F` metadata/profile gate, `_d_modrm`,
the byte register/reference write path, `_GetAddressSize`, and flag writes.
All sixteen entries use the one checked `r/m8` write route; no second executor,
memory route, or condition-flag producer is involved.

`core-machine-setcc-smoke` is the focused prepared-state proof. It executes
both truth values for all sixteen conditions into AL and a normal 16-bit
addressed memory byte, preserving the full entry EFLAGS in every case. It
also proves `66h` leaves the byte destination width unchanged, and `67h`
selects the 32-bit effective-address form without changing EFLAGS or ESI.
An 80286 profile receives `#UD` before an AL write or EFLAGS publication. A
prepared 80386 protected DS limit makes a `67h` memory SETcc write fail; the
existing T308 contributor-delivery route terminates that no-IDT fixture as
`#DF`, while the destination byte, EAX, EFLAGS, and EIP retain their entry
values. The terminal diagnostic is not a new SETcc exception policy.

The direct marker is `M5:T310:S3:SETCC:OK`. S3 retains the operand/address,
checked-memory, and CPU profile-gate smoke intersections. The focused proof
found no production SETcc defect, so S3 adds the probe and CMake registration
but deliberately does not refactor otherwise correct CPU handlers. No
artifact, Setup observation, Queue change, or product-path change is made.

## S4 MOVZX/MOVSX Evidence

Intel 80386 PRM Chapter 4 defines B6/B7/BE/BF with an operand-size-selected
register destination: a 16-bit destination updates only its low half, while a
32-bit destination receives the complete zero- or sign-extended result.
Bochs 2.6 `cpu/data_xfer16.cc` and `cpu/data_xfer32.cc`, and PCjs 2.00.0
`machines/pcx86/modules/v2/x86op0f.js` and `x86help.js`, remain read-only
behavior comparisons; no source was copied.

The focused `core-machine-movx-smoke` established the S2 candidate as a real
defect. `MOVZX_R32_RM16` and `MOVSX_R32_RM16` decoded their destination and
wrote it as four bytes regardless of operand size. S4 changes only those two
calls to use `_GetOperandSize`, matching their B6/BE peers. In particular,
`66h`/16-bit B7 and BF now retain the high sixteen bits of the destination.
The sweep covered B6/B7/BE/BF metadata and table entries, `_d_modrm`,
`_m_read_rm`, `_m_write_ref`, operand/address-size selection, sign/zero
extension, EFLAGS writes, and focused registration. No second executor or
memory route was found.

The probe covers B6/B7/BE/BF with byte/word register and memory sources,
zero/sign extension, both 16/32-bit destination widths, `66h`, and a combined
`67h`/`66h` memory form. It preserves EFLAGS in every success case. A
read-counting provider proves that all four forms receive `#UD` on 80186 and
80286 before ModRM source access or destination publication. The 8086 profile
is explicitly excluded from that assertion: retained
`core-machine-cpu-profile-gate-smoke` preserves its historical `0F` `POP CS`
compatibility route, which is neither MOVZX nor MOVSX support. A prepared
protected DS-limit source read fails through the retained checked-memory and
T308 delivery route; its terminal no-IDT diagnostic is `#DF`, while ECX,
EFLAGS, and EIP retain entry values.

The direct marker is `M5:T310:S4:MOVX:OK`. S4 retains the S3 SETcc,
operand/address, real-mode 386 address, checked-memory, and CPU profile-gate
intersections. No artifact, Setup observation, Queue change, or product-path
change is made. BT-family, double-shift, scan, IMUL, paging, system, task,
and V86 work remain deferred.

## S5 Bit-Test/Modify Evidence

Intel 80386 PRM Chapter 4 defines `BT`, `BTS`, `BTR`, and `BTC` as
`r/m16, r16` or `r/m32, r32` forms selected by operand size, with `CF`
receiving the prior bit.  For a memory bit string, the signed register index
selects the containing element before its in-element bit; an unsigned `ib`
index on `0F BA /4`--`/7` follows the same element-selection rule.  The other
status flags are architecturally undefined and are deliberately not test
oracles.  Bochs 2.6 `cpu/bit16.cc`, `cpu/bit32.cc`, and `cpu/fetchdecode.cc`,
plus PCjs 2.00.0 `machines/pcx86/modules/v2/x86op0f.js` and `x86help.js`, were
read-only behavior comparisons; no source was copied.

The focused probe found one production defect: `_d_bit_rmimm` correctly
selected a later memory element for signed register indexes but reduced an
immediate index to its in-element bit without advancing the memory offset.
S5 adds the missing `2 * (ib / 16)` or `4 * (ib / 32)` adjustment only for a
memory immediate operand.  Register destinations remain reduced within their
operand width.  The change retains the one executor, ModRM decoder, checked
logical memory route, and write path.

`core-machine-bit-test-smoke` covers all four register-index and immediate
group forms, 16- and 32-bit elements, register and memory destinations,
`66h`, combined `67h`/`66h` addressing, read-only `BT`, and modifying
`BTS`/`BTR`/`BTC`.  It proves signed negative register-index selection and an
immediate index that advances to the next memory element.  Access-counting
providers show invalid `0F BA /0`--`/3` and 80186/80286 forms reach `#UD` before
an operand read or write.  The retained 8086 `0F` `POP CS` compatibility is
not a bit-family form and remains covered by `core-machine-cpu-profile-gate-
smoke`.

Two protected prepared states prove current-element nonpublication: a
DS-limit read failure for `BT`, and a read-only DS write failure for `BTS`.
Both use the retained checked-memory and terminal no-IDT delivery route; the
observed terminal diagnostic is the established `#DF`, while `CF`, `EIP`, the
register index, and the physical destination element remain at entry state.
This does not alter exception-delivery policy.

The sweep covered A3/AB/B3/BB and BA metadata/table entries, `INS_0F` profile
and invalid-group gating, `_d_bit_rmimm`, `_m_read_rm`, `_m_write_rm`,
operand/address prefix selection, and focused-smoke registration.  No second
executor, memory route, or public surface was introduced.  SETcc, MOVX,
SHLD/SHRD, BSF/BSR, IMUL, paging, system, task, and V86 work remain deferred.
The direct marker is `M5:T310:S5:BIT:OK`; S5 creates no artifact, Queue
change, Setup observation, or product-path change.

## S6 SHLD/SHRD Evidence

Intel 80386 PRM Chapter 4 defines `0F A4/A5` (`SHLD`) and `0F AC/AD`
(`SHRD`) for a 16- or 32-bit `r/m` destination and register source.  The
immediate or `CL` count is masked to five bits.  A resulting zero count is a
complete no-op; for defined nonzero counts, `CF`, `SF`, `ZF`, and `PF` are
defined, while `OF` is defined only for count one and `AF` is undefined.
Results outside the operand-width domain are not test oracles.  Bochs 2.6
`cpu/bit16.cc`, `cpu/bit32.cc`, and `cpu/fetchdecode.cc`, and PCjs 2.00.0
`machines/pcx86/modules/v2/x86op0f.js` and `x86help.js`, were read-only
behavior comparisons; no source was copied.

The new focused proof found a count-zero publication defect.  `_a_shld` and
`_a_shrd` correctly returned for a masked zero count, but all four immediate
and `CL` handlers then wrote the stale `instruction_state.data.result` to the
destination.  S6 gates that write on the same masked count.  No nonzero
arithmetic, flag, decoder, memory, or executor route changed.

`core-machine-double-shift-smoke` exhaustively covers the defined 16-bit
counts one through sixteen and 32-bit counts one through thirty-one for both
directions, both immediate and `CL` forms, and register and memory
destinations.  It verifies `66h`, combined `67h`/`66h` memory addressing,
result and defined flags, and count-zero destination/full-EFLAGS no-op.
The profile cases prove 80186 and 80286 receive `#UD` before destination or
defined-flag publication; the retained 8086 `0F` `POP CS` compatibility is
not a double-shift form and remains covered by the profile-gate smoke.

Protected DS-limit read failure for `SHLD` and read-only DS write failure for
`SHRD` take the existing checked-memory and no-IDT terminal route.  The
established `#DF` diagnostic is observed while the physical destination,
`EIP`, and defined entry flags remain unchanged.  This adds no delivery
policy.  The sweep covered A4/A5/AC/AD metadata and table entries,
`_a_shld`, `_a_shrd`, their four handlers, count masking, read/write helpers,
prefix selection, profile gate, and focused registration.  No public surface
or second route is involved.  The marker is `M5:T310:S6:DOUBLE-SHIFT:OK`;
S6 creates no artifact, Queue change, Setup observation, or product-path
change.  BSF/BSR, IMUL, paging, system, task, and V86 work remain deferred.

## S7 BSF/BSR Evidence

Intel 80386 PRM Chapter 4 defines `BSF` and `BSR` as 16- or 32-bit register
destinations with an `r/m` source.  A nonzero source clears `ZF` and writes the
lowest or highest set-bit index.  A zero source sets `ZF`; its destination and
the flags other than `ZF` are undefined, so the probe does not make them test
oracles.  Bochs 2.6 `cpu/bit16.cc`, `cpu/bit32.cc`, and `cpu/fetchdecode.cc`,
and PCjs 2.00.0 `machines/pcx86/modules/v2/x86op0f.js` and `x86help.js`, were
read-only behavior comparisons; no source was copied.

`core-machine-bit-scan-smoke` proves both BC and BD forms with nonzero and
zero register and memory sources, 16/32-bit destination widths, `66h`, and a
combined `67h`/`66h` memory source.  Nonzero cases assert the required index
and `ZF=0`; zero cases assert only `ZF=1`.  An access-counting provider proves
80186 and 80286 receive `#UD` before the source access.  The retained 8086
`0F` `POP CS` compatibility remains outside this family.

Protected DS-limit read failures for both scans take the retained checked
memory/no-IDT terminal route.  The established `#DF` diagnostic is observed
while ECX, EIP, and the entry EFLAGS, including `ZF`, retain their values.  The
sweep covered BC/BD metadata/table entries, `_a_bscc`, zero-test and
destination-reference publication, operand/address prefixes, profile gate,
and focused registration.  Existing production behavior passed; no executor,
memory route, public surface, or product path changed.  The marker is
`M5:T310:S7:BIT-SCAN:OK`; S7 creates no artifact, Queue change, or Setup
observation.  IMUL, paging, system, task, and V86 work remain deferred.
