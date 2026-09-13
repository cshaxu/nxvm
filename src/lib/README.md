# Shared Library

`src/lib` contains product-neutral, copied-value platform capabilities. It has
no application state pointer, runtime queue, monitor command, lifecycle
policy, or native SDK type in a public API. `MANIFEST.sha256` lists every source file and
its exact corpus revision; `verify_manifest.cmake` rejects an unlisted, stale,
or changed library file.

## Header visibility

Cross-component contracts live at the component root and are named
`*_interface.h`. All component `win32/` and `linux/` implementation files are
accessible only from that component's corresponding platform implementation,
not from a sibling component or a forwarding root header. The sole exception
is `types/{win32,linux}`: these shared external declarations may be included
by matching platform implementations. Application-facing
copied-value APIs are distinct from the leaf-support contracts:
`ui-base/worker_interface.h`, `mailbox_interface.h`, `mailbox_wake_interface.h`,
`ui-base/input_interface.h` serves only the UI leaves;
`console/binding_interface.h` serves host binding implementations.
Other component headers are exclusively component-local. They use short
names and live directly in their owning directory; no filename carries a
`_private`, `_internal`, or `_native` qualifier. Names describe the operation:
`clock.h`, `input.h`, and worker start/join/state. Application code may include
only application-facing `*_interface.h`, not the leaf-support contracts.
Types declaration headers are the explicit naming exception. The common types interface includes its own atomic
vocabulary helper; it never imports platform SDK headers.

Input support owns common normalization and copied-event delivery in ui-base.
Its same-signature platform operations decode keys and query physical text
layout. Window message flags and key-state queries are Window-local. External
declarations remain types-owned; actual consumers declare OS link libraries.

## Component graph

An arrow means the component on the right may use the generic contract of the
component on the left:

```text
types -> console + host + storage + ui-base + ui-window + ui-console
console -> host + ui-console
ui-base -> ui-window + ui-console
```

No other component edge is allowed. In particular, `host`, `storage`,
`ui-window`, and `ui-console` are peers. `ui-window` neither includes nor
calls `console` or `host`; `ui-console` consumes only the neutral `console`
contract. `host` does not include UI. There is no unified UI aggregate,
lifecycle controller, or public unified presenter API.

- `types` is header-only and provides scalar aliases, status values, atomic
  helpers, and typed wrappers around C-runtime/SDK vocabulary. It owns no
  compiled platform worker, component state, or I/O policy. Every component's
  selected platform source supplies the same component-private operation shape
  to its platform-neutral base source. Common types headers contain no OS
  selection; platform declaration groups live in `types/win32` and
  `types/linux`. The compiler-only atomic adaptation remains common.
- `console` provides the logical Console object. It is a neutral copied-value
  endpoint: it has no native handle, platform input mode, Window, raw Console,
  monitor, or product-lifecycle meaning.
- `host` exposes an opaque `host_console_broker` that binds one caller-owned
  logical Console to native I/O and provides clock/sync. A caller supplies its
  expected Current Console on every replacement or cooked-line request; host
  has no monitor, raw Console, prompt, or lifecycle vocabulary. A replacement first
  retires and confirms the old native reader, then activates the next binding;
  it uses the same transaction for every raw/cooked pair. If retirement cannot
  complete, no next reader starts and the broker fails closed with host-I/O
  failure rather than claiming either Console is usable.
- `storage` provides file and byte-medium primitives.
- `ui-base` provides copied frame/input values, source-local registered-hotkey
  matching, source identities, and private mailbox mechanics.
- `ui-window` owns one Window lifecycle; `ui-console` owns one raw-Console
  lifecycle and its logical Console object. Both report copied UI input only;
  neither makes product decisions.

## UI mailbox and lifetime contract

Every `ui-window` and `ui-console` instance owns a separate, private pair of
mailboxes. Callers never share or address a mailbox directly.

- The frame mailbox holds one copied frame. Publishing replaces that value:
  frames are **latest-wins**, but unconsumed dirty rectangles are unioned under
  the mailbox admission lock. Consumption takes latest complete pixels and accumulated
  damage together. Dimensions, mode or palette changes invalidate the full image.
- The control mailbox is FIFO. It accepts up to 32 ordinary control records;
  enqueue beyond that limit returns `LIB_STATUS_LIMIT_EXCEEDED` without
  overwriting an existing record. A STOP record has one reserved FIFO slot and
  is idempotent. Once STOP is queued or a terminal fault closes admission, later
  frame and non-STOP control requests return
  `LIB_STATUS_INVALID_STATE`. A multi-record operation is all-or-nothing:
  insufficient ordinary capacity leaves every requested record unqueued. A
  non-OK control enqueue is also reported through the component failure sink.
- A worker drains control records in FIFO order before it considers the latest
  frame. On STOP it consumes no later control or frame: it retires native
  input/output, emits exactly one `UI_EVENT_SOURCE_RETIRED`, and exits.
  Post-start Window failures use that same cleanup path, reporting the fault;
  creation failure is distinct and does not retire an uncreated source.

Unexpected native Console reader errors emit `LIB_CONSOLE_EVENT_IO_FAILURE`.
Before native activation, after old input quiesces, host delivers INPUT_RESET
synchronously through the logical Console. Consumers clear local input history
before the next reader starts; this is distinct from successful ACTIVATED and
permanent UI source retirement. Rollback uses the same reset-before-reader path.
Normal replacement cancellation is not failure. A UI Console reports genuine
input/output errors through its failure sink and retires; NOT_CURRENT output
is an expected inactive-object write. Neither path makes application decisions.
Broker replacement holds its transaction lock through old output-sink cleanup;
the backend output lock is released first so in-flight writes can complete.

Each component receives a process-wide monotonic, never-reused
`source_identity`. Every `ui_input_event` carries both that identity and a
borrowed source handle. The identity, not the address, is safe to use after
component storage can be reused. `UI_EVENT_SOURCE_RETIRED` is the final
asynchronous input-lifetime fact for that identity; an application uses it to
clear source-specific pressed-input state and must not dereference the source
handle from it.

## Platform scope

Linux event waits share one host-sync mutex/condition, so a waiter on multiple
events sleeps until a predicate can change instead of polling. Auto-reset
consumption occurs under the same lock. Timed waits use a monotonic deadline;
infinite waits have no deadline. Callers must join all waiters before destroying
their event objects. The shared wait primitive has process lifetime and
contains no application context. Mailbox waits use a per-mailbox monotonic
condition with the same timeout and spurious-wake semantics.

The public component contracts are cross-platform. This corpus currently has
supported Win32 leaves; Linux UI leaves are intentional
`LIB_STATUS_UNSUPPORTED` placeholders, not claimed presenter implementations.

Unrecoverable input delivery rejection closes the source immediately; the
worker wakes, quiesces input, reports the failure and retires once. Pending
keyboard replay is never retried after a partial sink failure. Mouse/close
records do not flush keyboard prefixes; only keyboard order, not key/mouse
interleaving, is retained while a prefix is pending.

Console callback/output gates and broker replacement use component-private
blocking locks; no sibling host dependency is introduced into console. Native
I/O lock ordering and detach barriers are unchanged. Callbacks must not
synchronously reenter binding replacement or destruction on the same owner.


Win32 cooked input emits only complete bounded lines. Overflow drains through
LF, emits one REJECTED_LINE, and never submits a truncated tail. Raw activation
requests native foreground/focus; cooked activation does not. The reader's
confirmed retirement, not focus, establishes the input handoff.

Console text frames use the fixed PC-display mapping documented by `console`.
Palette caches advance only after successful native palette application;
unavailable palette support may retry without terminating text output. Text
and cursor I/O failures remain explicit. Window sizing/title completion likewise
never records an unsuccessful native call as completed.
Nonempty native text writes invalidate the prior frame cache, even on partial
failure; an unchanged subsequent frame must overwrite those intervening cells.
Frame writes likewise invalidate the old cache before touching native cells;
only a successful write of the entire requested rectangle commits the cache.
Clipped native success is incomplete output and returns IO_ERROR.
Surface size is established after palette application, which can change native
buffer geometry; output does not rely on a precondition invalidated by metadata.
The broker alone sequences deactivation, including failed initial activation.
Backend disposal frees inactive resources without repeating mode restoration
or reader retirement. UI Console disposal similarly follows its worker's sink
detachment; it does not detach a second time.
