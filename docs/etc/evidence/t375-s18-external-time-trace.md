# T375 S18: External-Time Trace

`core_machine_advance_time()` now records one
`CORE_MACHINE_TRACE_EXTERNAL_TIME` event through the existing core trace owner.
The event carries the published batch in `value` and the post-publication
`elapsed_ticks`; CPU-retirement events remain distinct. No file recorder or
second trace mechanism was added.

Focused proof passed:

```text
M5:T375:S18:EXTERNAL-TIME-TRACE:OK
```

The existing core trace smoke also passed. Persistent trace-file format,
record/replay consumption, and all remaining board/device phase work transfer
to later T375 work.
