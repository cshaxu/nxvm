# T580 S17 — PC110 post-DD05 port observation

## Question

Does the existing real-TFT path reach the proposed `F000:DD05` checkpoint
after the established C000-to-port-61 return, and if so does its next
execution select an unmodeled port receiver that can be implemented without
inventing a cold chipset image or a synthetic firmware route?

## Method

The existing App-local real-firmware diagnostic was temporarily widened.  It
would retain a single port read or write only after retirement observed
`F000:DD05`; every port direction and value was eligible.  The diagnostic
continued through the ordinary Core executor and tick axis.  Its fixed
eight-million-instruction limit was containment only, never a success
condition.  The temporary mode was run on both x64 and x86, then removed.

## Result

Both hosts reached the same prior port-61 progress boundary without a Core
fault, but neither reached `F000:DD05` before containment ended.  The final
observation remained in the existing firmware delay region (`F000:EA8C`).
Consequently the capture never armed and cannot say whether a later I/O owner
exists.  This is a rejected checkpoint, not a negative observation of panel,
power, PCMCIA, storage, guest or lifecycle behavior.

## Disposition

No receiver is selected and no production model is added.  In particular,
this does not justify seeding live SCAMP/ECED values, adding a synthetic
power or panel device, or treating an execution budget as POST completion.
The temporary capture was removed.  Further progress requires a source-defined
checkpoint that is actually reached, or an independent reproducible semantic
checkpoint.
