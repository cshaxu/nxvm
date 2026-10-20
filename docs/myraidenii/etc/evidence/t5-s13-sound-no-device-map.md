# M1 T5 S13 — Sound No-Device Map Slot

## Evidence And Grade

The already pinned MAME Raiden II driver, revision
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff`, maps the sound-side address
`0x4004` as `noprw` between the Seibu interrupt-acknowledge registers and the
YM2151 pair. This is a board-integration observation from an external emulator,
so it is **L2**. It permits the local Sound address decoder to accept that one
read/write slot; no MAME code, data table or ROM content is imported.

The same L2 map routes both addresses `0x4008--0x4009` through the Seibu YM
read/write owner. The local decoder had incorrectly accepted only the first
read address. It now exposes its existing YM status through both mapped read
addresses. That routing fact is L2; the status value remains the local device
model already owned by Sound.

The inspected Z80 manual establishes processor instruction semantics, but does
not define the selected board's no-device read value. The local read result of
`0xff` is therefore a stable **L1** no-device model, not an L3 hardware fact or
a claim about electrical bus state. Writes have no state effect.

## Diagnosis And Ownership

S12's bounded external-set result was a Sound-owned `LIB_STATUS_UNSUPPORTED`.
For S13, a disposable debug execution tested only whether that failure reached
the L2-known `0x4004` decoder slot. It did; it emitted no guest address, opcode,
program counter, byte window, ROM member name, trace, frame or PCM payload.
The temporary staged input and debug build output were removed after the
diagnosis.

This identifies a fixed-map hole, not an unimplemented Z80 operation. The
receiver remains only `src/myraidenii/core/sound.c`: Board stays the sole time
and transport owner, and no Product, Machine, Shared component or second
execution path is involved.

## Bounded External Result

After both mapped-read fixes, the existing selected-set probe reaches its
two-million-instruction budget on both host widths rather than a Sound fault.
Both report `STOP_BUDGET`, 33,301,143 elapsed ticks and the same aggregate
frame/PCM counters. The changed terminal establishes only that this fixed-map
receiver no longer rejects the observed path. It does not qualify the image as
accurate graphics or the zero-valued copied PCM as valid audio.

## Regression Contract

The existing Core Sound smoke executes the actual Z80 read and write route:

1. a read from the no-device slot reaches the existing Seibu response sink and
   yields the chosen L1 value;
2. a write cannot change that value;
3. the existing YM device-map path reads status through the second mapped
   address; and
4. the programs halt without an unsupported decoder/map failure.

This is not a sound-fidelity, selected-game startup, audible output, desktop or
playability qualification. Any later terminal remains explicit and must be
received by its own evidence-graded owner.
