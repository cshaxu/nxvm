# T376 S3: 8272A Deleted-Data And Control-Mark Path

`M5:T376:S3:8272A-DELETED-DATA:OK`

## Delivered Behavior

The FDC command decoder now accepts Read Deleted Data (`0Ch`) and Write
Deleted Data (`09h`) with the same nine-byte command form as ordinary read and
write. All four routes use the existing transfer state machine, DMA2 provider,
non-DMA `3F5h` servicing, result/IRQ6 completion, DOR cancellation and T375
500-kbit/s byte gates.

At each logical-sector boundary the FDC obtains an address mark through the
frozen media provider. Ordinary Read Data expects a data mark; Read Deleted
Data expects a deleted-data mark. A mismatch transfers the sector through the
same read path and records ST2 Control Mark (`40h`) in the existing seven-byte
result. Write Data establishes an ordinary mark; Write Deleted Data establishes
a deleted-data mark before its normal byte transfer. Provider refusal maps to
the retained not-writable/no-data result path.

Address-mark mutation deliberately does not advance media generation: it is a
write to mounted media, not an insertion/removal event. This prevents the
controller's disk-change detector from falsely reporting a media change after
a deleted-data write.

## Focused Proof

`core-machine-fdc-smoke` uses the existing in-memory provider, now with its
real address-mark capability. It proves ordinary read of a deleted mark sets
ST2 Control Mark, deleted read of that same sector clears it, and deleted write
persists both byte data and the deleted mark. Existing DMA, non-DMA, seek and
reset/cancellation checks remain in the same run:

```text
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T375:S20:FDC-DMA-CADENCE:OK
M5:T375:S21:FDC-SEEK-CADENCE:OK
M5:T375:S24:FDC-NDMA-CADENCE:OK
M5:T376:S3:8272A-DELETED-DATA:OK
```

## Remaining Receiver

Scan Equal, Low-or-Equal and High-or-Equal remain unimplemented. They are the
next T376 FDC owner work and must retain this provider/transfer/timing path.
