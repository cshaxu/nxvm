# T372 S4: Model-339 Primary-Fact Ledger

## Decision

The Type-3 primary-source sweep admits no new physical scalar for NXVM. IBM's
March 1986 PC/AT Technical Reference explicitly distinguishes 6- and 8-MHz
timing, while the locked Model 339 supplies the selected 8-MHz/Type-3/512-KB
topology. Intel and controller manuals supply clocked component semantics,
interfaces, and some internal units. None names the missing conversion from a
selected Model-339 board route to NXVM's completed-retirement scheduling
domain. A chip clock, a minimum interface bound, or an internal controller
unit therefore remains a source fact, not an implementation value.

This also corrects S3's wording: user-supplied guest media can be an isolated
local research input after its own authorization/review; it is never a
repository, default-build, or release asset. S4 does not use local media.

## Fact and domain matrix

| S1 residual group | Primary-source fact | Direct Model-339 implementation candidate? | Reason / receiver |
| --- | --- | --- | --- |
| CPU retirement, fetch and external occupancy | The 80286 hardware reference defines READY sampling and HOLD/HLDA sequencing; the CPU completes its current bus sequence before granting the bus. | No. | It does not select which Type-3 RAM/ROM/CGA/ISA route waits, a maximum grant latency, fetch occupancy, or a mapping into NXVM ticks. Retain the T372 physical-boundary receiver. |
| RAM, ROM, CGA, ISA and I/O READY/waits | IBM documents the PC/AT board and I/O-channel signal topology; the March 1986 revision distinguishes 8-MHz timing where applicable. | No. | Board topology and a CPU-rate distinction are not a per-route READY assertion or wait table. Retain the T372 physical-boundary receiver. |
| DMA, PIT, PIC and INTA | IBM establishes dual-DMA/dual-PIC/timer topology; Intel 8237A, 8254 and 8259A documents define request, counter and acknowledgement semantics. | No. | Neither source identifies Model-339 HRQ/HLDA/DACK/AEN/INTA wave spacing or maps controller clocks to NXVM's elapsed ticks. Retain the T372 physical-boundary receiver. |
| FDC, RTC, KBC and keyboard | uPD765A, MC146818 and UPI-41/42 documentation define controller-specific timers, oscillator/interface behavior and protocols. | No. | The field-upgrade TEAC drive and selected controller service routes lack a Type-3 board conversion and a project-domain mapping. Retain the T372 physical-boundary receiver. |
| Planar parity, NMI and CGA | IBM establishes the selected planar-parity route, NMI masking/topology and CGA connection. | No. | It does not yield NMI propagation, CGA dot/retrace/contention, or copied-frame cadence in the project time domain. Retain the T372 physical-boundary receiver. |
| Reset, cancellation and replay | IBM/controller documentation specifies reset-visible behavior; project tests prove reset/cancellation order. | No. | No source labels a Type-3 settle interval or turns deterministic callback order into a physical measurement. Retain the T372 physical-boundary receiver. |

## Source record

- [IBM PC/AT Technical Reference, March 1986](https://www.minuszerodegrees.net/manuals/IBM/IBM_5170_Technical_Reference_MAR86.pdf): Type-3-era board documentation; its preface states that 8-MHz timing differences are shown where applicable.
- [IBM PC/AT Technical Reference, September 1985](https://www.bitsavers.org/pdf/ibm/pc/at/6139362_PC_AT_Technical_Reference_Sep85.pdf): board, I/O, DMA, PIC, timer and controller topology.
- [Intel 80286 Hardware Reference](https://www.dosdays.co.uk/media/intel/210760-002_80286_Hardware_Reference_Manual_1987.pdf): local-bus READY and HOLD/HLDA interface behavior.
- [Intel 8237A data sheet](https://www.pcjs.org/documents/datasheets/intel/INTEL_8237A_DMA.pdf), Intel 8254/8259A manuals, [uPD765A data sheet](https://hxc2001.com/download/datasheet/floppy/thirdparty/FDC/NEC/uPD765A.pdf), MC146818 and Intel UPI-41/42 documentation: component-local clocks and protocol semantics only.

The cited documents are research sources, not imported implementation or
assets. Their facts confirm the existing T369--T371 non-admissions rather
than override them.

## Exact transfer

No code change is permitted from this ledger. The next T372 physical-boundary
evidence S must either obtain a Type-3 board-specific timing source or receive
an owner-authorized, isolated local-reference experiment that meets S2's
whole-tuple configuration and observation conditions. It must keep raw ROMs,
guest media, raw traces, and local paths out of the repository. **5170
Model-L3 remains open.**

## Similar-issue sweep

The sweep covers every S1 residual group, the Type-3 board/I/O material, 80286
READY/HOLD, DMA/PIT/PIC, FDC/RTC/KBC, parity/CGA, reset, the prior physical-time
non-admissions, and S2/S3's reference exclusions. No direct Model-339
project-domain timing mapping was found; no scalar, source, asset, runtime
route, ABI, or test changed.
