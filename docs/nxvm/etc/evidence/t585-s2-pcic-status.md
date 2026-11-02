# M5 T585 S2 — PC Card status register receiver

The Ricoh RF5C296/RF5C396L/RB5C396 manual EA-028-9804, June 1995,
SHA-256 `7eb037b495d7ed3f4334a1cdc6c6582c8fa9ceadf0fd680ebadec2575b60c7ec`,
printed pages 29–32, defines card-detect changes at index 04h, default
clear-on-read and the optional write-one acknowledgement selected by global
control bit 2. Indexes 1Eh and 5Eh address one shared global register.

The existing `x86-nxvm/chips/rb5c396` owner previously treated those indexes
as ordinary independent register bytes and did not latch insertion/removal.
The chip now latches the board-supplied `card_present` transition in the
selected socket, implements both acknowledgement modes and aliases the global
register. The App still owns 3E0h/3E1h bus routes; its existing unit test now
observes the change register through those real routes. Chip tests cover
both identification banks, socket isolation, acknowledgement and reset.

The same decoder permitted indexes 3Ch–3Fh even though each socket has only
60 register bytes. It now returns the existing unselected-device `FFh` value
and ignores writes for these indexes, avoiding out-of-bounds access. Tests
exercise both sockets at the invalid range; the `FFh` behavior is a bounded
model rather than a claim of a measured electrical readback.

This is a bounded controller register change. The single Boolean input models
both detect pins together; no claim is made about independently changing pins.
Ready/battery changes, software card-detect, interrupt output, card windows,
the card backend and App insertion/removal are not implemented by this change.
PC110-QEMU and PC110-EMU contain only empty port placeholders, not a full card
reference. Those remaining routes need separate implementation and tests.

Focused x64 and x86 tests each passed 3/3: the chip contract, App PCIC bus
contract and source manifest. After the bounds correction, cached full
repository unit tests passed 712/712 on each width. The default-profile
registered integrations passed 48/48 on each width: 33 background cases,
six native cases on an isolated desktop and nine PC110 cases. The desktop
fixture is not evidence of physical RDP input. MyRaidenII's opt-in selected
ROM startup and deployed Console probes also passed on both widths; the startup
probe is a finite execution checkpoint, not a gameplay claim.

Cached builds produced all 14 product executables; their PE architecture was
checked. MyPC110's two deployed hashes matched their corresponding build
outputs. Ten NXVM-family deployed binaries changed; MyNES and MyRaidenII
outputs remained unchanged. The first attempted x64 integration build used
the nonexistent `build-integration-tests` target and failed before testing;
the subsequent explicit integration targets built successfully. An early x86
PC110 integration overlapped a release rebuild and was not counted; the
post-build repeat passed 9/9. Isolation setup was denied in the restricted
environment; the same hidden-desktop launch succeeded with the approved
execution environment.

An initial x64 XT profile run used the aggregate target and repeated all 28
registered integrations, including native tests, instead of only XT-specific
cases. It completed 28/28 in the ordinary background test environment; that
run was not the isolated-desktop verification described above. The following
aggregate was interrupted by the executor after identifying the duplication;
subsequent profile runs select only the profile-specific `integration.*`
cases. No interrupted run is counted as a pass.

The four PC profiles' selected integrations passed on each width: default
within the 48 registered cases; XT x64 28/28 aggregate and x86 3/3 selected;
AT 5/5 selected per width; DeskPro 6/6 selected per width. Both stable build
trees were restored to the default profile and its product rebuilt afterwards.
Shared production and Shared test changes were reviewed and committed as
`eeb281cb2`; NXVM-owned test, artifacts and this record are delivered by the
following product-scoped commit. No owner INI, ROM master or disk was changed.

## Released executable identities

| Product artifact | PE width | SHA-256 |
| --- | --- | --- |
| my5160/my5160_0_5_0582_x64.exe | x64 | `e099238b8d0996ff83e216c8e3fdd210febf494e262e2e00d55002490d97ddf7` |
| my5160/my5160_0_5_0582_x86.exe | x86 | `d09bfce279b260d197a9a232e82715dc05a4cbb7ba77286823f63e982a92e192` |
| my5170/my5170_0_5_0582_x64.exe | x64 | `dc10ce33d841e2aa9f9bb175cd8ee7f2cce85be3eb36c0543a11cc9ce07e2acb` |
| my5170/my5170_0_5_0582_x86.exe | x86 | `71f87e35b964dec95094173ccb71ac45beb73d95f28f96e03fd7ee8f52b6a6ee` |
| mydeskpro386/mydeskpro386_0_5_0582_x64.exe | x64 | `37538848bb12b272204e9b8e7c655eec8fa45f7f49f916684f775f90f8d0562a` |
| mydeskpro386/mydeskpro386_0_5_0582_x86.exe | x86 | `9e94c67ee35767a782aeed7253663798d5b9be3fb0e986ddf9b0d2435f37e6e5` |
| mynes/mynes_0_0_0044_x64.exe | x64 | `b499cd2583ecddd1ae01eed882e21d49d20187558e2138858e5a27117cf74fc8` |
| mynes/mynes_0_0_0044_x86.exe | x86 | `718617af2fbf6d49c36ae3bb10043dc3b2383e35440e7a371e503b8bd4b4ea16` |
| mypc110/mypc110_x64.exe | x64 | `fc3d3fb8ffac41d2c43d446d5908c238269aa1a1e5fa275611d156eedcccfa18` |
| mypc110/mypc110_x86.exe | x86 | `454917445774ddf40392a79e7e72c6d3f0f1923074b3ce59036ccac2e3349a05` |
| myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `0e48ffde257e0f6094ed5dbccb3b03c5d2ea7e7fe8a01a0f27a11df1659fc637` |
| myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `5d0181af40715d47630fe3feffbc7bbf2bff1aaaad3825e8d5e95fcfec99f650` |
| nxvm/nxvm_0_5_0582_x64.exe | x64 | `12e68b1dd8a28e78b9beaaab5e314a51a19dc74d81375042c1a09fa4a1d701c7` |
| nxvm/nxvm_0_5_0582_x86.exe | x86 | `7208a876a1b5b2c66facd9462a958e565386a5781241dc52009136b98be47465` |
