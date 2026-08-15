# T375 S12: Model-339 CGA Identity Ledger

`M5:T375:S12:MODEL339-CGA-IDENTITY-LEDGER:OK`

## Identity Result

The frozen Model-339's selected **digital CGA** capability is the IBM
Color/Graphics Monitor Adapter option, feature `4910`, as listed for Models
319 and 339 in the IBM model documentation.  A companion IBM product guide
identifies that adapter as part `1504910`.  The IBM PC/AT technical reference
also assigns that adapter the `3D0h--3DFh` I/O range.  The adapter remains an
owner-selected expansion option, rather than a claim that the 5170 system
board integrated a display controller or that it shipped in every Model-339
system.

The compatible IBM CGA adapter technical reference establishes the 6845
controller, digital/direct-drive output and its port/register contract.  S8--S11
already consume only that register/status behavior.  This S does not select a
PCB/component revision, a composite-output mode, a monitor, a snow setting, a
factory CRTC mode table, or an oscillator-to-NXVM tick conversion.

Primary documents:

- [IBM PC/AT 5170 Models 319 and 339](https://sharktastica.co.uk/resources/docs/IBM_186-052_5170-319-339_86.pdf)
  lists Color/Graphics Monitor Adapter feature `4910` for the selected models;
  [IBM product guide](https://www.ibm5150.net/files/guide_87.pdf) identifies
  part `1504910`.
- [IBM PC/AT Technical Reference, March 1986](https://www.minuszerodegrees.net/manuals/IBM/IBM_5170_Technical_Reference_MAR86.pdf)
  records the CGA adapter I/O range.
- [IBM Options and Adapters Technical Reference, Volume 2](https://bitsavers.org/pdf/ibm/pc/cards/Technical_Reference_Options_and_Adapters_Volume_2_Apr84.pdf)
  supplies the IBM CGA register/controller/output contract.

## Reference Qualification

| Reference | Read-only local identity | What it can cross-check | What it cannot establish here |
| --- | --- | --- | --- |
| 86Box | `86box-source` detached source revision `4fef696a`; standalone device is explicitly named `IBM CGA` and has ISA identity `cga`. | Its CRTC masks, programmed horizontal display/blank partition and status transition model can be compared against a project-owned register trace. | A Model-339-specific board clock, a chosen IBM PCB revision, monitor behavior, a primary fact, or copied NXVM implementation. It exposes independent composite, RGB and snow configuration dimensions. |
| PCjs | local `pcjs` master source; 5170 color configuration instantiates a generic CGA card when color monitor selection applies. | Model-339 color-versus-mono selection and the generic CGA configuration route. | A precise physical cadence: its own CGA commentary labels parts of its retrace assumptions as assumptions. It does not name a Model-339 CGA board revision. |
| MAME | No checked-out local source/configuration qualified by this S. | None until a revision-pinned Model-339/CGA configuration is supplied to a later measurement contract. | It is not silently substituted by another MAME machine or a generic AT. |

No reference result is admitted as a board fact.  The source trees were read
only; no third-party code, binary, ROM, trace or guest medium entered NXVM.

## Availability Ledger

| Required physical claim | Disposition | Earliest receiver |
| --- | --- | --- |
| IBM CGA option family and PC/AT I/O identity | Primary-established. | Retained Model-339 selected capability. |
| Non-interlaced CRTC geometry and status semantics | Primary-established logical contract; implemented in S11. | Existing VADP owner. |
| Exact character-clock source and CPU-tick conversion | Not established by the selected IBM documents or the frozen profile. | T375 reference-measurement/cadence admission; must use an explicit project-owned measurement contract. |
| Factory default CRTC programming | Not established by the frozen no-ROM profile. | Later firmware/default-table admission, not a timing shortcut. |
| ISA display contention/snow, monitor waveform and output phase | Adapter/revision/output-path dependent and unselected. | Later selected-hardware or explicit L3-boundary decision. |
| MAME cross-check | Reference unavailable in this workspace. | Later revision-pinned reference experiment. |

## Similar-Issue Sweep And Handoff

The sweep covered the Model-339 profile/capability ledger, T374 CGA functional
inventory, T375 CGA timing evidence, local 86Box standalone-device identity,
and PCjs Model-339 color selection.  No project document may use bare
"digital CGA" to imply an integrated AT video controller, a generic-AT timing
scalar, an exact board revision, or a reference-emulator value.

The next eligible CGA timing S is a bounded **reference measurement contract**:
it must name the IBM 1504910-compatible device configuration, reference source
revision, a project-owned CRTC program, sampling/checkpoint schema, comparison
mask and no-progress budget.  It may refine logical phase behavior only after
that contract is reproducible.  Exact physical source publication, board
contention and Model-339 L3 remain open.
