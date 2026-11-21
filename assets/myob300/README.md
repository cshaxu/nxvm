# MyOB300 external inputs

Build and publish both Release architectures with `tools/build.cmd myob300`.
The executables are named `myob300_0_5_<revision>_x64.exe` and
`myob300_0_5_<revision>_x86.exe`. Keep
`myob300.ini` beside the executable. The launcher resolves the INI,
the fixed System Card path and the INI media paths from that
directory, including when a shortcut has a different working
directory.

Place the raw common-memory image from your own OmniBook 300
System 1.0 ABA card at
`../../../nxvm-assets/hp-omnibook300/rom/system10-aba-common.bin`
relative to the executable. The file has 16,777,216 bytes and SHA-256
`b98001a30b79f126483f823fd4d7a5ce91a973a5ab8fb35e3d76043b74a8e099`.
Extract the raw entry from the archive before launching. MyOB300
checks the full digest of the loaded bytes before creating its worker.
Keep protected card bytes outside this Git repository.

The sample INI points to
`../../../nxvm-assets/media-nxvm/ob300/ob300-c.img`. Create that directory
in your external asset location, then run this from the repository root
with Python 3.9 or later and your own System 1.0 ABA C-drive ZIP:

```cmd
python tools/nxvm/prepare_ob300_disk.py <C-drive-backup.zip> ../nxvm-assets/media-nxvm/ob300/ob300-c.img
```

The tool restores the ZIP hierarchy into an active FAT16 partition on a raw
ATA disk with 32 cylinders, 16 heads and 63 sectors per track. It preserves
the original file contents and refuses to overwrite an existing image.
The System Card supplies DOS
boot code. The default `overlay` mode keeps writes off the C image.
Use `direct` on a disposable copy when you intend to persist guest writes.
While MyOB300 has the C image mounted, Windows denies other writes and
file removal. Close other tools or emulator instances that have an
exclusive handle on the image before launching. An unavailable C image
can make the process exit during startup.

The current model selects 2 MiB RAM. Enter `start` at the monitor. On first
boot, the original ROM may report invalid configuration information and
ask for F1. Press F1 in the guest display to continue. When done, use the
monitor's `exit` to save the guest RTC and CMOS to `myob300.cmos` beside
the EXE. The next launch loads that file, independently of the C disk's
access mode. Keep the directory writable and run one instance per directory.
If the file is damaged, move it aside while MyOB300 is closed to return to
the first-boot configuration. Forced termination can lose the latest state.
The original C backup starts Windows from the System Card. Both x86 and x64
have drawn Program Manager in a replay. The owner confirmed menu navigation,
exit to DOS and usable mouse input. For a DOS-only check, use a disposable image
with the original CONFIG.SYS and a copy of AUTOEXEC.BAT with Windows
autostart removed.

In the guest display, press and release F13 to request firmware suspend.
After the guest enters suspend, press and release F13 again to wake it.
F13 also wakes a guest that entered HP firmware APM suspend on its own.
Leave the emulator running so the guest firmware can observe the key.
The power-key model was verified with original ROM DOS.
