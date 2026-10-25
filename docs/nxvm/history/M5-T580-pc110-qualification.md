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

The subsequent S47 shadow receiver implements the per-16-KiB C--F access
fields and source-labelled E/F Flash view, retiring App index/write-state
mirrors, prefilled guest shadow and the F9 hard-gate hypothesis. Both widths
execute the unchanged initializer, handler copy and return; enhanced existing
timer integration passes with delta 166E after one processor reset. The
[S47 evidence](../etc/evidence/t580-s47-cold-initialization-contract.md)
separates this direct proof from still-open SMRAM isolation, SMIACT forwarding,
trigger/RSM and boot-only Flash selection. S47 and T580 remain active.

The final S47 continuation uses Shared `794380f69` for CPU/DMA-qualified
memory routing, bounded cache-view invalidation, existing SMIACT forwarding,
chip ASMI and the missing F65535 DAC status read. The App owns the separate
SMRAM contents and hardware decode. Full BIOS caller reconciliation rejects
the draft's bit-7 trigger and establishes source mask 20 from the initializer
that registers the vendor handler and enables ASMI. The actual BIOS executes
that service, acknowledges it and returns through RSM on both host widths;
timer delta is 166C. All 712 repository unit cases pass per width, all four
receiving PC INI identities pass and the affected eight PC/two MyRaidenII
artifacts rebuild. Lib, Emulator and MyNES stay unchanged. Evidence and actual
source/test counts are in the [S47 record](../etc/evidence/t580-s47-cold-initialization-contract.md).
This is receiver delivery pending coordinator acceptance, not complete POST,
guest or PC110 product qualification.
