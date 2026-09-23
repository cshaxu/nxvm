# types

`types` is the root component. It provides scalar aliases, generic status
values, portable atomic helpers, and one-to-one C-runtime or SDK vocabulary
wrappers.
Every other component may depend on it; it has no component dependency,
resource policy, platform worker, or product behavior.

`types` is header-only. It centralizes external declarations and typed,
one-to-one wrappers, but does not compile a platform implementation. Each
owning component supplies its own selected `win32` or `linux` source behind a
uniform component-private contract: `base` owns synchronization; `console-broker` owns Console
native work, `storage` owns file native work, `audio` owns native PCM output,
and `kvm-base`/the KVM leaves own their own wake, input, and rendering work. `types` never interprets a
consumer's state machine or input protocol.

The layout has one rule:

- Top-level headers contain only common scalar/status, ISO C library, or
  compiler atomic vocabulary. They never select an operating system.
- `win32/` contains the used Windows file, clock, input, sync, Console, Window and
  audio declaration groups. `linux/` contains the used POSIX file, clock and
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
SDK definition belongs here. Conversely, Console statuses, KVM modifier
mapping, storage open policy and worker state belong to their components.
`lib_text_length` has the same non-null precondition as `strlen`; nullable
input is validated by its consumer, not silently reinterpreted here.

`file.h` contains common C stream declarations. `win32/file.h` and
`linux/file.h` add their platform declarations. File operations and ownership
remain in storage. Counter validation/composition belongs to base; input
normalization belongs to kvm-base and Window key-state queries to kvm-window.
There is no runtime layer or zero-result
fallback pretending to implement another platform's input query.

The common scalar vocabulary includes character and integer aliases, `lib_f64`,
`lib_iptr`, and `lib_uptr`. `void` remains direct ISO C because it has no
representation or platform contract to abstract. `lib_uptr` has an explicit
object-pointer round trip and an atomic counterpart for identity tokens and
lock-free publication. `types` does not define a product pointer policy.

Every public status is a portable outcome: invalid argument or state,
unsupported operation, allocation failure, I/O failure, internal error, or a
generic limit. Callers must translate a machine-specific detail into one of
these outcomes; a stale asynchronous delivery is a successful no-op, not a
public product-specific status.

`types_interface.h` owns memory, text, allocation, and character-class
vocabulary. `file.h` owns ISO C streams and formatted-output vocabulary.
These wrappers preserve C runtime behavior, including the unsigned-character
precondition for character classification; they do not add validation or
ownership rules.
