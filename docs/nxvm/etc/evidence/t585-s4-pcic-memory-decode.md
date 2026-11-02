# M5 T585 S4 — PC Card memory-window decode

Ricoh EA-028-9804, SHA-256
`7eb037b495d7ed3f4334a1cdc6c6582c8fa9ceadf0fd680ebadec2575b60c7ec`,
printed pages 18 and 38–40, describes five enabled 4 KiB-granular windows
per socket, a 24-bit ISA address and a 26-bit two's-complement card offset.
The offset-high register selects attribute/common space and write protection;
the start/stop fields also report data width and wait configuration. Those
register facts are manual-derived; actual board priority and timing remain
unverified.

The independent controller now translates an individual selected slot/window
and reports the card address, contiguous span up to the window end or card
wrap, attribute selection, protection, width and wait field. It rejects a
disabled, reversed, below-64-KiB or out-of-window decode. It does not claim
that a card is present and does not choose between overlapping windows; those
are board responsibilities. The decoder currently reports the stop-register
wait field, but does not yet expose the separate start-register zero-wait bit;
the timing receiver must cover both before claiming cycle-level window timing.
The existing chip test checks both sockets, all
five windows, boundaries, negative offset, 26-bit wrap, attributes, protection,
width, waits, invalid selectors and reset.

This S does **not** install a production Core memory provider or create a
card medium. Core's existing fast-span check treats any covering provider as
potentially active. A permanent 16-MiB overlay would therefore affect ordinary
RAM and CPU cache behavior even with no card inserted. The next S must design
and verify a live, single-owner decode route and its cache invalidation rather
than introducing that broad always-on penalty. The independently recorded
Core masked-span/fallback issue also needs consideration at that integration
boundary. Chip-unit success does not prove guest-visible card operation.

The reviewed Shared decoder and test are committed at `cf2782271`. Cached
unit suites passed 712/712 and default-profile registered integration suites
passed 48/48 on each width. XT, AT and DeskPro selected integrations passed
3/3, 5/5 and 6/6 respectively on each width. The optional selected-ROM
MyRaidenII startup and product Console probes passed on both widths without a
gameplay claim. All seven product pairs were built; their PE width and identity
are recorded below. The 10 changed NXVM-family artifacts are delivered in the
following product-scoped commit; MyNES and MyRaidenII binaries were unchanged.
Original owner configuration, ROM masters and disk images were not modified.

## Released executable identities

| Product artifact | PE width | SHA-256 |
| --- | --- | --- |
| my5160/my5160_0_5_0582_x64.exe | x64 | `812af8f63dc5f6bdded73649f3224bb3992074949ae2ae292c66b8fbcf69ced8` |
| my5160/my5160_0_5_0582_x86.exe | x86 | `c345caf751afaa36f295e6b54dbd525a7831b3f98bbf34760139d2d46c3f8ccb` |
| my5170/my5170_0_5_0582_x64.exe | x64 | `328eed3d030d631d478b93a9c87ccdf9f560942812dd5a29d06229f6813413e3` |
| my5170/my5170_0_5_0582_x86.exe | x86 | `4d9c26642a3c75609843862df5ed4ac21c84318d3f6ba726521421723510aa69` |
| mydeskpro386/mydeskpro386_0_5_0582_x64.exe | x64 | `75cbabb709bbfb1cf40ec95bde94de8d6ee532c1d84c354de48b29b058673304` |
| mydeskpro386/mydeskpro386_0_5_0582_x86.exe | x86 | `ef6778d37032dce10f0dbcb65299638f3063a0c7f2b75523e4ad59dd61ad0848` |
| mynes/mynes_0_0_0044_x64.exe | x64 | `b499cd2583ecddd1ae01eed882e21d49d20187558e2138858e5a27117cf74fc8` |
| mynes/mynes_0_0_0044_x86.exe | x86 | `718617af2fbf6d49c36ae3bb10043dc3b2383e35440e7a371e503b8bd4b4ea16` |
| mypc110/mypc110_x64.exe | x64 | `8f75ecbf86894e9f72b07879017ad1557871d2b0c86fde42d8c8d01384774021` |
| mypc110/mypc110_x86.exe | x86 | `13d4795624ceaa3676e6c03908cd2b0b5c91d565467ff7e2aa8ad908b5fabea3` |
| myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `0e48ffde257e0f6094ed5dbccb3b03c5d2ea7e7fe8a01a0f27a11df1659fc637` |
| myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `5d0181af40715d47630fe3feffbc7bbf2bff1aaaad3825e8d5e95fcfec99f650` |
| nxvm/nxvm_0_5_0582_x64.exe | x64 | `aef6371967d2e4903ae5d3f3e69761140a2f03a9e0be1d9648bd672378ca8b29` |
| nxvm/nxvm_0_5_0582_x86.exe | x86 | `bacc7e6f2d95c9051281bb5de78162c8b144c21c963b4592cbaffaa204b04f72` |
