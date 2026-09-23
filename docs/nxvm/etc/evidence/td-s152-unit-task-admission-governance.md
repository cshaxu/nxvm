# Td S152 Per-Unit Hardware Task Admission Governance

`M5:TD:S152:UNIT-TASK-ADMISSION-GOVERNANCE:ACCEPTED`

The owner rejected finding-driven hardware repair.  The shared M5 admission
program now requires every new CPU, chip, controller, media-controller or
display category to be one independent T with four ordered stages: original
manual/board-source ledger, complete List 1 function-and-timing ledger,
complete List 2 current-code gap ledger, then a single-owner implementation
and closure batch.

T485 is withdrawn after accepted S1--S20 rather than declared complete.  Its
remaining aggregate XT scope transfers to these ordered candidates:

1. 8088 instruction function/timing;
2. 5160 8088 board, RAM/ROM and ISA;
3. 5160 8259A;
4. 5160 8237A;
5. 5160 8254;
6. 5160 8255 PPI/keyboard/NMI;
7. 5160 8272A/FDC/media;
8. 5160 CGA/VADP; and
9. 5160 Xebec fixed disk.

Each proposal names one Core owner and explicitly excludes adjacent chips,
profiles, VM state and duplicate routes.  The final XT L3 audit remains after
all unit tasks; the pre-Windows audit remains after the XT audit.  No runtime,
ABI, build, artifact, source import or timing claim changes in this Td.
