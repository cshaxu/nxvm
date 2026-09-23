# T386 S27: CECG Residual Functional Audit

`M5:T386:S27:CECG-RESIDUAL:OK`

## Primary-Source Reconciliation

The Compaq *Enhanced Color Graphics Board / Color Monitor Technical Reference
Guide* (December 1986) was consulted transiently under the source policy. No
manual content, firmware, media, local path, hash, binary, or third-party
source is retained. The audit checks its control-and-status-register semantics
against the current shared VADP and selected Model-40 declaration.

S9--S13 already own the selected static configuration, Control Mode storage,
Feature-Control/Environment state, light-pen status, I/O-base routing,
CPU-video-memory gate, and SW1/Input-Status-0 observability. The remaining
register effects divide as follows:

| Source-determined observable | Current disposition | Receiver |
| --- | --- | --- |
| `3C2h` bit 4 Special-Features/video-driver selection | The selected Model-40 has no Special-Features board. The effect selects physical interface output, not a present logical consumer. | DeskPro board/device timing. |
| `3C2h` bit 5 Odd/Even display-page selection | No current VADP Odd/Even page state, memory route, or snapshot consumer exists. This is reusable display mechanism, not a Compaq-local workaround. | Next T386 shared-VADP receiver. |
| `3C2h` bits 6--7 sync polarity | Signal-polarity output has no timing/monitor consumer. | DeskPro board/device timing. |
| `3C6h` monitor selection, external/internal blank, restore and diagnostic modes | Current port state is retained; the primary effects concern board compatibility blanking and selected monitor signals. No selected internal monitor or timing model exists. | DeskPro board/device timing. |
| Input Status 1 multiplexed live-video bits | Requires live pixel/sync phase and attribute-plane timing, not a static palette or snapshot inference. | DeskPro board/device timing. |
| Option-ROM/BIOS mode programming | Requires separately admitted external EGA firmware lifecycle and a source-backed consumer contract. | Later profile/firmware admission. |

The static `3C2h` bit-5 omission is a reproduced shared functional gap. It
must be repaired at VADP before T386's final functional audit; its remaining
EGA breadth is not authorization to add IBM EGA, VGA, analog monitor or timing
behavior.

## Sweep And Transfer

The audit searched all tracked VADP personalities, CECG port handlers,
configuration/reset paths, display snapshots, Model-40 declarations, EGA
controller tests, TODO and T386 proposal records for `3C2h`, `3C6h`,
`odd/even`, page selection, monitor, blanking, status and firmware routes. It
found no alternate Odd/Even implementation or profile-local consumer. Generic
EGA remains isolated from CECG-only routes.

This evidence closes no board timing, firmware or L3 boundary. It creates the
next T386 functional prerequisite and preserves the remaining physical,
firmware and timing boundaries without treating static register storage as a
complete consumer contract.