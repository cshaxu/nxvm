# T479 S7 WD1007A-WAH ESDI Current-Gap Ledger

`M5:T479:S7:ESDI-GAPS:OK`

This consumes E1--E11 in the [source ledger](t479-s7-esdi-source-ledger.md).

| Rows | Current owner/path | Required result/disposition |
| --- | --- | --- |
| E1--E2 | `core_machine_hdc` has explicit ATA, Compaq and IBM WD1003 personalities only; every existing VM descriptor selects one of those immutable declarations. | No WD1007A-WAH enum or frozen descriptor exists. Do not add a generic ESDI selection or mutate an existing profile. Transfer to a later selected-profile admission. |
| E3--E7 | The retained HDC owns task-file/PIO/IRQ/CHS state; VM HDD owns bytes and persistence. | The abstract owner split can receive a selected WD1007 task-file branch, but no selected controller profile can validate geometry, firmware/ROM policy or commands. No implementation is admissible before that receiver. |
| E4 | Core causal completion has no 10-us physical timing axis for a WD1007A board. | Preserve the exact manual reset relation in the future controller branch only after a profile declares a compatible physical/board time input; do not fabricate elapsed time now. |
| E8--E10 | No raw ESDI, ECC, cache, alternate-sector, controller ROM or drive-firmware state owner exists. | Retain explicit unsupported boundaries. A future receiver must choose and source each one rather than widening logical media. |
| E11 | `default-at`, Model 339 and Model 40 descriptors are frozen selected configurations, not controller catalogues. | Regress unchanged; their existing hard-disk support remains ATA, IBM WD1003 and Compaq WD respectively. |

## Transfer

No current immutable profile can select WD1007A-WAH without changing a machine's
source-defined identity.  S7 consequently cannot implement an ESDI personality
honestly.  The exact receiver is a future profile admission that names both the
machine and the controller/drive/ROM policy; it must consume E1--E11 and retain
all existing HDC paths.  No placeholder enum, profile field, cache, timer or
ROM loader is created here.
