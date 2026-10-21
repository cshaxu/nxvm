# T7 S46 Sound ED Family Ledger

## Scope And Authority

This is a static, source-free receiving audit of the one local Sound ED decoder.
It reads no owner ROM and retains no guest byte, address, instruction trace or
media payload. The original Zilog Z80 CPU User Manual UM008011-0816 identified
in the [research register](../research/references.md) is the L3 authority for
documented ED forms, their operation, flags and cycle counts.

## Current ED Disposition

| Family | Local state | Owner-valid disposition |
| --- | --- | --- |
| Interrupt mode and return | IM aliases plus RETI/RETN subset are present. | Retain; a separate interrupt-contract correction owns the existing no-interrupt sentinel. |
| 16-bit `ADC HL,ss` / `SBC HL,ss` | Only `SBC HL,DE` exists; its subtract path confirms the local opcode identity. | Receive all four pair selections for both arithmetic operations through the existing 16-bit arithmetic helper. |
| `LD (nn),dd` / `LD dd,(nn)` | Only the DE forms exist. | Receive all four pair selections through the existing word fetch/read/write helpers. |
| Accumulator / refresh transfers and `NEG` | Not implemented. | Defer to a later register-state receiver; it needs a complete review of existing refresh-state ownership. |
| `RRD` / `RLD` | Not implemented. | Defer to a later accumulator-memory decimal receiver. |
| Block memory copy/compare | Only `LDIR` exists. | Defer to a complete block-memory receiver; it must define every flag and repeat condition together. |
| Port register and block I/O | Not implemented. | Defer: the Sound map must first have an evidence-backed port-device contract. Do not manufacture an I/O route. |
| Undefined ED encodings | Rejected. | Retain rejection. |

## Selected Receiver

The next bounded implementation batch is the pure CPU pair family:

- all four `ADC HL,ss` selections;
- all four `SBC HL,ss` selections;
- all four `LD (nn),dd` selections; and
- all four `LD dd,(nn)` selections.

It has one existing executor, existing pair accessors, word fetch and memory
helpers, and no dependence on a Sound I/O route. The follow-up must use the
manual's 15-T-state arithmetic and 20-T-state absolute-transfer contracts,
test each pair selection and carry/overflow boundaries, and preserve existing
memory-failure propagation. It must not treat this receiver as evidence for
the selected-set terminal or as title/gameplay qualification.

## Verification And Boundary

The local ED switch was read in full, including its only existing arithmetic,
absolute-transfer and block-copy forms. Documentation governance and
whitespace checks pass. The receiving batch is L3 for instruction behavior,
but this ledger does not claim silicon timing outside the Sound's existing
nominal tick model. No Shared, map, product, artifact or external asset path
changed.
