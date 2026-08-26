# T490 S1 IBM 5160 8253 PIT Original-Source Ledger

`M5:T490:S1:8253-ORIGINAL-SOURCE:ACCEPTANCE-CANDIDATE`

## Correction before List 1

The IBM 5160 is wired with an Intel **8253-5**, not an 8254.  The initial
candidate name was corrected before List 1: an 8254's read-back command and
status latch are not XT 8253 requirements.  No PIT code changed and no 8254
behavior may be added through this task merely because the existing common PIT
implementation labels itself 8254.

## Visually checked primary sources

Primary sources remain in the owner-managed `O:\assets` manual archive and
are not imported into NXVM.

| Source | Archive identity / visual condition | Pages checked | Facts admitted to later List 1 |
| --- | --- | --- | --- |
| Intel, *8253/8253-5 Programmable Interval Timer*, Nov. 1986, order `231306-001` | Public archival mirror `https://radio-hobby.org/uploads/datasheet/155/8253/8253-5.pdf`, retained outside NXVM as `controllers/intel/231306-001_8253_8253-5_Programmable_Interval_Timer_Nov1986.pdf`; SHA-256 `5E5B5737ED82D244682C9E768098F910326697DE9589DFA09EE2ABE6030C8DB5`; 11-page image scan, visually legible; no OCR assertion. | All PDF pp. 1--11 (printed pp. 3-51--3-61). | Three independent 16-bit binary/BCD counters; four port-select functions; six modes; control/count write order; direct and latched reads; Gate and output transitions; reload/zero rules; chip electrical timing/waveforms. |
| IBM, *IBM 5160 Technical Reference*, Apr. 1983 | `controllers/ibm/IBM_5160_Technical_Reference_APR83.pdf`; SHA-256 `18CD473851FDFE40C5BF2C7CCC870772857D84509D95ADA7880473CD802C63B8`; scanned board reference, visually legible for selected pages. | PDF pp. 19--26 (printed System Unit pp. 1-7--1-14). | System-board 8253-5 presence; `40h`--`43h` port range; counter 0 timer/IRQ0 role; counter 1/2 and PPI port-B speaker/gate relations; ISA timing context. |

## Explicit source boundary

The existing local Intel 8254 data sheet (`231164-005`) was visually checked
only to identify the incompatible superset feature: its `SC=11` Read-Back
command/status latch does **not** enter the 8253 ledger.  It is not a
normative T490 source.  Pin/electrical waveform figures are physical-source
facts; their unqualified conversion into Core elapsed ticks remains outside S1
and cannot be inferred.

The primary basis is sufficient to freeze the selected 8253 and IBM wiring
List 1 in S2.  No external emulator or second controller path is required for
this source-admission stage.
