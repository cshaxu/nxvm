# T388 S7: 80286 LSL Architecture Reconciliation

`M5:T388:S7:80286-LSL-ARCHITECTURE-RECONCILIATION:OK`

## Corrected Finding

T388 S6 correctly recorded that the current classifier and owner smoke execute
successful protected-mode `0F 03 /r LSL r16,r/m16` forms, but incorrectly
retained descriptor byte/page granularity as an 80286 timing receiver. Intel's
80286/80287 Programmer's Reference Manual Appendix B assigns `LSL` fixed
register `14`, memory `16` clocks. Its descriptor architecture has no page
Granularity (`G`) timing distinction. That distinction belongs to the later
80386 descriptor format and must be considered only by the 80386 timing owner.

The already-existing 80286 branch is exact for this bounded source claim:
when the completed instruction is unprefixed protected `0F 03`, it publishes
`14` for register ModR/M and `16` for memory ModR/M. The focused owner smoke
executes valid and invalid selector register outcomes and direct/indexed memory
outcomes; it observes `14`, `14`, `16`, and `16` respectively. A ZF-clear
selector result is still a successful `LSL` retirement and remains on the same
source row. No descriptor payload field selects another 80286 clock row.

## Evidence Boundary

This record supersedes only S6's active claim that 80286 `LSL` needs a
descriptor-granularity receiver. S6 and its P history remain immutable facts:
its actual classifier and smoke evidence are retained, while the corrected
architectural interpretation is recorded here. Historical T359/T360 documents
that describe 80386 descriptor granularity remain historical transfers; they
are not changed by this correction.

The correction does not validate descriptor semantics, a 80386 `LSL` row,
prefixes, fault/delivery, x87, waits, bus/device service or a physical clock.
The real T388 residual receivers remain unprefixed-success exclusions, generic
prefix/default paths, range-only arithmetic, x87/physical service and every
80386 form/context not separately sourced.

## Primary Anchors

- Intel, *80286 and 80287 Programmer's Reference Manual* (1987), Appendix B,
  `LSL r16,r/m16`: protected-mode `14,mem=16` clock row; descriptor fields are
  the 80286 format. [Primary scan](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf).
- Intel, *80386 Programmer's Reference Manual* (1986), descriptor format and
  `LSL` timing contexts: page-granularity is a 80386-only descriptor attribute.
  [Primary scan](https://bitsavers.org/components/intel/80386/230985-001_80386_Programmers_Reference_Manual_1986.pdf).
- T366 S28 and T368 S2 already record the same exact 80286 `14/16` row; this
  S reconciles T388 with those retained primary-source records.

## Similar-Issue Sweep

`rg -n -i -C 2 "lsl|granul|granularity|0f 03|14/16" docs/etc/evidence
docs/history/M5-T388-four-profile-cpu-physical-timebase-closure.md
docs/states/CURRENT.md cmake tests/machine`

| Hit class | Disposition |
| --- | --- |
| Current 80286 classifier and owner smoke | Exact `0F 03` register/memory `14/16` owner retained; no code change. |
| T366 S28 and T368 S2 evidence | Already correct primary-source records; retained. |
| T388 S6 evidence/history/current summary | Immutable S6 evidence/history is corrected prospectively by this S7 record and replacement current progress; no history rewrite. |
| T359/T360/363 historical transfers | Contain 80386 descriptor-granularity context or old cross-task transfer wording; historical evidence, not an active T388 receiver. Retain under their closure boundary. |
| 80386 instruction and descriptor source | Outside S7: its descriptor-page context remains a genuine later T388 receiver. |

## Static Boundary

`verify-t388-80286-lsl-architecture-reconciliation` binds the 80286 source
classifier's `0F 03` and `14/16` shape, the focused smoke and this record. It
rejects removal of the corrected architecture separation. It is a recurrence
detector, not a descriptor-semantic, 80386-timing, physical-clock or L3 proof.