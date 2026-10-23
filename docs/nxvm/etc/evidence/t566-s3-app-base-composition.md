# T566 S3 — PC110 App-local Base Composition

## Scope

This S composes only the mechanisms admitted by T566 S1 and S2:

- a 4 MiB 80486SX/no-FPU Core configuration on the documented 33 MHz macro
  axis;
- the neutral X86 NXVM Board controller substrate (cascaded PIC, 8254 PIT and
  two DMA controllers);
- the typed 256 KiB TFT firmware map at `C0000h..FFFFFh`; and
- the App-local VL82C420 route adapter.

`mypc110_base_composition` is the sole owner of that construction. It destroys
the Core first, thereby releasing the Board attachment and all admitted
controllers, then destroys the App-owned configuration chip.

## Explicit exclusions

The composition selects Board keyboard, display and PC/AT-facade topologies
as `NONE`. Therefore it intentionally creates neither a 60h/64h controller,
port 92h, VADP, nor the PC/AT high reset-ROM alias. Calling reset makes the
already-admitted controller registers live, but is not evidence that a PC110
firmware reset vector, POST or guest execution is connected.

No reset receiver, RTC/CMOS seed, panel/display, keyboard/pointer, storage,
PCMCIA, power or audio endpoint is inferred by this S. Those remain with the
ordered PC110 receiver tasks in the NXVM queue.

## Regression boundary

`mypc110.unit/base-composition` proves successful creation/finalization,
the physical TFT mapping, absence of the high reset alias and all forced
PC/AT endpoint ports, while retaining PIC/PIT ports. Configuration route
semantics remain solely in `mypc110.unit/config-route`; the composition test
does not duplicate that lower-level route coverage.

Both x64 and x86 stable Ninja/ccache trees build and pass all three App-local
PC110 units. The existing x86-nxvm neutral-controller regression remains the
separate substrate proof. No executable is published because the result is
deliberately not a runnable product.
