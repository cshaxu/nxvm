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

## S43--S44: actual timer calibration receiver

S43's sampled execution identifies a deliberate BIOS error halt and a rejected
PIT/refresh delta, superseding the unsampled delay-throughput interpretation.
S44 implementation `1f30224c3` corrects only the App's clock configuration and
existing firmware harness. Both widths report timer pass with delta 1668;
current unit coverage is 712 cases per width, with x64 timeout reruns explicitly
recorded. The code/build-registration delta is +100/-119, net -19.

The [S44 evidence](../etc/evidence/t580-s44-pit-refresh-clock.md) and
[original qualification ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
retain source grades, retained manual views and open product requirements.
Preview construction is verified; complete POST, display, guest and release
qualification remain open. No PC110 product is published by this delivery.

## S45: post-timer reset and firmware service wait

The bounded x64 continuation passes three processor-reset boundaries
and reaches the memory-flag wait calling the 15EE/AX=5380/BH=84 service. It does
not reach a boot interrupt within 200M instructions. No production or test
change remains after removing the sampler. [S45 evidence](../etc/evidence/t580-s45-post-timer-reset.md)
records the actual copied observations and the complete next receiver batch.

## S46: SMI/SMRAM mechanism audit

The [S46 audit](../etc/evidence/t580-s46-smi-smram-receiver-audit.md)
proves that the existing CPU can take SMI but the App has not prepared the
handler's active memory view. It rejects isolated pin, cold-constant and
C-shadow candidate changes, all of which were withdrawn. The next coherent
repair boundary is the complete board cold-decode/SMRAM/SMI mechanism, not a
host BIOS service or another instruction-specific patch.

## S47: port-width prerequisite and configuration receiver

Owner-approved Shared P1 `d603e81e7` preserves actual write widths through the
existing port callback; read callbacks remain unchanged. The App receiver
implements the source-labelled L2 SL-compatible byte/word unlock and reset
protocol, verified by actual CPU OUT instructions. Both widths retain timer
calibration success, and all 712 current unit cases have passing evidence
with explicitly recorded reruns. Eight receiving PC artifacts are refreshed
at 0580; MyRaidenII receives a separate same-version artifact refresh.

The [S47 evidence](../etc/evidence/t580-s47-cold-initialization-contract.md)
records the actual delta, cache repair, test qualifications and artifact hashes.
The complete SMRAM/SMIACT/initializer/trigger receiver remains open; this
delivery does not establish full POST, DOS or a publishable PC110 product.
