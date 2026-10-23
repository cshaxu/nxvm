# T566 S1: PC110 Minimal Base-Composition Audit

## Scope and evidence boundary

This is a construction audit, not a PC110 boot claim.  It traces the current
`x86-nxvm-board-base` constructor and compares its actual ownership with the
admitted PC110 board facts from T558/T565.  The Open-Source-PC110, PC110-EMU
and PC110-QEMU checkouts remain read-only research.  Their implementations are
not imported.  The only firmware candidate remains the separately governed,
external TFT BIOS binding established by T565.

## Current constructor fact

`x86_nxvm_board_attach_machine()` has one useful construction lifetime: it
binds the Core attachment, initializes clock domains, then owns the standard
controller state and its teardown.  Its controller mechanisms are reusable;
its endpoint installation is not neutral:

| Current action | Current owner | PC110 disposition |
| --- | --- | --- |
| Core attachment, clock domains, DMA bus, cascaded/single PIC bus, system PIT and IRQ0 output | `x86-nxvm/board-base` | Reuse in the same board lifetime.  PC110 evidence identifies standard 82C37, 82C59 and 8254-compatible functions; exact PC110 wiring remains App-selected. |
| RTC object, 70h/71h routes and IRQ binding | `x86-nxvm/board-base` optional topology | Reuse only after a PC110 composition supplies an explicit RTC seed, IRQ and timing source.  No AT CMOS defaults or checksum synthesis are admitted. |
| VL82C420 configuration ports | `app-mypc110/pc110_config` plus `x86-core/chips/vl82c420` | Already App-routed and remains separate from the standard-controller substrate. |
| Immutable TFT BIOS map C0000h--FFFFF | `app-mypc110/pc110_firmware` plus Core firmware map | Already typed and remains App-owned input. |
| High reset ROM alias | automatic `board-base` firmware callback | Extraction required.  A 486 reset vector needs a high reset view, but automatic PC/AT policy is not a PC110 endpoint.  The later PC110 composition must select its map explicitly from the typed firmware map. |
| Port 92h/A20 route | unconditional `board-base` call | Excluded.  PC110 A20/reset belongs to its later input/power receiver, not an AT port facade. |
| VADP creation and display ports | unconditional `board-base` call | Excluded.  PC110 F65535/panel is a later display receiver. |
| XT PPI or 8042 keyboard creation and IRQ1/IRQ12 wiring | unconditional branch in `board-base` | Excluded.  PC110 KBC, power and pointer are later input/power work. |
| FDC/HDC allocation | unconditional internal allocation in `board-base` | Not an endpoint route and not usable PC110 storage.  Retain only while it is private, inert setup; do not attach AT FDC/HDC routes.  A later storage receiver decides whether lazy construction is worth isolating. |
| PC/AT profile callbacks, refresh and AT contract materialization | existing App/board profile path | Excluded.  PC110 does not borrow an AT descriptor or profile factory. |

## Minimal extraction decision

The smallest legal change is **not** a second board manager and is not a new
generic device framework.  It is an extension of the existing
`x86_nxvm_board_config` construction selection so the existing board lifetime
can create its reusable controller substrate without installing an endpoint.

S2 will add explicit no-endpoint selections for the three currently forced
families:

1. keyboard endpoint: existing 8042 and XT PPI values retain their numeric
   values and behavior; a new `NONE` value creates neither keyboard facade nor
   keyboard IRQ sources;
2. display endpoint: existing VADP remains the zero/default choice for every
   current App; a new `NONE` value creates no VADP or display I/O routes;
3. reset/A20 facade: existing automatic reset alias and port-92 behavior remain
   the zero/default choice; a no-facade selection leaves future PC110 input and
   firmware composition as the only endpoint owners.

This preserves the four existing Apps' aggregate-zero configuration behavior
and keeps a single board object, single Core attachment and single controller
timeline.  The plan validator must accept the three `NONE` choices and reject
profile callbacks which request unavailable keyboard services.  The attachment
callbacks must be null-safe when a receiver is absent; they must not synthesize
success or reintroduce an AT endpoint.

## Implementation cohort and tests

| Cohort | Owner | Required direct proof | Stop condition |
| --- | --- | --- | --- |
| S2: neutral controller substrate | `src/test/x86-nxvm/board-base` | A no-endpoint plan binds Core, creates PIC/PIT/DMA, exposes only their standard routes and tears down atomically; all four existing App board tests retain their endpoint routes. | Any controller constructor itself relies on VADP/KBC/A20 state. |
| S3: PC110 minimal composition | `src/test/app-mypc110` | One App-owned assembly supplies 486SX, source-labelled clocks, TFT map and VL82C420 routes; it proves no 60h/64h, 92h or VADP routes are installed. | Need for unspecified reset alias, RTC seed, KBC/power or display policy. |
| Later endpoint receivers | their queued PC110 tasks | Each real port/device attaches through the existing composition only after source-backed settings exist. | Missing source or firmware behavior is not replaced with AT behavior. |

The S2 tests are X86 NXVM unit tests, with an App-level negative route test in
S3.  They do not assert a POST, console, Window, external ROM or executable.

## Similar-issue sweep

The four current PC Apps use the default VADP/keyboard/reset-A20 selections
and materialize their own profile/plan data.  No App may be changed to use the
PC110 no-endpoint selection.  No reverse App dependency is needed: PC110 uses
the inward X86 Core/X86 NXVM board API; existing Apps continue to own their
board definitions.

## Unexecuted proof

This audit does not prove PC110 ROM POST, RTC image, DMA channels, reset
aliasing, panel rendering, keyboard/pointer, power, storage, PCMCIA or audio.
Those facts remain deliberately unclaimed until their separately named
receivers supply evidence and tests.
