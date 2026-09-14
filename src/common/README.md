# Common corpus

`src/common` is the product-neutral virtual-machine coordination corpus.  It
uses public `lib` contracts and accepts product behavior only through copied
options and injected callbacks.  It never includes app, host, or MVDM source.
Common has no platform directories, platform types, native calls or OS-selected
implementations. Host owns blocking mutex/event/task/wait; Types owns the atomic
vocabulary. Common retains queue, lifecycle and frame-publication ownership.

Machine snapshot reads specify the expected run and leave the destination
unchanged on rejection. Session treats frame events as publication hints and
derives content, sequence and graphics/text routing from that one snapshot,
which may be newer than the notification. UI owns the shared action vocabulary;
session does not maintain a second action enum or mapping.
UI APIs have one control-thread caller. Immutable creation options are shared
with input callbacks; their run-generation tag uses a Types atomic. Frame
deduplication and object ownership remain control-thread-local. Native producers
must be destroyed before the event sink/context is released.

| Component | One responsibility | Public contract |
| --- | --- | --- |
| `machine` | executor, lifecycle/input queues, frame publication, optional paused debug adapter | `machine_interface.h` |
| `session` | one control queue, completed-fact reduction and dispatch | `session_interface.h` |
| interaction owner | monitor logical Console, broker and KVM composition | its root public contract |
| `xasm32` | copied byte/text assembly and disassembly | `xasm32_interface.h` |
| `debug` | generic debug command engine using the optional machine adapter | `debug_interface.h` |

`session` calls `machine` and `ui`.  `debug` calls `machine` and `xasm32`.
No other common component edge is permitted.  Application and host code may
compose only the root `*_interface.h` contracts; implementation headers and
source files remain component-local. Importing products decide whether to inject
an adapter or expose a command. SoftPC now selects debug through its app CLI
binding; neither component creates a product execution or Console path.
The machine debug contract is synchronous to the control-thread caller and
serviced by the existing paused executor. Disassembly reports instruction byte
count separately from text length; callers must not use text length as a PC step.

## Independent verification

Transfer src/common, src/lib, test/common and test/lib unchanged. No importing
product sources, configuration or resources are needed:

```text
cmake -S test/common -B build/common
cmake --build build/common
cmake --build build/common --target common-verify
ctest --test-dir build/common --output-on-failure
```

Production sources/verification and the external test suite have separate
LF-normalized MANIFEST.sha256 files. Verification rejects missing, extra,
changed or duplicate entries. The source/build DAG gate rejects platform
bypasses and sibling private includes; its negative tests live in test/common.
The source-only build still accepts an explicit COMMON_LIB_ROOT when configured
directly. It does not register or carry test code.
Common synchronization tests run against actual Host primitives without sleeps.
Platform backend availability does not imply complete native display parity.
