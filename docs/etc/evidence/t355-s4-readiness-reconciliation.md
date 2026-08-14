# T355 S4: Windows readiness-map reconciliation

## Probe and provenance disposition

| Retained probe | S4 disposition | Boundary retained |
| --- | --- | --- |
| `vm-windows31-checkpoint` | Unexecuted and outside current-gate. | Host thread/input plus `Sleep`; its DOS/BDA observation is not Windows boot evidence. |
| `vm-windows31-setup-probe` | Unexecuted and outside current-gate. | Host thread/input and long host waits cannot specify guest time or device latency. |
| `vm-windows31-hdd-admission-probe` | S3 executed once through the explicit S2 BYOB target and produced the stable HDD/INT13 checkpoint. | Reads the supplied HDD and uses a deleted temporary synthetic FDD; it is not Setup or a boot claim. |
| `vm-windows31-int13-trace-probe` | Unexecuted and outside current-gate. | Caller-supplied FDD/HDD and bounded trace remain a future separately admitted checkpoint. |

The CMake sweep confirms that all four targets remain outside every
`PROJECT_CURRENT_*_SMOKE_TARGETS` list.  `ctest -N -L current-gate -R windows31`
finds zero tests.  S2's `PROJECT_WINDOWS31_CHECKPOINT_HDD_IMAGE` defaults to
empty and is only consumed by the explicit `run-windows31-hdd-checkpoint`
target.  No local path, input bytes, or new media hash is tracked.

## Accepted prerequisites and remaining transfers

| Boundary | Reconciled result | Receiver / limitation |
| --- | --- | --- |
| ATA/FDC readiness | T347 pending-command/data service and T354 deterministic transaction visibility are accepted; S3's two ATA reads consume that baseline successfully. | General wait states, physical bus ownership, and unselected storage timing remain [Timing-Fidelity TODO](../../states/TODO.md#timing-fidelity-debt). |
| Selected L3 | Deterministic timeline and selected CPU/DMA/device ordering are accepted. | This remains event-and-bus L3, never a cycle/pin waveform claim. |
| Video and input | Selected VADP and KBC/AUX mechanisms have no Windows-driver corpus here. | Broader EGA/VGA/VBE and advanced AUX remain [Hardware And Compatibility Debt](../../states/TODO.md#hardware-and-compatibility-debt). |
| PC/AT optional signals | No S3 probe names PPI/speaker or a parity/I/O-channel NMI producer. | Keep the existing corpus-gated PPI/speaker and NMI TODO entries. |
| Product compatibility | No Setup, boot, Standard Mode, Enhanced Mode, driver, or general Windows probe executed. | Any such work needs a new owner-approved Queue candidate and a bounded BYOB checkpoint. |

The S4 sweep found one stale supporting claim: the current capability baseline
still described selected FDC/ATA command-service and readiness ordering as
post-T346 work.  It is corrected to reflect T347/T354 while preserving its
broader deferred timing boundary.

## T355 closure conclusion

T355 closes as a readiness **map**, not a Windows implementation milestone.
The one stable checkpoint verifies the retained BIOS/ATA path is a valid
consumer of the selected device/L3 baseline.  Every other Windows probe,
device breadth item, physical timing requirement, and compatibility outcome is
either explicitly unexecuted or transferred above; none is silently accepted.
