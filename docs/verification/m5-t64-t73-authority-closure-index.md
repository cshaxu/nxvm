# M5 T64--T73 Authority Closure Index

This compact index closes the task-level verification record for the completed
instance-authority sequence. Detailed commands, stop conditions, and migration
notes remain in the linked subtasks and archived plan.

| Task | Verification result | Artifact SHA-256 |
| --- | --- | --- |
| T64 | Design rules were approved and expanded into T65--T73; no runtime path changed. | None |
| T65 | Explicit VM session root, full build, DAG, Console/debug, and FDD progress gates passed. | `583E7E9CB9D53E37ADF03A0FA7AF077EBF07145406933E89FF161407E1D3BB28` |
| T66 | CPU context, CPU probe, debugger, and profile gates passed. | `12390790CD7497F076DCE47037D8FA3A740C0DB96EC3F8C71D54F394D567D8BC` |
| T67 | RAM/port isolation, CPU probe, Console, debugger, and profile gates passed. | `914118D823F788F5C7F7AD7BA3F5991166A0844BE6D38D791BDA80E1CB4E58BC` |
| T68 | S1 artifact is revoked; S2 repaired the live-machine callback binding and passed FDD boot survival gates. | `F16C5E84AD859FD24CA9AF9CF78376D9C59EE6891586629E61727002F8C8FE6F` (revoked) |
| T69 | Frozen provider-slot, Console, and FDD/HDD profile gates passed. | `F213C9A55D90BF6D6844D5FD286EFF11254D4A88A7FD1B874F6AC2787E6E7093` |
| T70 | S1 artifact is revoked; S2 corrected FDC initialization and the three-second DOS-prompt/display gate passed. | `8BE1DAD2D9F47643E1ABEA48C7AB91E604CD499ABF2D34D0C196A9A86A4E4FD4` |
| T71 | S1 artifact is revoked; S2 corrected snapshot binding and manual Console review passed. | `D63B4B8D12C83F67E7A98395B864CDA6A0BF369E54F2662C71D308ECE914460E` (`0071a`) |
| T72 | Console/debugger, full-authority, and FDD prompt/display gates passed. | `945DD773B4230E95D12E1C1F739070D68F7D656D6D83302570EC1965ABF11645` |
| T73 | Full GCC, selector closure scan, two-session isolation, Console/debugger, and FDD prompt/display gates passed. | `B4974DDA7DB47FC19E2BFA9691197DBD86397D7AE0595ECFCCB05E108950F92C` |

The authoritative task order and closure conditions are preserved in the
[archived M5 instance-authority plan](../history/m5/planning/m5-instance-authority-plan.md).
