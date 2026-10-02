# M5 T540 S46 Board-Owned PIT Pair

## Scope and actual diff

The primary and auxiliary PIT instances and the auxiliary-configured bit now
reside in the existing sole `core_machine_board_state`, rather than flat
`core_machine`. No copy, second owner, accessor API or device framework was
added. The frozen clock plan remains a separate value input; its
`auxiliary_pit` ratio is not a chip instance and was not moved.

The existing board constructor creates the same PIT pair in the same order.
Port transactions, Gate/OUT callbacks, PIT0 IRQ0, PIT1 DMA refresh, PIT2
speaker, Model 40 auxiliary PIT, reset, deadline and finalization still call
the same chip operations at the same points. Four production and eight direct
test consumers were retargeted to the board attachment. The PIT/PIC tail
source gate additionally rejects either new board PIT instance path in the
neutral scheduler. Tracked source/gate/test diff adds 56 and removes 51 lines
(net +5). No timing formula, chip algorithm, profile, INI, firmware, media,
Shared or MyNES source was changed.

## Verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
  The first x86 run, concurrent with four Release rebuilds, had a single
  failure in the timeout-sensitive `vm-runner-error-propagation-smoke`; it
  passed in isolation, and the complete suite passed after builds ended.
- `verify-current-specialized-gates`: x64 and x86 pass. The updated
  PIT/PIC-tail gate was rerun after its final edit and passes.
- Fixed-profile external boot probes, **once per profile and width**:
  Default reaches `dos-prompt`; XT, IBM 5170 and Model 40 reach
  `installer-running`. **8/8** accepted terminals.
- Eight optimized 0540 EXEs rebuilt in the existing four
  `assets/nxvm/<profile>/` directories. PE format is correct for four x64
  and four x86 products; `objdump -h` finds zero `.debug` sections.
  SHA-256 by profile, x64 then x86:
  - Model 40: `8A61074413173AE3AE6B08F46829CE217ACFB5802EA87546BC1494123D742412`, `0ECE630706A5D397CBA86DEB62C691F7E2B278817E4DAADFDC7D13719FDA9C60`.
  - Default: `CCD2ADAFACD2156ED7DE47DA63CAC009840D7E750E831D5656F5C4687E059245`, `307838DED75B411ECD12F772898CE033EE5571F13B05F2B7E76B613A35A2E3A2`.
  - XT: `914B5D58D5F1906282532BA810425DC30A5D014051BBD0DB33215F8F2454F03B`, `39B654B3263964F74F078179AABAB22E65104169412AB3FD85B5A70534938269`.
  - IBM 5170: `1183BF2234EB4661BCCF41249EFD7B61CC83D2D0BF69BF211E1774966DF91AD6`, `CE4002AC5F0CE8870A1EAE9164D020B9114534573A362C0A3B54915A39D19FE1`.

S46 closes only the PIT group. S47 receives DMA; T540 remains open.
