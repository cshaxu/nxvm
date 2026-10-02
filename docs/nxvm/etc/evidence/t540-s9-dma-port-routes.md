# M5 T540 S9 DMA Port Routes

DMA now registers its primary controller, optional secondary controller and
page latches through one Core-owned typed route batch. Core owns the only port
table and publishes the whole candidate or none. The DMA adapter retains its
existing controller and page state; it no longer receives `t_port`, handles
raw port callbacks or owns a registration checkpoint. On registration failure
it destroys the newly created chip instances, while Core rolls back the route
candidate.

The existing DMA page bank is made explicit as an exclusive byte-lane end in
the route description. A CPU word or dword access fully inside that bank calls
the same typed provider once for each consecutive eight-bit latch. Native
controller ports, other devices' ports and boundary-crossing accesses retain
one provider call. Neither an unregistered neighboring page address nor a
different route entry is consulted for the extra lanes. This is Core port
execution behavior, not a second DMA-specific dispatcher.

## Scope and actual-change review

- The source/test/CMake diff has 16 tracked paths, 196 added and 154 removed
  lines (net +42) by `git diff --numstat`; documentation and eight generated
  EXEs are excluded. The positive net is the bounded width contract plus its
  Core and rollback regressions. One DMA registration path replaces the old
  per-endpoint registration, byte/wide callback pair and local checkpoint.
- Review checked primary-only and paired route sets against the old source:
  primary controller ports 00h-0Fh, primary pages 81h-83h, optional spare
  pages 80h/84h-86h/88h/8Ch-8Eh, secondary pages 89h/8Ah/8Bh, and optional
  secondary controller even ports C0h-DEh. Read permissions for controller
  registers and byte-lane ranges are unchanged.
- The owner-local route test covers a four-lane page write/read and a wide
  access crossing the page-bank end. The DMA rollback test covers an existing
  registration error, allocation failure with no partial route, and retry.
  Existing DMA channel, XT primary-only, AT paired and board tests continue
  to use the single production installer.
- A source sweep of board adapters finds no remaining DMA `t_port` callback,
  `core_machine_port_add_*` call or registration checkpoint. KBC remains
  allocated to S10, FDC to S11, and VADP/HDC/RTC/board port transactions to
  S12. Core's own private port table and CPU bus correctly retain their
  `t_port` implementation until the neutral-Core cut. DMA memory transactions
  are distinct from port registration and remain allocated to S14.

## Verification

- Focused `core-machine-dma-channel`, `core-machine-transaction-s2` and
  `core-machine-port-assembly` pass on x64 and x86.
- Both Debug build trees compile. Complete repository-only unit suites pass
  467/467 per width. An earlier concurrent x64 run had one host-window modal
  timing failure; the isolated case and complete sequential x64 rerun passed.
- The T345 source-ownership static gate and its negative self-test pass; CMake
  source-owner configuration, NXVM documentation governance and
  `git diff --check` pass.
- Four profile-specific optimized Release pairs were built and copied into
  `assets/nxvm/<profile>/`. Every x64 file is `pei-x86-64`, every x86 file is
  `pei-i386`, and none has a `.debug` section. Adjacent INIs and MyNES paths
  remain unchanged.

| Profile | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| XT 5160 | `0BE79F67B14592DC3EAA98EBF98629E9726CC3BFD60C1C45C7900E3EE5E5FE13` | `2A9AB79B0508D019B9AFD0A9A3BAF1ED5152FAF1947941F43A75529B4F610BB5` |
| AT 5170 | `16E8B57A07221BD0A110BFF0C9275A2F44EFC07744779239792E9FFC93FBA326` | `2C851544482A467B82BB5531C0292A512F944A3DD1E81CBDF4DFFA3848D57260` |
| Model 40 | `B12C5AC2EAB5AA100806EB71C5A2D7EA8238E2F903A82E05E4F809AEEBC7104F` | `8C74F0DBF666D326E93C1F87B4758545616C5E8463B32818E93B5E2CC687573E` |
| Default PC/AT | `49484CAB02472D3946A88935A5234EAD66DC7609F1B7F8BB9CE469E54B6F2E41` | `79AB01F6EBADA0A0840AC4450974C26C19D31C5F2778F049F0E8D7A6BBF6251A` |

This S makes no new L3/timing claim. The physical `x86/core` move, shared
board extraction and four-profile external integration gate remain T540 work.
