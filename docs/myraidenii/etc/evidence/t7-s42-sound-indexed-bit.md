# T7 S42 Sound Indexed CB Transform

## Boundary

After S41, the selected-set qualification reached a later deterministic Sound
receiver. A temporary category-only diagnostic identified the existing
DD/FD-CB branch in the sole Sound executor. The diagnostic was removed before
this change; it retained no guest instruction, address, byte or media payload.

## Local Model

Ordinary CB and indexed CB now share one private transform helper. Indexed CB
continues to use its existing sequence: read the indexed byte, update the
existing flag owner, write the result back through the existing Sound memory
route and copy it to a selected register when the instruction form requires
that copy. No new decoder, bus, device route, state owner, queue or public
contract exists.

The helper covers the finite rotate/shift family, including SLL. The Zilog
manual covers the documented CB family; SLL is an L2 compatibility rule based
on independently observed emulator behaviour, not a claim of silicon-exact
documentation. Its result is the left-shifted input with bit zero set. The
implementation does not retain any external source, table or ROM data.

## Regression Scope

The existing ROM-free Sound smoke drives its real instruction executor through
two indexed transform cases. It proves indexed memory write-back and the
optional register-copy result, including a chained second transform that
depends on the first memory write. This extends the existing smoke rather than
adding another decoder fixture.

## Verification To Date

- Focused Sound smoke passes on x64 and x86.
- The complete MyRaidenII unit label passes 12/12 on x64 and 12/12 on x86.
- The bounded selected-set Coin/Start Probe passes at 2,000,000 and 8,000,000
  instructions on both widths after the change. Both widths produce matching
  evolving non-black frame samples and active Sound/Input/Video observations.

The post-change host execution channel cannot complete the earlier long
receiver-crossing budget within its single bounded invocation. The 8,000,000
instruction Probe is therefore a regression check, not evidence that the new
receiver has been crossed or that gameplay/release qualification is complete.
Each wrapper invocation removed its external staging directory.
