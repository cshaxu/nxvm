# T580 S35 — PC110 Post-Delay Receiver Inventory

## Baseline and method

The selected external TFT BIOS remains SHA-256
`232101C88466F311BCC32FBC215A4D7569F695CE19F9C07CA67CE2AEE5232312`.
This inventory examined every MyPC110 route installed by
`pc110_plan.c`, the App route implementations, the selected BIOS's nearby
`F000:5405h` return boundary, and source records in Open-Source-PC110.

PC110-EMU and PC110-QEMU were inspected only to identify non-importable
shortcuts. They are not source for a product implementation.

## Route reconciliation

| BIOS-visible surface | Existing NXVM owner | Evidence | Disposition |
| --- | --- | --- | --- |
| `60h`/`64h` keyboard controller | Board 8042 topology | Open-Source-PC110 Bowman/Pluto maps U67 to the classic 8042 DBB, IRQ1/IRQ12, A20G and reset lines. | Source-backed existing owner. |
| `ECh`/`EDh` indexed configuration | `pc110_config.c` / VL82C420 | Existing captured cold bank and S30 gate evidence. | Source-backed existing owner; not a live U6 MCU. |
| `15E8h`/`15ECh` mailbox | `pc110_ec_mailbox.c` | Pluto source identifies the mailbox but not a full command/reply protocol. | Deliberately bounded probe surface; no next guest protocol admitted. |
| `35EAh`/`35EBh` indexed EC block | `pc110_ec_block.c` | Open-Source-PC110 live capture establishes static indices and AC-related observations, but not a complete dynamic interface. | Captured L2 read owner; no fabricated power behavior. |
| `80h`--`8Fh` / `8Bh` IPL latch | Board plain scratch file | S29 source reconciliation. | Existing passive owner; no inferred nonzero reset. |
| F/C firmware shadows | `pc110_firmware.c` | T574/T575 mappings and selected BIOS bytes. | Existing source-backed owner; no C-segment alias beyond proof. |
| ATA / LPT probe | Board ATA and `pc110_lpt.c` | Open-Source-PC110 identifies ATA and observed LPT probe transition. | Existing bounded owners; no guest-media claim. |

## Why no new receiver is admitted

Open-Source-PC110's power-sequence record establishes U6 wiring, wake/power
enable outputs, a P2 handshake and APM-facing telemetry, but explicitly leaves
power thresholds, regulator details and the full host protocol incomplete.
The Bowman/Pluto source likewise identifies custom wiring while declaring the
relevant ASIC register behavior unavailable without a bus trace or fuller BIOS
reconstruction.

PC110-QEMU's `pc110post.c` directly intercepts BIOS addresses including
`F000:61BEh` and `F000:EA90h`; its chipset model seeds synthetic register
images. PC110-EMU contains analogous post helpers and synthetic power/firmware
models. These are useful evidence that a missing interface exists, but they
cannot establish the real protocol or be copied into NXVM.

## Disposition

S35 makes no source/test/asset change. The next legitimate receiver requires
one of: a real trace that defines the accessed custom/MCU protocol; an original
controller specification; or an owner-approved bounded L2 model with explicit
values, reset/failure semantics and direct falsifying test. Until then, T580
cannot claim POST, Setup, boot, power management or a publishable MyPC110 EXE.
