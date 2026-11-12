# M5 Candidate: HP OmniBook emulator

Owner requested queue registration on 2026-11-11. This is an unadmitted
candidate at the tail of the NXVM queue, not an allocated numeric T or an
authorization to modify shared code. Product naming and build variants will
be decided at admission.

## Objective

Emulate the classic OmniBook 300/425/430 hardware family and its System ROM
1.0, 1.1s and 2.0s cards. Execute the original guest BIOS and system software:
POST, ROM DOS, writable C-drive boot, and the original ROM Windows environment
where supplied. Preserve the distinction between machine hardware and card
revision; cross-machine card compatibility requires evidence.

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

## Design and implementation sequence

1. Freeze exact hardware/card identities and external asset provenance. Audit
   chip manuals, reset mappings, PCMCIA common/attribute memory, HP control
   registers and available references against current shared implementation.
   Establish a finite hardware/boot coverage ledger and explicit model grades.
2. Prove a minimal real-firmware boot chain. Prefer OB430 with 2.0s as the first
   research baseline because its boundaries, FAT12 files and BIOS checksums
   are verified; confirm this choice at admission. Trace reset, POST and
   missing I/O behavior instead of replacing guest BIOS services.
3. Implement the required board mapping, PCMCIA and writable storage, C&T
   65510 display, keyboard/pointer and necessary power behavior. Reuse existing
   CPU and neutral devices after checking contracts; do not relabel F65535 as
   F65510 or assume every AMD/Cyrix profile equals the generic Intel profile.
4. Qualify ROM DOS, C-drive boot and interactive read/write operation. Then
   add OB300/425 differences, FFS20/ROM module access and original OBSETUP,
   Windows, Word and Excel workflows. Guest ROM code owns software semantics.
5. Qualify lifecycle, reset, input, display and performance on both host widths;
   publish under the applicable artifact/source rules and obtain owner acceptance.

App code owns model composition, firmware/card binding and unique hardware
policy. Existing Lib/Emulator own host services and generic lifecycle; applicable
X86 packages supply CPU, debugger and PC mechanisms. Shared extensions require
demonstrated neutral ownership and normal review. No second CPU executor,
host-side Windows loader or unproved ordinary-AT substitute is planned.

## Acceptance and boundaries

Each admitted machine/card pair must have named, reproducible checkpoints:
real reset/POST, correct display, working input, ROM DOS or the intended C-drive
boot, persistent writable storage, and working reset/stop/start. The 1.x pairs
also require ROM Windows startup and basic Word/Excel interaction. A 2.0s DOS
boot does not qualify ROM Windows or Windows 95. Decide any Windows 95 target
and installation/driver matrix explicitly at admission.

Record every device/model as proved, bounded L1/L2/L3, or deferred with a named
receiver. Missing hardware documentation alone does not mandate indefinite
blocking: use justified reference models under the source policy, label their
accuracy honestly, and retain a single execution and timing path. Do not claim
complete family emulation from one successful BIOS screen. Avoid speculative
timelines or a numeric S allocation before the initial hardware audit.
