# M5 T580 S21 — External MCU And POST Model Boundary

## Reviewed material

The following external, read-only inputs were checked against the current
MyPC110 boundary:

| Input | Relevant finding |
| --- | --- |
| `PC110-EMU/Sources/PC110Core/pc110_core.c` | Its PC110 POST helpers complete named BIOS loops, including memory-pattern loops, in host code. |
| `PC110-QEMU/qemu/target-i386/pc110post.c` | Its optional `PC110POST` route intercepts named ROM offsets, changes registers/memory and provides synthetic INT 13h boot data. |
| `PC110-EMU/Sources/PC110Firmware/PC110FirmwareModel.cpp` | The PowerSense model reads image metadata and returns model-defined indexed values; it is not an M38223 execution engine or a PC110 bus receiver. |
| `Open-Source-PC110/Discovery/Power-Sequence/readme.md` | It supplies physical U6/M38223 power signals and MCU routine anchors, but no CPU-port or firmware-call request/reply trace for an emulator receiver. |
| External `pc110-power-mcu.rom` and `pc110-keyboard-mcu.rom` | BYOB raw inputs exist; their availability does not itself define an x86-visible protocol. |

## Decision

The two reference implementations are L2 research aids only.  Their named-ROM
loop completers, synthetic disk/interrupt handling and metadata controller
cannot be adapted into NXVM: doing so would create exactly the firmware-address
behavior, host shortcut and fabricated device path prohibited by T580.

The present Core KBC may continue to serve only its already-proven compatible
PC keyboard route.  Loading an M38813 image into that route without its actual
CPU-visible board protocol would be a false claim.  Likewise, the M38223
physical power sequence does not identify a port, call ABI, reply, reset state
or timing for an x86 power/APM receiver.

## Required next evidence

Any later PC110 receiver must first supply one of:

1. a reproducible real-firmware trace that identifies an x86-visible request,
   reply and reset/lifecycle result; or
2. an original/reliable controller specification defining that complete
   contract.

Only then may an App-local receiver be designed.  It must preserve the one
Core clock and ordinary bus/lifecycle route; it must not be a ROM-address
intercept or a second execution framework.

## T580 qualification consequence

T580 currently has no evidence-based route from the accepted TFT firmware
checkpoint to real POST, setup, guest storage, power/panel behavior or a
publishable MyPC110 artifact.  This is a source/contract gate, not a compiler
or test failure.  The existing other-app artifacts and Shared/Core interfaces
remain unchanged.
