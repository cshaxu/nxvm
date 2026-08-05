# M5 T233: ATA PIO Feature Matrix

## S1: Contract And Probe Design

**Status:** Complete.

T233 extends the one VM-session-owned ATA controller without adding a second
machine, a second image owner, IDE DMA, host timing, or a firmware/image
shortcut. The session HDC remains the sole owner of primary task-file state,
PIO buffer, command phase, error/status state, and IRQ14 source; `t_hdd`
remains its private image backend. Core owns only the port bus and PIC.

### Admitted Matrix

| Capability | T233 behavior |
| --- | --- |
| Primary master | One profile-declared 1F0h--1F7h / 3F6h channel with IRQ14. CHS PIO remains supported. |
| LBA28 | `READ SECTORS`/`WRITE SECTORS` decode LBA from 1F3h--1F6h when bit 6 of device/head is set; bounds are checked against backend sectors. Count zero means 256 sectors. |
| Slave select | The selected slave has no backend. A command terminates as DRDY+ERR/ABRT through the same controller, never aliases the master image. |
| Secondary channel | The default profile declares no secondary controller, ports, IRQ, image, or synthetic response. It remains unclaimed rather than silently reusing primary state. |
| SRST | Writing SRST high puts the primary controller in BSY with no IRQ. Releasing it resets task-file/phase/error state to DRDY+DSC with no IRQ. Commands while asserted do not start a transfer. |
| Status/IRQ | Command completion is deterministic at the command/data boundary: alternate status is observational; status read acknowledges/deasserts IRQ14. New commands clear any previous source before establishing their own result. |
| Error | Absent media and unsupported/slave/out-of-range commands have a defined error byte and DRDY+ERR state. |

No ATA DMA, ATAPI, secondary controller, slave backend, LBA48, cache, host
wall-clock latency, or generalized timing model is admitted. A file image is
only a bounded backend; every guest read/write still travels through task-file,
PIO data, status, and IRQ state.

### Evidence

The port smoke will verify LBA read/write and task-file progression, `count=0`
as a bounded 256-sector command, absent-slave ABRT without master mutation,
secondary-profile absence, SRST assert/release, status versus alternate-status
IRQ acknowledgement, no-media and out-of-range errors. The retained HDD boot,
FDD boot, DOS prompt, Console/debugger, and current GCC/CTest matrix remain
mandatory. A source gate will reject DMA, profile-firmware image access, and
any HDC direct CPU/PIC state mutation.

Stop if support needs another `vm_session` image owner, direct ROM access to
the backend, a VM-side execution loop, host-clock/device delay, a core-to-VM
dependency, or a change to retained NXVM startup/Console behavior.

## S2: Owned Implementation

**Status:** Complete.

`vm_machine_hdc` now decodes LBA28 from its own task-file registers and uses
the same VM-owned PIO buffer, status/error state, and IRQ14 source as CHS.
It accepts a zero sector-count as 256 sectors, keeps a 16-bit remaining count,
and advances the visible LBA task file between sectors. Device/head slave
selection fails as ABRT before backend access. SRST is an explicit port-edge
state: assert is BSY/no IRQ; release resets to DRDY+DSC/no IRQ. A new command
clears the prior source before it creates a new result. Profile metadata states
that LBA28 is present while slave and secondary controller topology are absent.

## S3: Verification And Closure

**Status:** Active.
