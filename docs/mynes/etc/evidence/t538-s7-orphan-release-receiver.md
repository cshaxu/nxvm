# T538 S7: MyNES Orphan-Release Repair Receiver

NXVM-hosted M5 T538 S7 admits the owner-reviewed Shared source-local orphan
release repair and both receiving Apps. MyNES T43 remains closed; no product
source, INI, snapshot or media is changed. The existing adapter consumes the
same neutral key events; no API or alternate input path is introduced.

Both current 0043 optimized stripped Release receivers were rebuilt. PE
architecture and absence of compiler debug sections were verified. Full suites
pass 132/132 on each width: 127 non-desktop plus five serial desktop cases.
The final non-desktop rerun also passes 127/127 per width. Both documentation
gates and six manifests pass. Shared source identity is S7 P1 064b9619b;
the receiver commit containing this evidence fixes the binary identity.

| Artifact | SHA-256 |
| --- | --- |
| mynes_0_0_0043_x64.exe | C9DB0320F3FA10167D1E250BCFADFED97559FD6556AC8296DB9B906AA729DA65 |
| mynes_0_0_0043_x86.exe | D355FCF73ABC57D51942E6DAF29FD6EEE69B6657BE50FA572FD795595810ACD1 |

The [host evidence](../../../nxvm/etc/evidence/t538-s7-orphan-release.md)
records the exact repair, source/test delta and qualification. SoftPC must import
the new Lib source/test roots in addition to S5's return differences; no sibling
was changed and six-root equality is not claimed.

Coordinator accepted receiver P3 193530996 after actual-change review. Subsequent
Shared P4 268464d49 changes only the owner-requested key-specific comment and
manifest; executable inputs/behavior and the two artifact hashes are unchanged,
so another build is not required. MyNES T43 remains closed.
