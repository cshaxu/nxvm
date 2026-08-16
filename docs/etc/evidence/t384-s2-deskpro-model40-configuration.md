# T384 S2: DeskPro Model 40 Configuration And EGA Personality Ownership

`M5:T384:S2:DESKPRO-MODEL40-CONFIGURATION:OK`

`M5:T384:S2:EGA-PERSONALITY-OWNERSHIP:OK`

## Owner-Approved Configuration

The selected machine is the original 1986 Compaq DeskPro 386 Model 40 / DeskPro
386/16 system-board and storage baseline accepted in T384 S1, with these
owner-approved configuration selections:

| Configuration field | Selected disposition | Evidence boundary |
| --- | --- | --- |
| Display | Compaq Enhanced Color Graphics Board plus Compaq Color Monitor. | Compaq September 1986 Volume I lists this documented pair.  It is a Compaq EGA personality, not an IBM EGA, CGA, VGA, or generic adapter claim. |
| Input | Compaq 101-key Enhanced Keyboard. | Compaq September 1986 Volume II identifies the Enhanced Keyboard as 101-key and distinguishes it from the optional 84-key keyboard.  AUX/IRQ12 remains unselected. |
| Firmware | Owner-managed DeskPro 386/16 Revision E external-ROM constraint. | The repository retains no byte, local path, hash, part catalogue, manifest or default dependency.  A future runtime composition may consume only an approved BYOB provider contract. |
| Storage | One 1.2 MB diskette drive and one 40 MB fixed disk through the Compaq Multipurpose Fixed Disk Controller. | T384 S1 primary-source selection remains binding; ATA/HDC and IBM MFM are excluded. |
| Numeric coprocessor | None. | No x87 expansion is selected. |

## VADP And Personality Boundary

The source sweep finds a single existing VADP owner for planar EGA aperture,
sequencer, graphics, attribute, CRTC, capture and registration semantics:
`core/machine/vadp`.  The current `default-pc-at` descriptor is a generic
80386/EGA/ATA product configuration, not a Compaq descriptor.  No DeskPro
descriptor, personality type, ROM binding or Compaq display route exists.

The approved implementation allocation is deliberately small and disjoint:

| Layer | IBM EGA path | Compaq EGA path | Receiver |
| --- | --- | --- | --- |
| Shared VADP core | One common owner for generic planar/register/state mechanics.  A shared defect is repaired once at this earliest owner only when both personality contracts require the same semantics. | Same shared owner; it must not embed IBM or Compaq defaults. | Earliest shared owner, admitted only by the later task that proves shared semantics. |
| Personality/profile contract | IBM EGA contract remains the retained current-product device capability. | Add one Compaq EGA personality/profile contract around the existing VADP core, with source-backed Compaq defaults, ports, memory, monitor-facing behavior and firmware binding. | IBM: current-product device-capability L3 closure.  Compaq: DeskPro selected-device functional closure. |
| Timing and acceptance | IBM service/raster behavior is proved only in current-product device-capability L3 closure. | Compaq board availability, service timing and monitor-related selected behavior are proved only in DeskPro board bus/device timing closure. | Their respective closure tasks; neither evidence set transfers across personalities. |

This is a source/ownership allocation, not a claim that the existing generic
EGA implementation already matches either selected personality.  It explicitly
forbids a second VADP core, copy-pasted EGA mechanics, an IBM-derived Compaq
default, or using Compaq behavior to close the current IBM EGA product surface.

## T384 Completion Transfer

T384 now has a complete Model-40 selection: CPU/system board, one MiB memory,
ROM constraint, Compaq EGA/color monitor, 101-key input, Compaq storage, no
x87, and its PIC/PIT/DMA/RTC/NMI/reset/bus ledger.  It transfers the next
queued DeskPro CPU candidate only the 80386DX-16 CPU/state contract.  The later
functional candidate owns the first runnable Compaq composition and both the
Compaq EGA personality and selected storage/input/platform completeness.  The
later board candidate owns all board/device timing.  T384 makes no runnable,
firmware-execution, device-completeness, or L3 claim.
