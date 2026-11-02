# M5 T585 S6 — PC110 PC Card memory receiver (in progress)

The existing Ricoh controller decodes two sockets and five memory windows each.
The PC110 adapter borrows task-owned Storage medium leases for common and
attribute spaces and registers a disabled-by-default Core replacement route.
Enabled windows bound its live range; register, insertion, removal and reset
changes invalidate the CPU view. A missing attribute image reads as open bus;
window or medium write protection ignores writes. Socket and window ordering
is an L1 deterministic model, not a verified physical priority claim. The
zero-wait and wait-state register fields are exposed as decoded facts; this
does not claim their bus timing is modeled.

Core must preserve wide accesses to devices which require them. A partial
replacement explicitly opts into splitting a lower route when only part of a
wide access matches; ordinary replacement devices retain their existing width
semantics. A separate masked-address correction handles multi-byte A20 wrap,
including when installed RAM ends at the boundary. The pre-existing issue of
a provider declining after a successful query remains separately tracked; the
PC110 provider does not deliberately decline after claiming an access.

An initial x64 unit run found a MyRaidenII board-control regression when an
unconditional byte probe changed its accepted word transactions. That run was
stopped; the opt-in contract was implemented and the existing MyRaidenII board
bus, Core memory registration and PC110 PCIC focused tests passed together on
x64. The current dual-width full verification has not yet completed.

Machine-boundary card media are test-owned; this does not claim a user-facing
insert command, I/O-card support, INTR wake, physical-card validation, or an
unmodified host medium. Slot/controller access is owned by the machine thread.

A subsequent review found that an error in a higher-priority partial query
could be hidden by a lower wide route. The partial-query error now follows the
actual byte route and a Core-owned regression injects that failure for reads
and writes. The three focused Core/PC110/MyRaidenII tests passed on both
architectures. The earlier 712/712 unit runs and x64 48/48 integration used
the preceding implementation, so they are retained only as preliminary
evidence; final release and the complete dual-width runs are being repeated.

## Final verification

Shared `6f68a2bde` is the reviewed Core/chip change. MyRaidenII receiver and
its release pair are `0e6cd7e95`; the PC110 adapter, PC-family artifacts and
this record are the NXVM delivery. No Lib or Emulator source was changed.

- Complete cached repository unit tests: x64 712/712; x86 712/712.
- Registered integration tests on an isolated desktop: x64 48/48; x86 48/48.
  Native MIDI output passed on both in this environment.
- Additional selected PC-profile integrations on both widths: XT 3/3,
  AT 5/5, DeskPro 6/6. These include real guest/firmware paths.
- The selected-ROM MyRaidenII bounded startup and deployed hidden Console
  probes passed on both widths. They are not a gameplay qualification.
- Fourteen executables were rebuilt and verified for PE width. The five PC App
  pairs and MyRaidenII pair changed; MyNES was built and its deployed bytes
  matched without modification. The default PC, PC110 and MyNES deployed
  files matched their final build files; MyRaidenII matched the staged stripped
  releases. Profile deployments were produced by the same build that ran
  each profile's tests.
- No physical card, interactive RDP session or real PC Card I/O device was
  exercised. The PCIC timing register values are decoded, not fully simulated.
  This does not implement a product insert command or prove a physical slot.

The pre-final x64 PC110 resume/cold handoff tests took 64.08/55.43 s versus
65.16/59.85 s in S5. These are test timings under different host load, not a
controlled benchmark or a claim that overall PC110 startup performance is fixed.
The existing lower-provider fallback-after-operation-decline debt remains
recorded separately; this card receiver does not intentionally return that
status after a successful claim.

## Release identities

| Artifact | PE | SHA-256 |
| --- | --- | --- |
| assets/nxvm/nxvm_0_5_0582_x64.exe | x64 | `0E803C552F2D745CAFA13D9960BEA054ABB2B102398F2C26E70DAE8A68D0546F` |
| assets/nxvm/nxvm_0_5_0582_x86.exe | x86 | `5E31133A95BE64EBF231B7FF5ACD971B1D165A6C97941099E8345DBA4621AE77` |
| assets/my5160/my5160_0_5_0582_x64.exe | x64 | `73D013CF1051AA95EE72B5E8617EBBC82FEF1CEA2D09CF2D98932C2A057E790E` |
| assets/my5160/my5160_0_5_0582_x86.exe | x86 | `1E7BDF83D8C04CC9588979BAA20D2FA073B220EA5D2505EBC8B0BAB1FB7BA675` |
| assets/my5170/my5170_0_5_0582_x64.exe | x64 | `14A7D3102D2EA694AA6C2CFFC43F16A660A603C99D71676E7496F4FE40C83723` |
| assets/my5170/my5170_0_5_0582_x86.exe | x86 | `618C781F7A5E5F78929F01D0739548DE03C4BD0437775BCBAC786D504AB9ED90` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe | x64 | `8D548BD49E238BFEC585A154B15678B5BF6C1CFE4F45BEF75767CBC96AF01CD7` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe | x86 | `906BC503973938B60BA3BF2C6E0EE860CED1DF875133A75CC6D2FE2A78ECD4B2` |
| assets/mypc110/mypc110_x64.exe | x64 | `9F9229BD9E8BDD3D3150276B7B2740D7F20D5249BCBE1987F89DB7D65B89C1F1` |
| assets/mypc110/mypc110_x86.exe | x86 | `CFBBA35E056536E99FC61B7276D62DA0D9B6E8E161C40BF7CEFE7D60813CA29F` |
| assets/mynes/mynes_0_0_0044_x64.exe | x64 | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| assets/mynes/mynes_0_0_0044_x86.exe | x86 | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `3F8C61E1630D85D23E06D44EA4FDF34C8E107DEEC53563AAB92DB82BE668033E` |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `056607CC84588A475301FDB9DE98E162B700D46B64254E7BCB2A2B76049EFF92` |
