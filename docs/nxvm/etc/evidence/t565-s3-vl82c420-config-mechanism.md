# M5 T565 S3 — VL82C420 configuration mechanism

## Boundary

`x86-core/chips/vl82c420` models only indexed configuration storage and its
access protocol. It owns neither an I/O port, a PC110 board, DRAM timing,
ROM shadowing nor a reset register image. The later PC110 board maps guest
ports to this address-neutral API and supplies only source-qualified values.

## Evidence and model grade

Open-Source-PC110's chipset investigation, section 13h, disassembles the
firmware sequence of reads from `FC23`, `F023`, `C023`, then `0023` before
accessing Block2 through its index/data pair. Section 13i records a live dump
using the same sequence. Its section 13j documents the live SCAMP indexed
window and section 13k establishes EC/ED indexed DRAM geometry registers.

The four-step ordering, Block2 gate and SCAMP's seven-bit index alias are
therefore modeled as source-backed behavior. The general opaque storage layer
and its zero-filled construction state are L2 scaffolding only: they make no
claim about a physical power-on register value. No default register value,
bit meaning or cold-reset image is inferred here.

## Reset contract

`x86_vl82c420_reset_protocol()` clears only transient selection/unlock state.
It preserves programmed bank bytes, matching the documented warm-reset
retention. A cold-board image is deliberately outside this chip mechanism
until the PC110 board owns a qualified seed. This avoids making unknown power
defaults look factual.

## Direct proof

The Core test proves:

- SCAMP index bit 7 aliases to bit 0;
- Block2 rejects index/data access until the exact four reads occur;
- a wrong read resets the partial unlock sequence;
- explicit relock blocks access again;
- Block2 and EC/ED stores are independent; and
- protocol reset relocks without erasing an existing EC/ED value.

It does not prove guest port decode, firmware POST, real DRAM configuration,
desktop behavior or hardware timing.
