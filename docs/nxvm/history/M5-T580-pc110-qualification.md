# M5 T580 — MyPC110 product and guest qualification

## Admission

T580 consumes the existing [MyPC110 qualification proposal](../proposals/m5-pc110-qualification.md)
after T579's bounded real-firmware source gate.  Its end state is four
build-fixed artifacts: original-DSTN x86/x64 and upgraded-TFT x86/x64.  The
task may only claim those artifacts runnable after their specific firmware,
panel, machine, lifecycle, configuration and guest proof gates have passed.

S1 is an evidence-led convergence audit.  It must enumerate every qualification
member and place each in a bounded successor, a source/asset-gated transfer or
an evidenced rejection.  It cannot turn the existing partial firmware trace
into a product acceptance or add a guessed fallback.
