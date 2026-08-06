# M5 T242: FDC/FDD Corpus-Driven Extension

## S1: Feature Matrix And Corpus Admission

**Status:** complete.

T242 extends the existing VM-owned FDC/FDD only when a guest-visible corpus
names a concrete deficit. It does not reinterpret a bootable DOS image as
general 8272A compatibility. The owner and route remain:

```text
guest 03F2h/03F4h/03F5h/03F7h -> vm_machine_fdc -> DMA2/core RAM
  -> vm_machine_fdd -> result -> IRQ6/core PIC -> guest IVT
```

`vm/machine/fdc` remains the only command, phase, DRQ, and IRQ6 owner;
`vm/machine/fdd` remains the only media backend owner. Core keeps DMA2, guest
RAM, PIC, and elapsed-tick scheduling. ROM and DOS tools are ordinary port
clients. No host file, BIOS INT 13h, or profile shortcut is admitted.

### Current Feature Matrix

| Area | Current bounded state | T242 disposition |
| --- | --- | --- |
| Drive/media | One fixed 1.44 MB backend, drive 0 only; insert/remove generation and DIR change acknowledgement | Multi-drive requires a profile topology admission; defer. |
| Commands | `SPECIFY`, sense, recalibrate, seek, version, read-ID, read/write-data, format-track | Retain; no duplicate implementation. |
| Read-track | Explicit invalid-command result, no DMA/IRQ transfer | **Admit for S2 investigation:** one drive-0, 80x2x18, 512-byte DMA2 slice only. |
| Deleted/scan/configure commands | Explicit invalid-command result | Defer: no current DOS corpus and independent result semantics. |
| Motor/rotation | DOR motor bits observable; no spin-up/rotation delay | Defer to a profile-clock/timing admission; no host sleep. |
| Formats/rates | Fixed 1.44 MB geometry; CCR 500/250 admission | Defer extra media geometry/rates until a supplied corpus requires one. |
| Errors/results | Existing no-media, write-protect, CHS/EOT, rate, result FIFO, SENSE-INTERRUPT | Retain and cover in matrix. |

### Minimal Corpus And S2 Boundary

The S1 owner probe drives `READ TRACK` (`42h`) against created, drive-0 media
after enabling DOR. The current controller returns its explicit invalid-command
result (`80h`) without starting a DMA2 request or asserting IRQ6. Its marker is
`M5:T242:S1:FDC:PORT:OK`; this records a reproducible failing capability rather
than treating boot success as evidence.

S2 may implement only one fixed-geometry `READ TRACK` transfer: current
drive 0, head/cylinder selection, 18 sectors of 512 bytes, existing DMA2
provider callbacks, result FIFO, and IRQ6/SENSE-INTERRUPT completion. It must
define the command's exact parameter and result behavior before code changes.
A later S3 will add an owner-built `FDC242.COM` to a temporary clone of the
owner-supplied DOS FDD image. That guest tool will configure the normal DMA2
and FDC ports, issue `READ TRACK`, and verify the same controller/DMA/IRQ6
route. Protected media is never modified or committed.

### Rules, Evidence, And Stop Conditions

- S1 probe: `vm-fdc-t242-corpus-port-smoke`; it observes only VM FDC ports,
  result phase, DMA request state through existing bindings, and IRQ6/PIC.
- S2/S3 retained matrix: FDC port/state-machine, DMA2, IRQ6, FDD/HDD boot,
  DOS prompt, keyboard, CGA/EGA, Console/debugger, and `current-gates-gcc`.
- Similar-issue sweep applies to every read-track decode, command-length,
  DMA transfer, result, and deferred-command path in `vm/machine/fdc*`,
  focused FDC tests, CMake registration, and task records.

Stop before S2 if a fixed read-track slice requires raw RAM/DMA/PIC access,
a second FDD/FDC state machine, a VM-side transfer loop, host-clock timing,
BIOS/INT 13h bypass, a second drive backend, or an unbounded command family.
Those outcomes remain a deferred profile, timing, or separate FDC admission.

### S1 Evidence And Exit

`vm-fdc-t242-corpus-port-smoke` emits `M5:T242:S1:FDC:PORT:OK`. It creates
the existing fixed drive-0 backend, enables DOR, issues `42h`, then proves the
one-byte `80h` invalid-command result, end of result phase, and absence of
IRQ6 delivery. No device source changes in S1.

`cmake --build --preset current-gates-gcc --parallel 4` passed 34 static and
ownership gates plus 79/79 current CTest smokes, including retained FDC/DMA2,
FDD/HDD boot, DOS prompt, keyboard, CGA/EGA, Console, and debugger coverage.
S1 is design/test-baseline work only, so it creates no product artifact. S2 is
bounded to the admitted drive-0 `READ TRACK` slice above; no other matrix row
is implicitly authorized.

## S2: Owner-Local Read-Track Slice

**Status:** complete.

S2 admits exactly the MFM `READ TRACK` command byte `42h`. Its complete
nine-byte command is `42h, DH=00h, C, H, R=01h, N=02h, EOT=12h, GPL, DTL`.
The only accepted transfer is drive 0, either valid fixed-image cylinder,
head 0 or 1, sector 1 through sector 18, 512-byte sectors, and DMA mode set by
the existing `SPECIFY` command. `MT`, `SK`, non-MFM opcode variants, nonzero
drive, a non-1 start sector, an EOT other than 18, non-512-byte `N`, unsupported
CCR, non-DMA mode, and unavailable media do not widen the slice; they retain
the controller's normal invalid-command or no-data result semantics.

The FDC owns command decoding, transfer position, result FIFO, DRQ and IRQ6.
It starts the existing channel-2 provider request and the core scheduler moves
each byte into core-owned RAM. After 9,216 bytes, the existing completion path
returns `ST0=00h, ST1=00h, ST2=00h, C, H, R=13h, N=02h`, asserts IRQ6, and the
ordinary `SENSE INTERRUPT` command releases that source. No FDC code reads or
writes RAM directly.

`vm-fdc-t242-corpus-port-smoke` creates a temporary boot-loop FDD image so the
core scheduler remains active, fills its first track through the FDD owner API,
programs ordinary DMA2 ports, and submits `SPECIFY` plus `42h`. It proves all
9,216 guest-RAM bytes, the seven-byte normal result, assertion then
`SENSE INTERRUPT` release of the FDC IRQ6 source, and no-data rejection of the
otherwise identical non-MFM `02h` command. The marker is
`M5:T242:S2:FDC:READ-TRACK:OK`.

## S3: DOS Corpus And Closure

**Status:** active.
