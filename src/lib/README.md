# Shared Library

`src/lib` contains product-neutral, copied-value platform capabilities. It has
no application machine pointer, runtime queue, monitor command, lifecycle
policy, or native SDK type in a public API. `MANIFEST.sha256` lists every source file and
its exact corpus revision; `verify_manifest.cmake` rejects an unlisted, stale,
or changed library file.

## Component graph

An arrow means the component on the right may use the generic contract of the
component on the left:

```text
base -> ux-base + host + storage
ux-base -> ux-window + ux-console
base -> ux-window + ux-console
```

No other component edge is allowed. In particular, `host`, `storage`,
`ux-window`, and `ux-console` are peers. Neither UX leaf includes or calls
`host`, and `host` does not include UX. The build-only `ux` aggregate links the
three split UX targets; it is not a lifecycle controller or a public unified
presenter API.

- `base` provides scalar aliases and the logical Console object. A logical
  Console is a neutral copied-value endpoint: it has no native handle, platform
  input mode, Window, VM, monitor, or product-lifecycle meaning.
- `host` exposes an opaque `host_console_broker` that binds one caller-owned
  logical Console to native I/O and provides clock/sync. A caller supplies its
  expected Current Console on every replacement or cooked-line request; host
  has no monitor, VM, prompt, or lifecycle vocabulary. A replacement first
  retires and confirms the old native reader, then activates the next binding;
  it uses the same transaction for every raw/cooked pair. If retirement cannot
  complete, no next reader starts and the broker fails closed with host-I/O
  failure rather than claiming either Console is usable.
- `storage` provides file and byte-medium primitives.
- `ux-base` provides copied frame/input values, source-local registered-hotkey
  matching, source identities, and private mailbox mechanics.
- `ux-window` owns one Window lifecycle; `ux-console` owns one VM-Console
  lifecycle and its logical Console object. Both report copied UX input only;
  neither makes product decisions.

## UX mailbox and lifetime contract

Every `ux-window` and `ux-console` instance owns a separate, private pair of
mailboxes. Callers never share or address a mailbox directly.

- The frame mailbox holds one copied frame. Publishing replaces that value:
  frames are **latest-wins**.
- The control mailbox is FIFO. It accepts up to 32 ordinary control records;
  enqueue beyond that limit returns `LIB_STATUS_LIMIT_EXCEEDED` without
  overwriting an existing record. A STOP record has one reserved FIFO slot and
  is idempotent. Once STOP is queued, a later non-STOP control request returns
  `LIB_STATUS_INVALID_STATE`. A multi-record operation is all-or-nothing:
  insufficient ordinary capacity leaves every requested record unqueued. A
  non-OK control enqueue is also reported through the component failure sink.
- A worker drains control records in FIFO order before it considers the latest
  frame. On STOP it consumes no later control or frame: it retires native
  input/output, emits exactly one `UX_EVENT_SOURCE_RETIRED`, and exits.

Each component receives a process-wide monotonic, never-reused
`source_identity`. Every `ux_input_event` carries both that identity and a
borrowed source handle. The identity, not the address, is safe to use after
component storage can be reused. `UX_EVENT_SOURCE_RETIRED` is the final
asynchronous input-lifetime fact for that identity; an application uses it to
clear source-specific pressed-input state and must not dereference the source
handle from it.

## Platform scope

The public component contracts are cross-platform. This corpus currently has
supported Win32 leaves; Linux UX leaves are intentional
`LIB_STATUS_UNSUPPORTED` placeholders, not claimed presenter implementations.
