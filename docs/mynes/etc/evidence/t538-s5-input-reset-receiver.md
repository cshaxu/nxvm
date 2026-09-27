# T538 S5: MyNES Input Reset Import Receiver

NXVM-hosted M5 T538 S5 admits receiving binaries/evidence only. MyNES T43 stays
closed. No MyNES source, configuration, snapshot or media changes are made.
Shared production is identical to SoftPC
40da7d0059c97e7f3a5026d16c128417e48e74b7 (also unchanged in 19e853c0).
The owner approved local portable test corrections for SoftPC to import;
[host evidence](../../../nxvm/etc/evidence/t538-s5-input-reset-import.md) enumerates
the eight differing test paths. There is no sibling build/runtime dependency.

Both 0043 Release products are rebuilt using -O3 -DNDEBUG and strip-all.
Objdump verifies pei-x86-64 / pei-i386 and absence of debug sections.
x64 and x86 full suites each pass 132/132: 127 non-desktop cases plus five
serial desktop cases. This includes repository-only units and configured
external integration; no cases are removed to obtain the pass.

| Artifact | SHA-256 |
| --- | --- |
| mynes_0_0_0043_x64.exe | B05149CEAE147B050D4CD5606E9E3BB84ED5360C9C3BAC12971ED1A5BFA96B6F |
| mynes_0_0_0043_x86.exe | BFAE4DFA4BF1358450C0AFA59CE173FB82A340ED996945ED3C1B385CB660D0BD |

INI SHA-256 remains 198846D0D4EB3C7CAB1357992D7AB3441BECA676706AF1BAB0DAF507FB6BCC15;
snapshot remains B68F9CE4BC41BBD447DB68832128B266D7D503A0971D4A9C66B6D30FD53EBEBD.
Common consumes source reset and emits ordinary recorded key breaks; the MyNES
driver uses its existing bindings, with no adapter fork or new guest protocol.

Shared source revision is 0c71110b0714fffee3ef8c40bb352ee3dd71ee40. Both product
documentation gates and all six manifests pass; this receiver delivery is
product-scoped under the NXVM-hosted packet, not a reopened MyNES T43.
