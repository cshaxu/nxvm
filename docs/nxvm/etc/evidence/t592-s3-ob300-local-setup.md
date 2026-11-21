# T592 S3 — Restore the external OmniBook media

The published files are byte-identical to the S2 verification inputs. In the
repository deployment directory, both executables returned exit code 1 before
composing the monitor. The external System Card and C image expected by the
adjacent INI layout were absent. The S2 native boot evidence used a populated
disposable layout and did not establish that the owner's local published
directory contained its runtime inputs.

The offline NXVM tool restores the owner-supplied C ZIP to an active
FAT16 partition on a 32-cylinder raw disk with 16 heads and 63 sectors per
track. The tool is independent of build, Product startup and ATA execution.
It reads the backup, preserves the 8.3 hierarchy and contents, and creates a
new output exclusively. It does not alter the archive or install the System
Card. The [external input guide](../../../assets/myob300/README.md) names the
matching archives, geometry, startup and access-mode boundaries.

Independent readback checked the signed MBR/BPB, the active partition,
both identical FATs, allocated cluster chains, directory parent links,
all 13 directories and all 43 files byte-for-byte. The System 1.0 ABA backup
SHA-256 stayed `4d3f6fbadfabfb056ad42d1dc82007f9d7ca57026fffb648903ae6135bf5715d`.
The restored raw image SHA-256 is
`1b146fb6d17c3025a571699217715f21d80dcd4e23005319595ef5cb76d2b325`.
A second restore to the same path refused to overwrite it, and a backup with
a traversal entry was rejected before creating an output.

The x64 and x86 Release copies launched as ordinary processes in a separate
local asset layout with the unmodified published INI. A foreign working
directory did not affect configuration lookup. Each ran `info`, reported
the 16,515,072-byte disk connected, and accepted `exit`. The source archives
and the sibling asset checkout were unchanged.

With the exact external System Card present, both binaries rejected missing,
511-byte and zero-byte C images and an invalid INI access mode before the
monitor appeared. The valid source image remained byte-identical. These
start-time cases did not exercise a storage fault after guest startup.

The local setup verification reaches the monitor with attached media. The
original, unmodified C backup proceeds into a separate ROM Windows path after
`start`. The original-C Windows boot, Window/RDP interaction and guest power
qualification remain outside this launch result.

## Original DOS write rejection after startup

The x64 and x86 replays used the original ROM and a disposable variant of
the restored C image. The variant kept the original `CONFIG.SYS` and
replaced the `AUTOEXEC.BAT` Windows launch with a DOS file creation
attempt. The Machine attached that disk in `readonly` mode.

After POST and the original DOS startup, both guests reached the write
attempt and displayed `Seek error writing drive C`. A neutral key event
selected Fail at the DOS prompt. DOS reported `File creation error` and
displayed the subsequent batch marker, confirming recovery to guest
command execution. Both runs completed with the disk and backup unchanged.

| Item | SHA-256 |
| --- | --- |
| Disposable readonly C image | `020de47933a0d1845f827889ea77530b53c055eb42b38f9bfe25a27ba94d5588` |
| Owner C archive | `4d3f6fbadfabfb056ad42d1dc82007f9d7ca57026fffb648903ae6135bf5715d` |
| x64 guest output | `2de405256ce38c7dcfc4944a3db23ed98cea628e98addb92edd14a76b8fc67bf` |
| x86 guest output | `dad05be11bd3a8abed4c2c7c17ab710460c70c4eeaa8779eff2fc99aa81caeb4` |

These instrumented runs establish the original DOS and ATA read-only
path. The earlier guest write/readback verification covers writable
media separately. Hot removal and backing-file loss were not tested.

## Guest power qualification boundary

The original *HP OmniBook Operating Guide* (`OmnibookOperatingGuide.pdf`,
pages 1-6 and 9-8) describes an On/Off key and distinct power modes.
Advanced mode preserves the working session on ordinary power-off.
Standard and Off discard it and boot again at the next power-on.
The HP Windows package includes `OBPOWER.DLL` and the `OBMGM` management
program. The current board models the SCAMP idle request clearing on
PIC wake and the SMI memory route. The physical On/Off key sequence,
Advanced suspend and resume, and the Standard/Off transition remain
open product evidence. Monitor pause and resume are already qualified
as a separate lifecycle path.

## Original APM BIOS query

The HP guide names APM and the Advanced mode, and the original System
Card file inventory ties the Windows Power panel to the OEM manager.
The Win16 OBPOWER module imports OBMGM. The original DOS OBMGM module
directly calls INT 15h with AX=530Ah and BX=0001h. This identifies the
guest interface to probe without intercepting it in the emulator.

A disposable startup variant kept the original CONFIG.SYS, HP resident
programs and DOS. After SHARE it ran a small project-authored DOS query
instead of launching Windows. C was mounted read-only and both runs
completed with the same disk hash. The x64 and x86 logs agree:

| Original ROM request | Observed return |
| --- | --- |
| Installation check, 5300h | Carry clear, AX=0101h, BX=504Dh, CX=0000h |
| Real-mode connect, 5301h | Carry set, AH=08h |
| System power status, 530Ah | Carry clear, BX=0000h, CX=013Ch |
| Next event, 530Bh | Carry set, AH=80h |

The installation signature advertises APM 1.1. The no-event response
matches the [Intel and Microsoft APM interface specification](https://intel-vintage-developer.eu5.org/IAL/POWERMGM/APMV12.PDF).
The connect result is kept raw because the later specification does
not list AH=08h as a real-mode connection result. The diagnostic also
issued a disconnect before returning to DOS. This run establishes
working stock BIOS query and event polling under idle conditions, not
a suspend request, switch event, or resume.

| Item | SHA-256 |
| --- | --- |
| Query program | `d49b652b141d6168d68687581f7ceda219295aa694187429bacd7543e8c9b714` |
| Disposable C image | `1fd14d4126d1ad5cf85caa711db261317eb4f4ab1ad62cb59767fec3b6818ed0` |
| x64 guest output | `133eaf08b4d85b6b5ea6318dc830edf322e3a8d5e377c61723ae16c8b1c1276f` |
| x86 guest output | `caa26ad48a6ebb3395cc65f2dce8188bf9558a35f586b6db17a4e83f07069234` |
