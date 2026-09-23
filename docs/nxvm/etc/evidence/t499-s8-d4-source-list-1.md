# T499 S8: Model-40 D4 Source List 1

`M5:T499:S8:D4-LIST-1:OK`

## Scope And Sources

The selected 1986/1987 Compaq DeskPro 386 D3PE processor-board material is
the primary authority. It was read transiently under the source policy; no
manual content, firmware, media, local asset path or digest is retained here.
Read-only 86Box and PCjs source reviews are secondary cross-checks. MAME,
Bochs and QEMU do not provide an applicable selected DeskPro D4 implementation
and therefore supply no D4 relation.

An external model confirms a Manual-L3 row only when it implements the same
relation already established by D3PE and the project probe observes it. A
reference-only relation is External-L2. A reference disagreement or omission
cannot promote a row.

## Complete Selected Functional Universe

| Row | Primary/manual result | External cross-check | Required disposition |
| --- | --- | --- | --- |
| D4 diagnostic/control aperture | D3PE names the diagnostic read and control write at the selected physical aperture. | 86Box and PCjs model the aperture. | Manual-L3. |
| Diagnostic parity status | D3PE assigns the low diagnostic bits to byte-lane parity status and defines control-write clearing. | 86Box and PCjs expose diagnostic state, but differ in unneeded details. | Manual-L3 for the selected lane/clear contract. |
| Diagnostic topology bits | D3PE identifies the selected base/extension configuration observation. | 86Box derives the visible configuration from installed RAM; PCjs labels some settings incomplete. | Manual-L3 only for D3PE-defined topology; unresolved encodings remain External-L2 or unsupported. |
| Compatibility RAM | D3PE defines the 128-KiB compatibility RAM region and states that firmware supplies any ROM copy. | 86Box and PCjs retain a distinct compatibility mapping. | Manual-L3 for decode/ownership; no firmware-content claim. |
| ROM replacement | D3PE defines the control-selected compatibility-RAM decode over the ordinary ROM window and reset-disabled state. | 86Box and PCjs both reconfigure mapping on the control write. | Manual-L3. |
| Write protection | D3PE defines control-selected protection of the compatibility RAM and, while replacement is active, its replacement window. | 86Box and PCjs both change write accessibility with control state. | Manual-L3. |
| A20 interaction | D3PE specifies the selected extension decode's dependence on LOWA20. | No external value is needed. | Manual-L3. |
| D4 RAM setup bytes | D3PE does not establish a complete software-visible setup-byte encoding or a selected mapping transition. | 86Box recomputes its internal RAM/cache model on a setup-low write; PCjs stores setup readback without a mapping change. For the selected two-MiB configuration, setup-low `2` exposes `FA0000h`--`FDFFFFh` as relocated `A0000h`--`DFFFFh` backing while `F00000h`--`F9FFFFh` is absent; setup-low `1` disconnects the extension-RAM banks below `F00000h`. | External-L2 for those selected reset/decode transitions and readback values. Other setup-low selections remain unsupported until a source/probe establishes them. |
| Aperture bytes outside admitted registers | D3PE identifies only the selected diagnostic/control byte. | 86Box and PCjs expose a page/register form but differ in details. | External-L2 only for the retained nonresponsive-page convention. |
| D4 RAM physical cycle, cache and ISA timing | D3PE supplies physical-cycle material, but the current Core time/transaction inputs do not yet admit a complete receiver. | Reference models are not timing authority. | Out of this functional batch; retained board-timing receiver. |

## Consequence For Migration

The current profile-private D4 state must move as one functional owner, but
S10 may implement only the rows above. In particular, it must preserve the
Manual-L3 control-selected relocation/protection path and may implement only
the selected reset decode established as External-L2; it must not infer the
remaining setup-byte state machine from a reference model. The Model-40
pre-FDD replay remains a separate diagnostic outcome until the migrated,
source-classified D4 owner is actually replayed.
