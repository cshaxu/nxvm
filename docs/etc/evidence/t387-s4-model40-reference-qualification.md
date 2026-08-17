# T387 S4: Model 40 Reference Qualification

`M5:T387:S4:MODEL40-REFERENCE-QUALIFICATION:OK`

## Scope

This is a read-only qualification of the local secondary-reference candidates
specified by T387 S3. No candidate was launched, no trace was captured, and no
firmware, guest media, source or local path is imported into NXVM. A candidate
qualifies only if its checked source/configuration proves both the original
1986 DeskPro 386 Model 40 board composition and the system-PIT-counter-1 to
port-`61h` refresh-pulse-visible route.

## Candidate Results

| Candidate | Immutable local revision | Exact-model evidence | Route and selection disposition | Result |
| --- | --- | --- | --- | --- |
| 86Box | `4fef696a4eead1d55a28d6ac0e5bd2864e5454da` | `machine_table.c` exposes `[ISA] Compaq Deskpro 386`, and `m_at_386dx.c` selects the 16 MHz-capable DeskPro CPU package and an optional September 1986 BIOS entry. | Its DeskPro initializer calls `machine_at_common_init()`. That common initializer binds generic AT refresh to primary PIT channel 1 and installs the default `port_6x_device`. The default device returns only generic PPI/speaker state at `61h`; its independent refresh toggle exists only for the PS/2/Xi8088 flagged variants. The DeskPro initializer adds no Model-40-specific port-61 route or second 8254 at `48h-4Bh`. | Rejected for this receiver: the model label is not proof of the required D4 counter-1/port-61h route. |
| PCjs | `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70` | `tools/pc/compaq386.json` labels a DeskPro 386 and models 32-bit CPU/RAM/ROM objects. | The checked configuration selects 2 MiB extended memory, IBM VGA, AT HDC and 1.44 MB floppies. Its own DeskPro EGA documentation says the displayed configuration uses IBM EGA because Compaq EGA support has not been completely tested. The checked configuration contains no selected D4 dual-8254 or counter-1/port-61h route. | Rejected: it is a documented non-baseline configuration and does not expose the required observable. |
| MAME | No local source or configuration manifest was present in the owner-managed reference corpus. | No checked evidence. | Not inspected as an implementation candidate; absence is not a statement about upstream MAME support. | Unavailable locally, not a negative claim about MAME. |

## Reproducible Sweep

The qualification sweep searched local reference source/configuration manifests
for `compaq`, `deskpro`, `model 40`, `port 61`, PIT counter/channel and machine
initializers. It then inspected the 86Box DeskPro table/initializer, its common
AT initializer and port-6x implementation; it inspected PCjs's `compaq386`
configuration and DeskPro adapter documentation; and it enumerated available
reference source roots for a MAME manifest. The checked 86Box branch does not
contain a separate Model-40 `61h` receiver, and the checked PCjs configuration
is explicitly a variant rather than the selected D4 backbone. No generic AT,
IBM Model 339, clone or later DeskPro result was promoted as a substitute.

## Decision And Transfer

The S3 bridge contract has no qualified local candidate. The first calibration
receiver therefore remains unobserved. This negative result is useful: it
prevents a generic 86Box AT/PIT cadence, PCjs IBM-adapter configuration, or a
reference-internal scheduler from becoming an NXVM CPU/PIT scalar.

A later bounded research S may qualify a new exact Model-40 implementation or
obtain a primary corpus that joins the board observable to the required domain.
Until then, source-backed board behavior may proceed only where it does not
require the missing conversion; CPU-to-PIT calibration, DMA DCLK placement,
physical periods and all L3 conclusions transfer unchanged.
