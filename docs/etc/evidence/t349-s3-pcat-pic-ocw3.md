# T349 S3: PC/AT PIC OCW3 Modes

## Manual contract and one owner

The Intel 8259A datasheet defines OCW3 status selection as persistent, except
for poll; a poll command makes the next read an acknowledgement that returns
the selected priority level and sets ISR when a request exists. It also defines
special mask as a priority effect, not an ISR deletion, and SFNM as a
master-cascade exception that admits higher-priority work within the active
slave. See the [Intel 8259A datasheet](https://www.cs.umb.edu/cs341/Intel8259/I8259APIC.pdf),
pages 14--17.

`pic.c` retains one state owner. OCW3 reads use the S2 controller selection and
`RespondINTR` acknowledgement path; priority comparison, cascade selection,
and EOI all use the same controller state rather than a poll-private mirror.

## Reproduced repairs

| Contract | Prior construction | Repaired behavior |
| --- | --- | --- |
| Status read selection | ICW1 reset OCW3 to zero, and an OCW3 poll command with `RR=0` erased a prior IRR/ISR read choice. | Initialization selects IRR. OCW3 preserves `RR/RIS` whenever `RR=0`; repeated selected status reads remain stable after a poll. |
| Poll | `P=1` only formatted a return byte and persisted into every later read. | The next port read selects an eligible request, calls the one acknowledgement owner, returns `I|level`, and clears `P`; no-request poll returns zero without publication. AEOI remains the same `RespondINTR` behavior. |
| Special mask | An OCW1 write destructively cleared every masked ISR bit. | ISR is retained. Priority and non-specific EOI use an effective service view that excludes only masked ISR levels while SMM is set, preserving the manual's nested-service boundary. Specific EOI remains explicit. |
| SFNM | The prior generic `<=` comparison enabled equal-priority behavior on either controller and did not model the master-cascade condition. | Only master IR2 may re-enter while its IR2 ISR bit is active, only with ICW4.SFNM, and only when the slave has a higher-priority eligible request. Normal nested mode remains blocked. |

## Focused proof

`tests/machine/core_machine_pic_ocw3_smoke.c` emits
`M5:T349:S3:PIC-OCW3:OK` and proves:

- default and persistent IRR/ISR port reads;
- poll request/no-request, one-shot P clearing, ISR/IRR acknowledgement, and
  AEOI;
- normal nested blocking, special-mask entry, retained masked ISR, lower
  request delivery, non-specific EOI preservation, and SMM exit;
- normal versus SFNM master behavior after slave IRQ14 is in service and
  higher-priority slave IRQ8 arrives.

The sweep covered every OCW3 writer/read, `imr`/`isr` priority comparison,
poll branch, non-specific EOI, SFNM comparison, cascade select, and public
scan/peek/get caller. No device provider or CPU binding changed. S4 retains
spurious IRQ7/15 and source/reset/finalize composition.

## Verification

The owner target is registered once in `PROJECT_CURRENT_SMOKE_TARGETS`; the
T345 exact pure owner-test count is 124, with three mixed targets unchanged.
Focused PIC and device/CPU regressions, documentation governance, diff check,
and the complete current gate are recorded with the S3 acceptance commit.
