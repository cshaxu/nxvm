# T386 S5: DeskPro Model-40 Storage Controller Contract

`M5:T386:S5:STORAGE-PRIMARY-CONTRACT:OK`

`M5:T386:S5:COMPAQ-HDC-ROUTE:OK`

`M5:T386:S5:PORT-WIRED-OR:OK`

`M5:T386:S5:COMPAQ-HDC-MACHINE:OK`

## Primary Contract And Classification

The source is Compaq, *DeskPro 386 Technical Reference Guide, Volume II*
(September 1986), chapter 7 and its fixed-disk command/register material. It
is consulted transiently as external research under the source policy; neither
its text, ROMs, media, hashes nor local paths are committed. T384 remains the
machine-selection authority.

| Selected fact | Classification | S5 disposition |
| --- | --- | --- |
| The Multipurpose Fixed Disk Controller combines a 765A diskette controller with fixed-disk address decoding, buffers and host connectors; the 40 MB drive has its own integrated controller. | Primary. | Reuse the existing shared 765/FDC/DMA2 owner for its established controller mechanics; introduce a distinct Compaq fixed-disk protocol instead of calling the ATA personality a Compaq or IBM MFM controller. |
| Diskette ports are `3F2h`, `3F4h`, `3F5h`, and `3F7h`; the diskette path uses DMA channel 2 and IRQ6. The selected 1.2 MB drive uses the documented 500 kbit/s rate. | Primary. | Existing FDC topology continues to own these registers, DMA2, IRQ6, reset, and the accepted 500-kbit/s cadence. S5 does not publish a DeskPro profile; S7 alone may bind the selected drive and firmware-visible policy. |
| Fixed-disk task-file registers occupy `1F0h`--`1F7h`, alternate status/device control is `3F6h`, and the drive-address read shares `3F7h`: its low seven bits are fixed-disk state while bit 7 is supplied by the diskette side. Reading fixed status acknowledges IRQ14; alternate status does not. | Primary. | `core_machine_hdc` gains an explicit `COMPAQ_WD_40MB` personality and `machine.c` composes the read-only `3F7h` register as FDC normal provider plus HDC wired-OR contribution. The normal provider remains unique; this is not generic port aliasing. |
| Normal read/write, verify, recalibrate, seek, initialize-drive-parameters and diagnostics belong to the controller command set; diagnostics report `01h` on success. | Primary. | The Compaq personality implements ordinary CHS PIO read/write (including the retry-bit variants), verify, recalibrate, seek, initialize and diagnostics; it rejects ATA Identify. IRQ14 completion, fixed-status acknowledgement, nIEN/SRST, invalid CHS and absent/read-only media all remain in the one HDC owner. |
| Read/Write Long, format-track, ECC/error-recovery detail, physical sector identity and drive-side service behaviour need media/drive representation not supplied by the current pure logical RAW-IMG contract. | Primary fact plus explicit current-media limitation. | Never coerce those opcodes to ordinary 512-byte I/O. S5 rejects them with the documented abort path and transfers their representation to TODO below. This is not a claim that an ATA error value models every Compaq diagnostic condition. |

The implementation also rejects LBA mode for the Compaq personality and
requires the Compaq sector-size/drive-head selection form before a command can
run. Default ATA remains a separately selected existing personality: its
accepted commands are deliberately not widened by the Compaq retry-bit forms.

## Owner And Composition Contract

`core_machine_hdc_config` now carries a protocol selector, a drive-address
read port and the pre-existing IRQ/LBA policy. `core_machine_configure_hdc()`
validates that the Compaq form has no LBA capability, is configured after the
selected FDC, and names that FDC direction port as the shared read address.
It registers ordinary HDC task-file ports atomically, then adds only the HDC
low-bit wired-OR reader at `3F7h`; failure rolls every registration back.

`core_machine_port` therefore still rejects an ordinary read-owner collision.
A wired-OR reader can only contribute to an existing normal read owner and
cannot replace it. This is the minimum shared-register mechanism required by
the primary contract; it adds no second bus, test-only API or product profile.

## Focused Proof

The Git-Bash/WinLibs GCC build ran:

```text
cmake -S . -B build/t386-s5-gitbash
cmake --build build/t386-s5-gitbash --target \
  core-machine-hdc-smoke core-machine-compaq-hdc-s5-smoke \
  core-machine-compaq-hdc-machine-s5-smoke -j4
ctest --test-dir build/t386-s5-gitbash -R \
  "core-machine-(hdc|compaq-hdc-s5|compaq-hdc-machine-s5)-smoke" \
  --output-on-failure
```

All three tests passed. The retained ATA smoke is the negative compatibility
control. The Compaq HDC smoke proves normal CHS transfer, low-seven-bit
contribution at `3F7h`, IRQ14 acknowledgement only on fixed-status read,
diagnostics success, ATA Identify rejection, Long-command rejection and SRST
reset. The machine smoke creates DMA then FDC, configures the Compaq HDC
through the real machine composition path, freezes/resets it, and proves the
shared `3F7h` read retains the HDC contribution. It emits the markers above.

## Full Gate And Test-Isolation Transfer

The final serial `ctest -L current-gate --parallel 1 --output-on-failure` replay
passed **255/255** after the S5 focused replay. Documentation governance also
passed. The prior `--parallel 4` replay had one failure in the pre-existing
`vm-product-console-memory-roundtrip-smoke`: it discovered the simultaneously
created lifecycle YAML and selected the wrong catalog ordinal. Its individual
replay passed, and source inspection confirms both tests create fixed profile
filenames in the common working directory before catalog discovery. This is a
current-gate test-isolation defect, not storage behaviour; it is explicitly
transferred to `TODO(High)`, **Parallel Console profile-smoke isolation**. S5
neither edits Console code nor treats the serial control as proof that the
parallel gate is now reliable.
## Similar-Issue Sweep

The S5 sweep covers `hdc`, HDC topology/configuration, `1F0h`--`1F7h`, `3F6h`,
`3F7h`, FDC direction/control, ATA/LBA, IRQ14, DMA2, and every existing
machine HDC caller. The result is one retained ATA personality and one
explicit Compaq personality under the same neutral HDC owner; IBM MFM remains
unselected and untouched. The generic FDC is not renamed or silently made
Compaq-specific, and no profile/catalog/firmware path is published.

## Transfer

S5 establishes the primary-backed storage-controller and shared-register
boundary, but does not claim a runnable Model 40, a selected FDC/firmware
binding, long/format/ECC physical-media behaviour, controller service time,
board waits, or L3. S6 retains the Compaq EGA personality. S7 alone may
compose accepted owners into the first internal Model-40 path. The following
DeskPro board/device closure retains service time, DMA/ISA availability and
all physical fixed-disk behaviour that the pure IMG medium cannot represent.