# T372 S2: Model-339 Reference-Model Admissibility

## Decision

No numeric Model-339 physical boundary is admitted in S2.  The owner permits
86Box, MAME, and PCjs as secondary references only when a primary manual has
a range or no deterministic value.  That permission does not make an
incompatible default, an undocumented emulator clock, or a source-code
constant a Model-339 board fact.

The selected baseline remains IBM PC/AT 5170 Model 339 / Type 3: 8 MHz 80286,
512 KB planar RAM, Rev.3 ROM slot, 101-key keyboard, CGA, no fixed disk, and
the separately identified 1.44 MB field upgrade.  A reference observation is
eligible only when its recorded run proves every applicable one of those
inputs, the exercised route, its reference revision, and the observation
method.  It may corroborate a primary-source range or source-absent behavior;
it never supersedes IBM, Intel, or controller documentation and never imports
reference code, ROMs, media, or captures.

## Source and configuration result

| Candidate | Publicly established configuration fact | Model-339 comparison | S2 disposition |
| --- | --- | --- | --- |
| IBM/Intel/controller manuals | The existing T369--T371 ledgers establish signal, lifecycle, and controller-domain contracts, but do not select Model-339 READY/wait, arbitration, controller-to-project-clock, CGA dot/retrace, or reset-settle values. | Primary authority is necessary but insufficient for the unresolved board conversion. | Retain its exact range or absence.  A later observation may only constrain a boundary which that primary result leaves open. |
| 86Box `ibmat` | Its machine notes state that IBM AT on-board memory is limited to 512 KB. | This corroborates the selected planar population, but the notes identify neither Model 339/Type 3/Rev.3/101-key/CGA/no-HDC/field-upgrade tuple nor a measured board phase. | **Not admitted** as a numeric or waveform source.  It becomes eligible only after a revision-pinned, fully recorded matching configuration and an observation design tied to a primary-source-open boundary. |
| MAME `ibm5170a` | MAME names `ibm5170a` "PC/AT 5170 8MHz", but its inherited RAM default is 1664 KB; its AT source also marks the ISA-bus clock as undetermined. | The CPU rate alone matches.  The default memory and unselected board/drive/video tuple do not; MAME itself supplies no selected ISA clock. | **Rejected** for every Model-339 scalar, wait, or waveform.  It may remain a qualitative logical-acknowledgement comparison after a matching configuration is reproducibly established. |
| PCjs 5170 | Its 5170 documentation describes Models 319/339 as Type 3, 8 MHz, 512 KB and Rev.3, exposes CGA and 1440-KB floppy configuration, and says the 3.5-inch values require Rev.3. | These documented fields make a candidate configuration possible, but S2 has no revision-pinned, run-recorded instance proving the whole no-HDC/field-upgrade tuple or any physical timing observation. | **Not yet admitted**.  It is the only named reference with publicly described controls covering most selected fields; a later S may qualify one precise instance and a bounded observation, without treating PCjs as IBM authority. |

The public references used above are the [86Box IBM AT notes](https://github.com/86Box/docs/blob/master/hardware/machinespecific.rst), [MAME AT driver](https://raw.githubusercontent.com/mamedev/mame/master/src/mame/pc/at.cpp), and [PCjs 5170 documentation](https://www.pcjs.org/machines/pcx86/ibm/5170/).  They are research provenance only under the [source and research policy](../operations/policy/source-policy.md); no source text or asset is copied into NXVM.

## Residual-boundary matrix

| Residual group from S1 | Primary result | Secondary-reference applicability | S2 result and receiver |
| --- | --- | --- | --- |
| 80286 retirement, prefix, fetch and external occupancy | T368 closes successful-retirement accounting; Intel's execution values do not select a Model-339 fetch/prefetch or external-bus phase. | The owner exception is for CPU instruction timing only where the primary result is ranged or absent.  No still-open successful-retirement row is presented here. | No scalar selected.  Fetch/prefetch and bus occupancy remain the Model-339 physical-phase receiver in this open T372 audit. |
| RAM, ROM, CGA, ISA and I/O READY/waits | Intel specifies the CPU interface and IBM identifies topology, not the selected device assertion or project-clock conversion. | All available named defaults are incomplete or incompatible; MAME additionally leaves ISA clock undetermined. | No scalar selected.  A later T372 implementation S requires a primary-open boundary plus a fully matched, revision-pinned observation. |
| DMA, PIT, PIC and interrupt acknowledgement | Chip manuals define controller behavior; the prior ledgers do not derive HRQ/HLDA/DACK/AEN, PIT conversion, IRQ/INTA spacing, or a Model-339 waveform. | A logical callback/order is not a physical probe.  No qualified matching observation exists. | No scalar selected; retain physical arbitration and acknowledgement at the Model-339 physical-phase receiver. |
| FDC, RTC, KBC and keyboard | Controller/drive manuals retain independent domains and the 1.44 MB drive is a field upgrade, not a factory IBM timing value. | No named reference run proves the exact field-upgrade drive plus the selected board tuple; emulator controller behavior cannot replace the drive/controller contract. | No scalar selected; retain controller-domain conversion and service delay at the Model-339 physical-phase receiver. |
| Planar parity, NMI and CGA | IBM/topology evidence proves source and routing, not propagation delay, dot/retrace, contention or cadence. | Configuration documentation and logical rendering do not supply a measured board phase. | No scalar selected; retain propagation/display timing at the Model-339 physical-phase receiver. |
| Reset, cancellation and replay | Project evidence proves deterministic logical ordering only. | A reference boot/reset result without a bounded clocked probe is not a settle-time measurement. | No scalar selected; retain reset-settle and physical replay boundaries at the Model-339 physical-phase receiver. |

## Admission rule for the next observation

A subsequent T372 S may admit one reference observation only after recording:

1. the reference name, immutable revision, machine identifier and all selected
   Model-339 fields, including explicit absence of fixed disk/ATA;
2. the exact public, redistributable test stimulus and an observation schema
   that excludes ROM, firmware, guest-media, and raw reference traces from the
   repository;
3. the primary-manual range or explicit absence that makes this a permitted
   secondary comparison; and
4. a result expressed as corroboration or a bounded model choice, never as an
   IBM hardware measurement.

Until those four conditions are met, the exact receiver is the next admitted
T372 physical-phase implementation/research S.  T372 cannot claim 5170
Model-L3 readiness from this S2 matrix.

## Similar-issue sweep

The S2 sweep reviewed every S1 residual group and each owner-authorized
reference (86Box, MAME, PCjs), then compared their documented configuration
against the locked Type-3 baseline.  No source, ROM, media, binary, capture,
runtime behavior, ABI, or timing value changed.  The exclusion is systematic:
none of the six residual groups has both a primary-open numeric boundary and a
qualified same-profile observation.
