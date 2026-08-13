# T350 S4: PC/AT NMI Producer Decision

## Decision

T350 does not admit a PC/AT parity or I/O-channel-check NMI producer. The IBM
PC/AT reference identifies those sources as distinct board signals, but the
selected NXVM PC/AT profile has no documented input, latch, clear operation,
port/provider, reset owner, timeline producer, or corpus that could define
their state without inventing board behavior. The existing CPU pending-NMI
state and CMOS port-70h bit 7 are consumers/mask state, not a producer.

The result is an explicit transfer to `TODO(Medium) PC/AT NMI source
ownership`. It must be re-admitted only with one selected IBM-PC/AT source,
its assertion and clear/latch semantics, NMI-mask interaction, reset/finalize
rule, profile wiring, deterministic visibility, and real/protected/VM86
consumer proof. It must not use RTC events, a synthetic test flag, or port 61h
as a substitute for that contract.

## Source And Test Sweep

| Candidate or route | Observed owner/result | Disposition |
| --- | --- | --- |
| IBM PC/AT parity and I/O-channel check | IBM board signals, not RTC or PIC IRQ sources | No selected profile input or project-owned contract exists; transfer. |
| `cpu.data.flagNMI` and `ExecInt` | CPU delivery consumer; tests seed the pending state directly | Retain as NMI delivery proof only. It cannot originate a board signal. |
| CMOS ports 70h/71h | `machine.c` applies index-port bit 7 to `core_machine_set_nmi_mask`, then selects a low-seven-bit RTC register | Retain as mask-only adapter. T350 S3 proves it does not create an NMI producer. |
| RTC PF/AF/UF/IRQF | `rtc.c` owns event flags and one bound IRQ8 source | Retain as IRQ8-only. S3 proves Register C acknowledgement and source release. |
| PIT/PIC/KBC/FDC/HDC/VADP readiness and source callbacks | T346/T347/T348/T349 owners | No parity/I/O-channel NMI assertion route was found. Existing device IRQ paths remain unchanged. |
| Port 61h/PPI/speaker | No core provider or state owner | Separate corpus-gated TODO; it cannot be used to infer parity/NMI state. |
| VM profile/composition | CMOS adapter declares only the mask bit and IRQ8 configuration | No parity/I/O-check profile declaration or firmware contract exists; transfer. |

The static sweep searched tracked production, tests, build descriptions, and
task material for `nmi`, `parity`, `channel check`, `io check`, `flagNMI`,
`set_nmi_mask`, `nmi_mask`, and port-61 references. Every production NMI hit
was classified above. The tests that write `flagNMI` construct CPU-delivery
inputs deliberately; they do not claim a device signal path.

## Retained Deterministic-L3 Boundary

T346 remains the sole due-event/arbitration owner. A later selected NMI source
must enter that timeline before the CPU consumer observes it, and must release
or latch through one device owner across reset and finalize. S4 makes no clock,
CPU, PIC, or firmware change and therefore cannot create a second timing path.

## Verification

S4 re-runs the retained hardware NMI-delivery, CMOS/RTC, PIC lifecycle, and
T346 timeline/arbitration regressions through the current gate. The transfer is
accepted only because the TODO identifies the exact owner boundary, risk, and
admission condition rather than hiding the absence behind generic L3 wording.

Primary reference:

- [IBM PC/AT Technical Reference](https://ftp3.us.freebsd.org/pub/misc/bitsavers/pdf/ibm/pc/at/1502494_PC_AT_Technical_Reference_Mar84.pdf)
