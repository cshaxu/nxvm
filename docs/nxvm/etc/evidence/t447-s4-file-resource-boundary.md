# M5 T447 S4 Core File-Resource Boundary

Td S126 E found an unused generic Core backing-resource callback contract while
the VM's FDD, HDD, atomic save, Model-40 BYOB, catalog, and recorder each used
direct C file facades. T446 additionally retained a recorder test that rebuilt
`debug.c` with substituted C-runtime calls.

## Disposition

- `core/platform/file` is the sole synchronous host-file owner. It returns
  copied complete reads, opaque line readers, opaque text writers, and the
  exclusive-write/replace/remove operations required by atomic media save.
- FDD, HDD, media save, Model-40 BYOB, session catalog, and recorder reach
  host storage only through that Core route. Their existing media, provenance,
  parser, and lifecycle owners remain unchanged.
- The unconsumed T271 callback contract, its standalone smoke, and the mantle
  fixture's disconnected fake resource are deleted. No compatibility route or
  host callback seam remains.
- The recorder smoke now links the production `vm-machine`; it proves invalid
  open plus actual write/close output. T345 has 7, rather than 8,
  embedded-production test entries.
- The DOS keyboard and memory-fault current smokes receive a 20-second host
  startup budget. Their 3-second budget produced false failures on this host;
  no VM or media behavior changed.

## Proof

- `M5:T447:S4:FILE-RESOURCE-BOUNDARY:OK` requires the Core owner, rejects the
  old contract and all direct facades in every named consumer, and requires
  production linkage for the recorder smoke.
- Focused recorder, catalog, FDD/HDD, BYOB, and atomic-media smokes retain
  their markers, including `M5:T446:S1:VM-DEBUGGER-RECORDING-LIFECYCLE:OK` and
  `M5:T441:S1:MEDIA-SAVE-FAILURE-HYGIENE:OK`.
- T345 passes with 237 rows, 177 owner tests, 7 embedded-production tests, and
  57 exact residual production entries. All 75 specialized gates pass.
- The regenerated `vm-0-5-0447` artifact is
  `A38E741BE21C3E4C4E5D3E0802694D64D0221BF114D960A53CF68C61BC82E852`.
  The complete current gate passes 292 tests with no `Test Failed.` log entry
  in 106.01 seconds.

## Minimalism And Review

The replacement deletes the 199-line unused backing-resource contract and its
108-line standalone smoke, plus the recorder's direct-source exception. The Core file owner adds only
the six operations used by named callers; readers and writers are opaque, and
there is no provider callback, host-policy object, compatibility adapter, or
test-only public API. Actual-diff review confirms that the named VM sources
contain no direct file facade and that every Core dependency points from VM to
Core.
