# T391 S5: C1 Boot-Transfer Capture

`M5:T391:S5:C1-TRANSFER-CAPTURE:OK`

The existing test-only Model-40 retirement capture has a C1 transfer mode. It
ignores observations through the accepted C0A immediate port-61h endpoint,
then aggregates normalized copied records until an existing copied
`linear_pc` equals `0:7C00`. The test consumer retains only the reached
boolean; it emits no PC, instruction bytes, ROM/media identity, path, digest
or raw trace.

The canonical synthetic capture proves the C0A endpoint is excluded, two later
classified records aggregate, and the second reaches the boolean endpoint. It
also continues to run the prior C0 and C0A synthetic checks. Cold reset creates
a new capture; unallocated, form-capacity, fault and containment terminals
remain separate and cannot be C1 success.

The mode uses the existing copied observer only. No Core/VM interface,
production path, port provider, profile behavior, physical retirement, board
time, firmware compatibility or L3 claim changes.

## Transfer

A future contained owner-managed replay may now report whether the named C1
boot-transfer endpoint is reached. Its result must first receive a complete
form/context batch disposition before it can affect CPU qualification; a
containment budget alone remains non-semantic.
