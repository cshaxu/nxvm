# types

`types` is the root component. It provides scalar aliases, status values,
portable atomic helpers, and one-to-one C-runtime or SDK vocabulary wrappers.
Every other component may depend on it; it has no component dependency,
resource policy, platform worker, or product behavior.

`types` is header-only. It centralizes external declarations and typed,
one-to-one wrappers, but does not compile a platform implementation. Each
owning component supplies its own selected `win32` or `linux` source behind a
uniform component-private contract: `host` owns synchronization and Console
native work, `storage` owns file native work, and `ui-base`/the UI leaves own
their own wake, input, and rendering work. `types` never interprets a
consumer's state machine or input protocol.

The layout has one rule:

- Top-level headers contain only common scalar/status, ISO C library, or
  compiler atomic vocabulary. They never select an operating system.
- `win32/` contains the used Windows file, clock, input, sync, Console and
  Window declaration groups. `linux/` contains the used POSIX file, clock and
  sync declaration groups. Platform sources explicitly include the matching
  group; no common header selects one and no platform header dispatches again.
- `atomic.h` may select MSVC intrinsics versus C atomics by `_MSC_VER`.
  This compiler-only exception does not select component behavior.

Platform headers preserve SDK/POSIX signatures. Windows groups include the
SDK's common windows.h declaration source; repeated guarded SDK includes do
not create implementations or independent types. The groups name usage, not
a promise that windows.h hides unrelated SDK declarations.

Consumers use the `lib_win32_*`, `lib_linux_*` and `lib_c_*` aliases rather
than raw external identifiers. `win32/scalar.h` owns shared SDK scalar and
handle aliases; purpose headers own the corresponding SDK operations and
records. Aliases preserve the original calling convention, return value and
error behavior. They do not add component semantics. The layout gate derives
forbidden raw tokens from these declarations and checks every consumer.

The number of consumers does not decide ownership: even a single-use original
SDK definition belongs here. Conversely, Console statuses, UI modifier
mapping, storage open policy and worker state belong to their components.
`lib_text_length` has the same non-null precondition as `strlen`; nullable
input is validated by its consumer, not silently reinterpreted here.

`file.h` contains common C stream declarations. `win32/file.h` and
`linux/file.h` add their platform declarations. File operations and ownership
remain in storage. Counter validation/composition belongs to host; input
normalization belongs to ui-base and Window key-state queries to ui-window.
There is no runtime layer or zero-result
fallback pretending to implement another platform's input query.
