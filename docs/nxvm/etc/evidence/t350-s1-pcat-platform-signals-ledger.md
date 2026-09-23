# T350 S1: PC/AT Platform-Signals Ledger

## Primary contracts and method

This ledger uses the Intel 8254 programmable-interval-timer contract, the
[Motorola MC146818A data sheet](https://www.ardent-tool.com/datasheets/Motorola_MC146818A.pdf),
and the [IBM PC/AT Technical Reference](https://ftp3.us.freebsd.org/pub/misc/bitsavers/pdf/ibm/pc/at/1502494_PC_AT_Technical_Reference_Mar84.pdf).
The IBM reference assigns PIT output 0 to IRQ0, RTC to IRQ8, and parity or
I/O-channel check to NMI; it distinguishes that NMI source from the CMOS
index-port mask. No reference implementation source, firmware, or guest media
was imported or executed.

The audit swept `pit.c`, `rtc.c`, `machine.c`, VM profile/composition CMOS
wiring, PIC source lifecycle, timeline callbacks, and every focused PIT,
RTC/CMOS, NMI-delivery, and L3 regression. "Accepted" means the named bounded
mechanism exists with proof; it does not claim whole-chip or whole-board
parity.

## Manual-to-owner-to-proof ledger

| Signal / manual requirement | Current owner and route | Current proof / status | Receiver and bounded disposition |
| --- | --- | --- | --- |
| 8254 counters 0--2, binary/BCD count, modes, readback/latch, gate, output | `core/machine/pit.c`; ports 40h--43h; counter-local output and gate state | PIT divider/readback/waveform smoke covers the retained deterministic subset. | **S2** must audit every retained 8254 form against the manual and reconcile lifecycle/output ownership; unsupported electrical timing remains later L3. |
| PIT channel 0 to IRQ0 | `machine.c` binds one PIT output callback to a T349 PIC source; arbitration is `DMA -> PIT -> PIC` | PIT waveform, T346 arbitration, and T349 S4 lifecycle proof. | **S2** proves pulse/edge consumption, reset/finalize, equal-tick visibility, and CPU-facing IRQ0 without a second timer owner. |
| PIT channel 2, port 61h/PPI, speaker gate/data | No port-61 provider, PPI state, speaker owner, or focused proof exists. `pit.c` can expose channel output but machine composition only binds channel 0. | Existing TODO accurately records the absence. | **Transfer:** retain `TODO(Medium) PC/AT speaker/PPI 61h`; later port-topology/peripheral admission needs a DOS/Windows corpus, IBM port-61 contract, deterministic event owner, reset rule, and copied host-audio boundary. S2 must not invent it. |
| MC146818 calendar, BCD/binary, 12/24 hour, alarm, update, periodic rate/UIP | `core/machine/rtc.c` owns neutral calendar/register state; readiness tick advances RTC after FDC/HDC and before next PIC arbitration | RTC, CMOS port, authority, timer-firmware, and T346 timeline proofs cover the retained path. | **S3** audits the complete admitted register/event matrix, exact register-C acknowledgement, event coalescing, reset/finalize, and IRQ8 visibility. |
| RTC IRQ8 and register-C acknowledgement | RTC holds PF/AF/UF/IRQF, asserts one T349 PIC source when enabled, and deasserts/clears C on read | `core-machine-rtc-smoke`, CMOS port, T349 lifecycle and PIC proofs | **S3** owns event-to-IRQ8 publication order and no-publication/reset boundaries; no RTC-to-NMI behavior is allowed. |
| CMOS ports 70h/71h and NMI mask | `machine.c` installs the configured port pair; index write applies configured bit 7 through `core_machine_set_nmi_mask`, then selects lower seven-bit RTC index | `vm-cmos-rtc-port-smoke`, RTC authority/configuration proof, existing NMI delivery smokes | **S3** owns PC/AT adapter mask/index/read/write composition and ensures it stays separate from any NMI producer. |
| PC/AT parity or I/O-channel check NMI | CPU can deliver a pending NMI (`flagNMI`) and core exposes only mask set/get; no parity/I/O-channel source, latch, port-61 status, profile declaration, timeline assertion, or reset owner exists | Hardware/NMI tests directly seed CPU pending state; they are delivery evidence, not device-production evidence. | **S4** may admit one IBM PC/AT source only after defining its mask/assert/deassert/reset and deterministic visibility. If a selected profile/corpus cannot justify it, retain the precise NMI TODO; do not synthesize RTC NMI. |
| Physical waveform, host audio, oscillator phase, sampled INTA / spurious relationship | No pin/waveform or host audio model; deterministic timeline owns only due-event order | T346/T349 explicitly exclude these electrical boundaries | **Transfer:** queued L3 bus-timing task owns physical bus/INTA visibility; speaker host output remains the port-topology TODO. |

## State and lifecycle graph

```text
PIT ch0 output -> bound PIC IRQ0 source -> PIC refresh -> CPU IRQ selection
RTC event flags -> RTC IRQ8 source -> slave PIC -> master IR2 -> CPU IRQ selection
CMOS 70h bit7 -> core NMI mask only
parity / I/O-channel check -> [absent selected producer] -> CPU NMI pending
PIT ch2 / port 61h -> [absent selected peripheral owner]
```

Current deterministic ordering is deliberate: the arbitration callback
advances DMA then PIT then PIC; the readiness callback advances FDC/HDC then
RTC. A new RTC source becomes eligible for PIC arbitration on the following
due tick. KBC/VADP run later at the same logical time boundary. S2--S4 must
retain that ordering or change it only through a separately justified timeline
contract.

## S2--S4 allocation and exit decision

| Receiver | Finite work | Required proof | No-go boundary |
| --- | --- | --- | --- |
| S2 | 8254 retained subset and IRQ0 lifecycle | mode/gate/latch/readback/BCD retained contract, channel-0 callback edge/reset/finalize, deterministic IRQ0 order | No port-61/PPI/speaker or host audio without its separate corpus. |
| S3 | MC146818 and PC/AT CMOS adapter | calendar/event/IRQ8/register-C, port 70h bit7 mask versus index, reset/finalize and timeline order | No host time, firmware rewrite, or RTC NMI. |
| S4 | One selected PC/AT NMI producer or an evidence-backed non-admission | IBM wiring, source latch/mask/assert/deassert/reset, real/protected/VM86 delivery through existing CPU path | No generic CPU delivery redesign, arbitrary board source, or port-61 implementation by implication. |

This S1 closes only the audit allocation. It deliberately retains the explicit
speaker/PPI and NMI TODO entries until their designated receiver either proves
the required hardware contract or records a truthful non-admission.
