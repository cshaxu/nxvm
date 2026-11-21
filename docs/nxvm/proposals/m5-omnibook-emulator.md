# HP OmniBook research and OB300 implementation plan

Owner requested family queue registration on 2026-11-11. On 2026-11-19 the
owner prioritized OmniBook 300 and requested several bounded T candidates at
the queue head. This file now preserves the family research and common design
boundaries; it is not an additional queued T or an implementation admission.

## Objective

Deliver a usable OmniBook 300 with System ROM 1.0 ABA: original firmware,
ROM DOS and original OBSETUP/ROM Windows, with persistent writable
storage and measured responsiveness. OB425/430 and cards 1.1s/2.0s remain
research references and possible future scope, not prerequisites or currently
queued implementations. HP's service manual identifies OB300 as 20 MHz
386SXLV, whereas OB425/430 use 486SLC/e; verify concrete silicon differences
rather than importing another machine's CPU personality.

## Read-only research baseline

Owner-supplied ABA archives were inspected in memory, without importing files
or executing the guest. All three binary and file ZIPs passed CRC checks.

| Card | Observed baseline |
| --- | --- |
| 1.0 / OB300 | 10 MiB content; FFS20 CIS; BIOS 1.0, reset date 1993-03-27; C&T 65510 VGA BIOS 1.3.0; ROM DOS 5.00 and Windows/Word/Excel module names. File archive has 125 files. |
| 1.1s / OB425 | 10 MiB content; FFS20 CIS; BIOS 1.1, reset date 1993-07-31; VGA BIOS 1.4.0; ROM DOS 5.00 and the ROM Windows arrangement. File archive has 142 files. |
| 2.0s / OB430 | 512 KiB image; BIOS 2.0, reset date 1994-02-01; VGA BIOS 1.5.0; DOS 6.20 in FAT12, with twelve files matching the file ZIP exactly; no corresponding ROM Windows bundle. |

The 1.x 16 MiB dumps contain repeating patterns beyond the 10 MiB declared
capacity. The 2.0s 16 MiB dump is exactly 32 repetitions of the normal 512 KiB
file. The separately named bad 512 KiB file differs at five CIS bytes and is
not the normal baseline. These observations do not establish hardware address
decoding by themselves; preserve original dumps.

For 1.x, Windows comprises ROM modules plus OBSETUP-installed writable files
and ROMUpdates. Absence of ordinary kernel/application EXEs from the file ZIP
does not mean the card's Windows environment is incomplete. The 1.1s scripts
provide BOOTDOS/BOOTWIN and OBBOOT C1/C3/C0 to select FFS/DoubleSpace behavior;
turning off the D-drive filesystem does not remove the firmware dependency.
The 2.0s BIOS contains explicit C-drive and temporary D-ROM-disk boot options.
Neither Windows 95 nor cross-card boot has been dynamically validated here.

Original image SHA-256 anchors:

- 1.0 common-memory 16 MiB: b98001a30b79f126483f823fd4d7a5ce91a973a5ab8fb35e3d76043b74a8e099
- 1.1s common-memory 16 MiB: d5ab5fb3c529df5bd2b920ceb509285ac1de003de49eb1a54ee7b444fb77b0d8
- 2.0s normal 512 KiB: 7f5611ee30fbdf7122da148937580967d1f0a1a23377c218a0af5153ebb8c053

Hardware source: HP service manual 5965-0244, 1996,
[OmniBook 300/425/430/530](https://www.elhvb.com/mobokive/edwin/laptops/hp/omnibook/service/ob300-425-430-530_sm.pdf).
Apply the existing [source policy](../etc/operations/policy/source-policy.md)
before further acquisition, derivation or packaging. Raw card images, Microsoft
software and writable media remain external; queue registration does not
authorize embedding the combined Microsoft-bearing card into published EXEs.

## Ordered implementation candidates

1. [Hardware, ROM and execution design](m5-ob300-design.md).
2. [Real firmware and ROM DOS](m5-ob300-rom-dos.md).
3. [Execution and interaction performance](m5-ob300-performance.md).
4. [Original ROM Windows](m5-ob300-rom-windows.md).
5. [Complete product qualification](m5-ob300-product-qualification.md).

This replaces the earlier suggestion to start with OB430. The existing family
ROM observations remain valid research, not proof of OB300 boot. Performance
is measured from the first executable path and has an explicit usability gate
before full Windows work; no elapsed-time promise is inferred from PC110.

App code owns model composition, firmware/card binding and unique hardware
policy. Existing Lib/Emulator own host services and generic lifecycle; applicable
X86 packages supply CPU, debugger and PC mechanisms. Shared extensions require
demonstrated neutral ownership and normal review. No second CPU executor,
host-side Windows loader or unproved ordinary-AT substitute is planned.

## Acceptance and boundaries

Each admitted machine/card pair must have named, reproducible checkpoints:
real reset/POST, correct display, working input, ROM DOS or the intended C-drive
boot, persistent writable storage, and working reset/stop/start. The 1.x pairs
also require ROM Windows startup and desktop interaction. A 2.0s DOS
boot does not qualify ROM Windows or Windows 95. Decide any Windows 95 target
and installation/driver matrix explicitly at admission.

Record every device/model as proved, bounded L1/L2/L3, or deferred with a named
receiver. Missing hardware documentation alone does not mandate indefinite
blocking: use justified reference models under the source policy, label their
accuracy honestly, and retain a single execution and timing path. Do not claim
complete family emulation from one successful BIOS screen. Avoid speculative
timelines or a numeric S allocation before the initial hardware audit.
