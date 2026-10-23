# PC110 Minimal Base Composition

## Admission And Dependencies

Unnumbered M5 candidate.  Allocate a numeric T only when admitted.  It receives
the source-backed configuration and TFT firmware bindings from closed T565 and
precedes any PC110 endpoint task that requires a live Core construction.

## Objective And Scope

Create the one PC110 App-local base composition without borrowing a PC/AT
profile.  Split only the actually reusable standard-controller substrate from
the current X86 NXVM board assembly where that assembly currently forces AT
display, keyboard, A20, ROM-alias or reset policy.  Attach only source-backed
PIC, PIT, DMA and RTC port/timeline responsibilities; retain PC110-specific
reset, shadow, KBC, panel, storage, PCMCIA, power and audio behavior at their
named receivers.

The composition consumes the existing PC110 VL82C420 route adapter and typed
TFT firmware provider.  It must have one construction/lifetime owner and must
not publish a runnable executable until its device contract is complete.

## Non-goals And Stop Conditions

No PC/AT descriptor reuse, BIOS interception, guessed reset/CMOS/SCAMP
defaults, generic device manager, second executor, artificial POST success or
runtime firmware selection.  Stop and transfer any controller behavior whose
PC110 route/timing/reset evidence is not sufficient for an explicit L2 model.

## Verification And Exit

Direct construction tests prove atomic attach/teardown, exactly one owner for
each admitted standard route, firmware-map preservation and absence of forced
AT endpoints.  Both Ninja/ccache widths run the affected Core/X86 NXVM/App
tests.  Full boot, display, input, storage, power, audio and release artifacts
remain for their separate receivers unless this task proves their real complete
connection.
