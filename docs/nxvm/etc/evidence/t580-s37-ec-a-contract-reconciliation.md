# T580 S37 — EC-A Guest-Visible Contract Reconciliation

## Method

S37 compared the selected PC110 BIOS's known EC-A helper family with the
existing App receiver and the clean-room Open-Source-PC110 records. The BIOS
has helpers for `15E8h`/`15ECh`, indexed `15EAh`/`15EBh`, and output through
`15EEh`; this is an interface inventory, not a claim that any helper is
reached during the unqualified POST path.

## Contract table

| Surface | Source-backed semantics | Existing owner | Disposition |
| --- | --- | --- | --- |
| `15E8h` data and `15ECh` command/status | ULTRACHG disassembly proves ready when status bits 0--1 are clear, command `31h`, and data writes for `Zn10`/`Zn00`. It does not prove a busy duration or general command grammar. | `pc110_ec_mailbox.c` returns ready (`0`) and records command/data writes. | Complete for the admitted L2 ready/write receiver; no change. |
| `15EAh`/`15EBh` | BIOS helper accesses exist, but the source corpus does not establish reset image, index/data semantics, or response/failure behavior for this EC-A pair. | No route. | Source-gated; do not invent an indexed file. |
| `15EEh`/`15EFh` | SCAMP descriptors and BIOS helper establish an output port address, but no guest-visible command, reply, reset or failure contract. | No route. | Source-gated; no zero/stub route. |
| `35EAh`/`35EBh` EC-B | Open-Source-PC110 captures a 32-entry indexed file and masks the index to five bits. | `pc110_ec_block.c`. | Existing source-backed owner; outside EC-A work. |

## Result

The only complete EC-A contract is already implemented by the existing
mailbox owner. The other two BIOS-visible pairs lack the minimum reset,
operation and failure facts required for a receiver. PC110-QEMU's synthetic
register file and status-settle behavior are explicitly not used as a contract.

No production/test/asset change is warranted. T580 remains source-gated for
POST, Setup and guest qualification until an original controller specification,
a real trace with sufficient semantics, or an owner-approved explicit L2 model
defines one of the missing interfaces.
