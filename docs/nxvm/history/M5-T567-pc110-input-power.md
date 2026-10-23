# M5 T567 — PC110 Keyboard, Pointer And Power Disposition

## Closure

T567 is closed as the bounded PC110 input-and-power disposition task, not as a
complete PC110 power-management or booting-product claim.

S1 separated source-qualified keyboard/TrackPoint facts from unqualified
power behavior.  S2 delivered one neutral 8042-style Board topology: MyPC110
now uses the existing KBC and PS/2 transport at `60h`/`64h`, with IRQ routing,
but deliberately has no PC/AT output-port A20/reset wiring, port `92h`, VADP
or host-to-BIOS injection.  Both stable Ninja widths pass the direct MyPC110
composition regression and the 230-test X86 NXVM unit corpus.

S3 found that U6 physical power sequencing and the existence of APM services
do not amount to a complete guest-visible protocol.  In particular,
historical `0ECh/0EDh` attribution conflicts with the later source-qualified
VL82C420 configuration-bank route.  No fabricated mailbox, lifecycle alias or
successful placeholder is installed.

The retained endpoint is the source-gated
[PC110 power management and APM](../proposals/m5-pc110-power-apm.md) receiver.
Display/font, storage, audio, firmware boot and final product qualification
remain independently queued.  No raw firmware, media, asset or executable is
committed by this task.
