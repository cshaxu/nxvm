# T572 S1 — PC110 TFT Real-Firmware POST Admission

## Scope And Inputs

This record covers only the TFT PC110 BIOS/font pair selected by the active
packet.  The protected bytes remain outside the repository:

| External input | Size | SHA-256 |
| --- | ---: | --- |
| `profiles-nxvm/ibm-pc110-tft/firmware/pc110-tft-bios-1995-09-19.rom` | 262,144 | `232101c88466f311bcc32fbc215a4d7569f695ce19f9c07ca67ce2aee5232312` |
| `profiles-nxvm/ibm-pc110-tft/firmware/pc110-kanji-font.rom` | 1,048,576 | `9829fdb8281c12022dc3b77686044ed1a5213ab526ce4329f2841cd64171784c` |

`app-mypc110` validates both before CMake generates an ignored immutable byte
source.  There is no runtime firmware path and no raw payload in Git.

## Normal Route

`mypc110_machine_construction_create()` is the sole App composition.  It
selects the existing 80486SX, 20 MiB, neutral-8042, cascaded-PIC, 8254 and
F65535 routes; then it uses the existing profile binding to declare an
80486 shutdown as a processor-only reset.  The normal X86-NXVM executor owns
the reset vector, execution, frame copy and lifecycle.  No firmware-address
interception, POST completer, DOS helper or alternate machine is installed.

The additional board facts below are source-qualified L2 wiring facts from the
read-only PC110 research set, not claimed physical timing reconstruction:

| Firmware-visible route | Owner | Disposition |
| --- | --- | --- |
| 8042 self-test flag before POST | neutral 8042 controller | Persistent command-byte system flag is configured before reset. |
| CMOS `70h/71h`, IRQ8 | existing RTC/CMOS board route | App plan supplies conventional decode and 32,768 Hz timing declaration. |
| Port `61h` refresh-status polling | existing planar-parity port/timer/speaker route | `memory_bytes=0` deliberately leaves PC/AT parity-memory behavior absent; PIT counter 1 supplies only the observed status bit. |
| 80486 shutdown reset | existing profile attachment | Existing `shutdown_reset` returns the CPU to reset without cold-resetting RAM or boards. |

The original `board_deadline` guard made PIT wake deadlines conditional on a
VADP.  A no-VADP PC110 is valid, so the video and PIT deadline calculations are
now independently guarded.  The new Board-Base regression proves a programmed
PIT has a deadline without a VADP.

## Real Firmware Observation

The real BIOS self-test loads an all-ones table descriptor with operand-size
32 and then reads it through ordinary six-byte `SGDT`/`SIDT`.  The prior Core
implementation truncated the sixth byte for the default operand-size form on
80386+ profiles, causing the BIOS to halt at `F6806`.  The generic fix preserves
the full 32-bit base for 80386 and later, while retaining the documented 80286
compatibility byte.  `cpu_sgdt_sidt_smoke` now executes this mixed-size round
trip on 80386 and 80486SX; it is not a firmware-specific CPU path.

The bounded diagnostic starts the generated real image through the normal
machine driver.  It must execute at least 8,000,000 guest instructions and
observe one real 80486 shutdown/reset request; otherwise it fails.  The most
recent x64/x86 runs passed this checkpoint.  It crosses the former descriptor
self-test and shutdown boundary, and then continues through the port-61 path.

This is a POST-admission checkpoint, **not** a completed POST, Setup or DOS
boot claim.  At the budget boundary, the BIOS remains in real guest execution
with delivered exceptions requiring a separate, bounded owner diagnosis before
any product executable is published.  The next receiver is the queued
MyPC110 product-and-guest qualification task: it must identify the first
remaining CPU-visible service/interrupt/device boundary with a finite trace
and add only a source-backed owner route.

## Verification

Both configured Ninja graphs passed:

```text
x86-core.cpu/cpu_sgdt_sidt
x86-nxvm.board-base/board_neutral_controller
mypc110.unit/machine-construction
mypc110.diagnostic/real-firmware-post
```

The diagnostic took approximately 20 seconds on x64 and 24 seconds on x86.
It is labelled `diagnostic`, not a product integration or boot qualification.
No MyPC110 release EXE is emitted by this task.

## Similar-Issue Sweep

The sweep covered existing App firmware bindings, profile attachment, KBC reset
state, VADP/PIT deadline computation and SGDT/SIDT writers.  No second PC110
Core, copied frame path, external runtime file read or PC/AT facade was added.
The two generic defects above were repaired at their existing owners; the
remaining boot/service boundary is transferred explicitly rather than hidden
behind a fallback.
