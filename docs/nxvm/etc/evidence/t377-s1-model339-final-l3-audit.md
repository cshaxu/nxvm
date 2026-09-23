# T377 S1: IBM PC/AT 5170 Model-339 Final L3 Audit

`M5:T377:S1:MODEL339-FINAL-L3-AUDIT:OK`

## Decision

**Not ready: the frozen IBM PC/AT 5170 Model 339/Type 3 baseline cannot yet
be accepted as L3.** The focused model evidence is sound, but the current full
gate proves that the selected native keyboard-to-8042 production path bypasses
its required transport ingress operation. A passing controller smoke does not
compensate for that production-boundary failure. The selected configuration
remains 8 MHz 80286, Rev.3 BIOS slot, 512 KB planar memory, CGA, AT keyboard
and no fixed disk. Its optional 1.44 MB route is an aftermarket TEAC
FD-235HF-A540 upgrade, not an original IBM factory claim.

L3 here is deterministic event-and-bus timing, not physical cycle-exact or
waveform emulation.  Consequently a source-exhausted physical boundary is
accepted only where it is explicit, has no invented scalar, and cannot change
the selected logical state, reset/cancellation or deterministic event order.
It is not evidence for a broader PC/AT, clone, MFM/ATA or Windows result.

## Independent Requirement-To-Evidence Matrix

| Selected requirement | Functional and timing evidence | Audit disposition |
| --- | --- | --- |
| Exact machine identity and supported surface | T366 S2 profile lock; Model-339 composition smoke proves the 8 MHz/512 KB/CGA/Rev.3/no-HDD descriptor and rejects HDD configuration. | Accepted; MFM/ST-506, ATA/HDC and I/O-check NMI remain outside this baseline. |
| 80286 instruction retirement and CPU time | T368 S7 accepted the source-labelled successful-retirement ledger; T375 S25 retains prefetch/READY/HOLD as no-synthetic-scalar boundaries. | Accepted at L3: retirement publishes the sole deterministic CPU time; physical occupancy is unclaimed. |
| RAM, ROM, port and ISA transactions | T369/T372 transaction ownership and T375 S25 prove one transaction owner and no selected READY/RAM/ROM/ISA conversion. | Accepted at L3: validation/publication and order are deterministic; no electrical wait/phase claim is made. |
| DMA, PIC, PIT and RTC/CMOS | T374 S15 functional closure; T375 S2, S16 and S25 bind rational RTC/PIT clocks, one logical DMA/PIC lifecycle and defined arbitration order. | Accepted; HRQ/HLDA/DACK/AEN/INTA propagation and pin spacing remain explicit physical exclusions. |
| 8042, 101-key keyboard and IRQ1 | T374 S17--S18 functional/production-input boundary; T375 S22--S23 bind IBM typematic nominal/range and F3/default cadence. | **Not accepted:** the full gate proves Win32 keyboard events bypass the production transport ingress, so the declared native input boundary is not enforced. |
| 8272A, FDC, DMA2/IRQ6 and factory floppy route | T374 S4--S10 functional closure; T375 S19--S21/S24 source-gated byte/seek/NDMA timing; T376 S2--S4/S6 raw sidecar, Deleted Data, Control Mark and Scan proof. | Accepted for ordinary raw IMG plus optional sidecar.  Physical CHRN duplication, encoded layout/CRC/flux, index/search phase, motor upper-bound completion and controller-to-grant electrical phase are explicitly unsupported, not silently emulated. |
| Aftermarket 1.44 MB route | T373 S2/S3, T375 S19 and T376 evidence identify the TEAC 500-kbit/s/300-RPM route and raw-sidecar media boundary. | Accepted as an aftermarket compatibility condition only; no factory configuration or IBM-controller substitution is inferred. |
| Digital CGA | T374 S11--S16 functional closure; T375 S8/S11--S15 binds CRTC/status/defaults and a qualified 86Box-derived VADP cadence only to the project clock domain. | Accepted at L3: logical raster/status/reset ordering is deterministic.  Adapter revision, ISA contention, snow, monitor and waveform claims remain excluded. |
| NMI, reset, cancellation and cross-device order | T365 non-admission, T372 S8, T375 S4/S6/S17/S18/S25 and focused replays establish selected parity/reset/cancel and ordered virtual-time publication. | Accepted; unselected I/O-check NMI and physical reset/pin settle time are excluded. |

No row relies on a ROM, guest image, local path, reference-emulator runtime or
third-party implementation.  86Box is used only as the already-qualified,
secondary CGA cadence source; it is not a Model-339 authority.

## Replay

The current rebuilt focused targets passed on the audit worktree:

```text
M5:T366:S5:MODEL339-COMPOSITION:OK
M5:T375:S2:MODEL339-CLOCK-CONTRACT:OK
M5:T375:S13:MODEL339-CGA-REFERENCE-CONTRACT:OK
M5:T375:S22:MODEL339-TYPEMATIC:OK
M5:T375:S23:KBC-F3-CADENCE:OK
M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK
M5:T375:S15:MODEL339-REV3-CGA-DEFAULTS:OK
M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK
M5:T376:S2:RAW-IMG-SIDECAR-LIFECYCLE:OK
M5:T376:S6:MALFORMED-SIDECAR:OK
M5:T376:S3:8272A-DELETED-DATA:OK
M5:T376:S4:8272A-SCAN:OK
```

The FDC replay also retains the existing atomic media, FDC service, DMA
cadence, seek cadence and non-DMA cadence markers.  This is a source/test
replay of the selected contract, not a guest-boot or hardware-measurement
claim.

## Boundary And Receiver Audit

The reference-exhausted physical rows do not need invented implementation:
they are outside the project L3 definition and remain permanently visible
through this audit.  A future request for physical timing, flux media,
MFM/ST-506, ATA/HDC, I/O-check NMI, broader CGA fidelity or a different AT
configuration requires its own capability selection and admission.  The
later current-product device closure still owns retained EGA/VGA, ATA/HDC,
AUX and other exposed non-Model-339 capability; no Model-339 result upgrades
them.

## Current-Gate Disposition

The focused Model-339 and FDC replay above passed after rebuilding its targets.
The configured full `current-gates-gcc` run did not complete cleanly because
three global checks fail:

| Failing global check | Reported condition | T377 disposition |
| --- | --- | --- |
| `verify-keyboard-ingress-boundary` | Win32 keyboard events bypass the transport ingress operation. | Selected Model-339 native-input defect. The new **Model-339 preclosure gate repair** candidate is the earliest shared owner. |
| `verify-aux-mouse-boundary` | Win32 mouse events bypass the platform transport. | The same candidate repairs the shared production-input boundary while retaining the Model-339 IRQ1-only selection. |
| `verify-dependency-dag` | `src/vm/platform/virtual_time.h|vm/composition` is a forbidden edge. | The same candidate owns restoring the published composition boundary before an audit can claim a green current gate. |

The documentation-governance failure first reported in this run was caused by
nine retained recent task-level rows in `CURRENT.md`; S1 corrects that status
retention error before its final governance replay. The three code-gate
failures remain visible transfers and prevent T377 from claiming an all-green
repository gate. The repair candidate is immediately followed by a fresh
Model-339 L3 re-audit; neither task may reuse this negative decision as an
acceptance.

## Audit Result

The audit confirms the CPU, board, FDC/raw-IMG and CGA records, and retains all
physical boundaries truthfully. It nevertheless returns **not ready** because
the selected keyboard production path fails the full current gate. The
immediate repair candidate must restore the native input and dependency gates,
then the fresh Model-339 re-audit must replay the entire matrix and make the
only subsequent ready/not-ready decision.
