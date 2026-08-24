# T450 S19 Controller Source Form, Sufficiency And Coverage Reconciliation

## Direct PDF Form Inspection

S19 opened/rendered all admitted controller PDFs and compared their rendered
first pages with full-document `pdftotext` output and font metadata.  This is
form evidence, not a claim that OCR text is error-free.

| Manual | Directly observed form | Required reading consequence |
| --- | --- | --- |
| Intel 8259A | Scan with OCR text layer | Rendered tables/diagrams govern. |
| Intel 8237A | Scan with OCR text layer | Rendered tables/diagrams govern. |
| Intel 8254 | Scan with OCR text layer | Rendered timing tables govern. |
| Motorola MC146818A | Scan without usable text layer (122 extracted bytes) | Every value must be read from rendered pages. |
| Intel UPI-42 | Scan with OCR text layer | Rendered pin/timing tables govern. |
| NEC uPD765 | Scan with OCR text layer | Rendered command/timing tables govern. |
| IBM Options/Adapters Vol. 2 | Mixed: scanned pages, OCR body, cover without text | Rendered EGA pages govern. |
| X3T13 ATA-3 | Born-digital text | Rendered source remains page authority. |
| IBM 5170 board reference | Mixed: scanned pages with OCR body, cover without text | Rendered board pages govern. |

## Primary-Manual Sufficiency And Reference Boundary

All eight source checklists now carry an S19 form/sufficiency record.  The
manuals are sufficient for their chip/interface facts.  The unresolved inputs
are not OCR gaps: selected board wiring/service phase (PIC/DMA/PIT/RTC/KBC),
drive/media/revision and physical cadence (FDC), selected adapter/monitor
(VADP), and selected controller/device/backing-media/cable (ATA).  FDC's
Version command-count wording is the sole identified internal manual ambiguity.

No 86Box, MAME, PCjs, Bochs or QEMU source was used.  If a later task needs one
for a bounded behavior cross-check, it must label it reference-derived and may
not replace a primary fact, select a personality, or become a dependency.

## Coverage Reconciliation

The eight source lists S3/S5/S7/S9/S11/S13/S15/S17 and eight audits
S4/S6/S8/S10/S12/S14/S16/S18 reconcile their actual frozen universes: 128
source rows and 128 audit rows.  The earlier five 15-row batches and older
16--18-row batches are intentionally not renumbered or normalized. Each audit preserves one current owner path
and assigns every partial/missing/blocked item once to its named queued
controller phase contract.  No source list duplicates another controller's
row, and no audit claims full implementation or L3 closure.

S19 changes documentation only.  `git diff --check` and documentation
governance are required for acceptance; no build is relevant.
