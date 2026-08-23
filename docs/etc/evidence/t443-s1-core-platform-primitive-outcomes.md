# M5 T443 S1 Core Platform Primitive Initialization And Outcome Contract

## Scope And Result

T443 repairs the Core presentation mailbox's initialization and status
boundary without adding a display layer, fallback frame, callback wrapper, or
public ABI.

- The mailbox initializes its only `atomic_flag` with C11 `ATOMIC_FLAG_INIT`
  before any flag operation.
- All five production capture consumers stop before using their local frame
  when `core_platform_presentation_mailbox_capture` is not `OK`: Linux console,
  Win32 window set/paint, and Win32 console set/paint.
- VM display composition now has one publish tail for text and indexed frames.
  It offers the next generation to the mailbox and commits
  `display_generation` only after successful publication.

Mailbox publish/capture already preserve the prior frame by writing only while
the mailbox is active. Renderer failure does not need a second prior-frame
copy: it simply returns, leaving host policy outside Core.

## Focused Proof

`core-platform-presentation-mailbox-smoke` passed and emitted:

```
M5:T443:S1:MAILBOX-INITIALIZATION-AND-OUTCOMES:OK
```

It proves initialized publication/capture and that an inactive capture returns
`INVALID_STATE` without changing caller-provided frame storage.

`vm-display-composition-s5-smoke` passed and emitted:

```
M5:T443:S1:DISPLAY-PUBLISH-FAILURE:OK
```

It finalizes the mailbox, invokes the normal VM publisher, and proves failed
publication leaves `session->display_generation` unchanged and capture remains
`INVALID_STATE`.

## Similar-Issue Sweep

| Search shape | Production hits and disposition |
| --- | --- |
| `atomic_flag` | One Core mailbox lock. Fixed with `ATOMIC_FLAG_INIT`; no other production atomic flag exists. |
| `presentation_mailbox_capture` | Five renderer consumers. All now test the returned status before any local frame read. Core's capture function is the state owner, not a consumer. |
| `presentation_mailbox_publish` | One Core owner plus the two former VM composition branches. The branches were merged into one publication path that commits generation only on `OK`. |

No unrelated test-only capture calls were changed: their own assertions already
check status or are diagnostic reads outside production rendering. No helper
was introduced; the publish tail deleted the duplicated publication/generation
sequence instead.

## Change Accounting And Delivery

Production code is `+35/-36` lines (net `-1`); direct smoke coverage is
`+14/-1`. The only new source statement has the non-duplicable responsibility
of C11 flag initialization. The host renderers add only their required status
guards, and the VM publisher removes one duplicate publication path.

Focused commands:

```
cmake --build build\\mingw-gcc-x64 --target core-platform-presentation-mailbox-smoke vm-display-composition-s5-smoke
ctest --test-dir build\\mingw-gcc-x64 -V -R current.core-platform-presentation-mailbox-smoke
ctest --test-dir build\\mingw-gcc-x64 -V -R current.vm-display-composition-s5-smoke
```

The current developer artifact is `vm-0-5-0443` at
`build/output/nxvm_0_5_0443.exe`, SHA-256
`16BE1A019DFB61D9E28EFD9396E70134F6FA1834FC72365577E49C6BA2D1EC40`.

The Win32 palette renderer smoke also passed. Documentation governance passed
for 0443. The complete current gate reached the existing independent T344
historical-fixture failure (71 expected direct machine constructors, 75 found)
after the 0443 artifact-truth check passed.

The current-fast aggregate additionally reproduced 20 independent CPU-timing,
planar-parity, RTC, DMA/RTC, and control-transfer smoke failures; the focused
80186 timing-ledger replay also fails. T443 changes no `src/core/machine/`
production file, no CPU/controller test, and no timing fixture. The two T443
smokes and the Win32 renderer smoke pass in the same configured build. These
pre-existing failures are recorded rather than repaired here because doing so
would expand this platform-mailbox task into unrelated CPU/controller work.
