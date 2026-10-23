# M5 T566 — PC110 Minimal Base Composition

## Closure

T566 is closed as the bounded PC110 base-composition task, not as a runnable,
booting or qualified PC110 product.

Its accepted delivery is deliberately layered:

- S1 freezes the construction ledger and proves why the existing PC/AT Board
  assembly cannot be reused unchanged.
- S2 makes keyboard, display and PC/AT facade selections explicit in the
  existing X86 NXVM Board lifetime. Existing PC Apps retain their defaults;
  the neutral choice keeps PIC, PIT and DMA while omitting forced endpoints.
- S3 creates the sole App-local PC110 assembly: a 4 MiB 80486SX/no-FPU Core,
  neutral controller substrate, typed TFT BIOS mapping and existing VL82C420
  configuration routes.

The final App regression confirms the physical TFT map and absence of 60h/64h,
92h, VADP and the PC/AT high reset-ROM alias. It uses a reset only to activate
the admitted controller registers; it neither claims a PC110 reset vector nor
executes firmware. Both stable Ninja/ccache widths pass the three MyPC110
units plus the neutral-controller substrate unit. Documentation governance and
the MyPC110 inward-dependency gate pass.

No Lib, Emulator, X86 Core, X86 Debug or X86 IBM PC source/test contract is
changed by S3. No raw firmware, media, INI or EXE is committed or published.

The next queue entries remain the mandatory receivers for keyboard/pointer/
power, F65535 display/font, PCMCIA/storage, audio/peripherals, real firmware
POST/boot and final MyPC110 qualification. They must connect the real device
contracts before any PC110 executable is offered.
