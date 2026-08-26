# T479 S3 Immutable HDC Personality Contract

`M5:T479:S3:HDC-PERSONALITY-CONTRACT:OK`

S3 removes the default-PC/AT profile's duplicate ATA-shaped `hdc_pio` payload.
The profile now holds one copied `core_machine_hdc_config`; the existing plan
copies that one value to the one Core HDC owner.  `CORE_MACHINE_HDC_PROTOCOL_INVALID`
is zero, so an omitted protocol is rejected by existing Core topology
validation rather than silently becoming ATA.

| Route | Retained construction input | Retained semantic reason |
| --- | --- | --- |
| Default-AT | `descriptor.hdc`, explicit `ATA_PIO`; session passes it directly to the existing plan call. | ATA-3 PIO task-file/LBA behavior remains unchanged. |
| DeskPro Model 40 | `core_machine_hdc_config`, explicit `COMPAQ_WD_40MB`, constructed with the already source-qualified Compaq ports. | Its `3F7h` wired-OR rule remains validated in Core against the FDC owner. |
| IBM 5170 Model 339 | Explicit `INVALID` configuration while `hdc_present` remains false. | Preserves the accepted no-disk profile and prevents accidental ATA selection. |

No command-phase, media, IRQ, DRQ or deadline state moved from
`core_machine_hdc`; VM continues to own image bytes and persistence only.
The current flat port fields are the common task-file form used by the two
implemented personalities. S6 must introduce a distinct non-task-file form
for IBM XT rather than add XT behavior to ATA fields.

## Focused proof

- The zero-initialized HDC configuration is rejected by
  `core-machine-hdc-smoke` before the explicit ATA configuration is accepted.
- ATA, Compaq, Model-40 and PC/AT retained-route tests all pass:
  `core-machine-hdc-smoke`, `core-machine-compaq-hdc-s5-smoke`,
  `core-machine-compaq-hdc-machine-s5-smoke`, `vm-hdc-port-smoke`,
  `vm-pcat-composition-s4-smoke`, `vm-pcat-topology-s2-smoke`, and
  `vm-model40-hdc-s26-smoke`.
- `cmake --build --preset current-gates-gcc` passes all 78 configured gates;
  its ATA feature verifier now checks the copied Core personality rather than
  the deleted VM mirror.

## Simplicity accounting

`git diff --numstat -- src cmake tests` records 64 added and 87 removed lines,
for **-23** tracked implementation/test/gate lines (documentation excluded).
The material deletion is the 18-line VM-to-Core field-by-field HDC copy plus
the 19-line profile-only payload. The retained extra lines make the protocol
invalid by default and prove that rejection; they replace an implicit behavior
with one Core-owned validation rule, not a second production path.
