# T523 S1 Cross-Product Compatibility Ledger

## Scope and Method

Baseline: `dbb0a4ea` (T522 closure). This ledger covers every tracked source
under `src/lib`, its public callers and build/test registration. The
similar-issue sweep is:

```powershell
rg -n '#include.*(type\.h|core/|vm/|profile/)|core_machine|vm_session|vm_profile' src/lib test CMakeLists.txt
rg -n 'Sleep\(|WaitFor.*250|MsgWaitForMultipleObjects.*16|frame->graphics' src/lib
rg -n 'ntvdm64|nxvm|\.tmp\.000|\.bak' src/lib
```

SoftPC is not imported or derived by this task. Its future adoption is bounded
to copying the manifest-selected source unchanged and supplying a binding
outside its preserved machine implementation. No current local SoftPC checkout
was available to inventory; this is a product-local receiver, not a missing
library ABI source.

## Frozen Corpus and Dispositions

| Surface | Current state | Disposition and receiver |
| --- | --- | --- |
| `lib/base` | Absent; every library public header includes NXVM `type.h`. | S2 creates a library-only fixed-width/status/atomic facade. Products adapt at their bindings. |
| `ux/frame` | Fixed text/graphics bounds; no graphics stride, dynamic text geometry or full cursor shape/phase. | S3 replaces it with one copied, complete value contract. |
| `ux/event` and actions | Basic relative mouse/key data; action enum names NXVM guest commands. | S3 uses normalized make/break/Unicode/mouse/release values and product-registered opaque actions/callbacks. |
| `ux/mailbox` | Generation/spin publication; native loops have fixed waits. | S2 gives one latest-copied-frame mailbox a host event wake; no polling fallback remains. |
| `ux/router` | Chooses Console/Window from `frame->graphics`. | S3 makes only explicit product requests select/open/close presenters. |
| `ux` native loops | Win32 Console waits up to 250 ms; Window has a 16 ms timer/message wait. | S2/S3 wait only for message, frame-wake or explicit product request; native handles remain private. |
| `host/sync` | Sleep/yield and a 20 ms polling helper, no event/cancel/join contract. | S4 replaces it with value/callback contracts and Win32/Linux-private mechanics. |
| `session` | Start/stop forwarding only. | S4 supplies generic serialized lifecycle and acknowledgement; product callback retains machine turns/safe points/result. |
| `storage` | Direct-readonly and overlay only; temporary suffixes name `ntvdm64`. | S4 supplies direct-writable, explicit commit/discard and neutral private names; product owns topology and guest semantics. |
| `observability` | Bounded start result only. | S4 adds only copied lifecycle/presenter/input/media/stop facts; no trace/debugger. |
| Top-level root coupling | `ux`, `host`, `session`, `storage`, `observability` currently do not include one another. | Retain and enforce as a static S5 conformance gate. |
| Product/Core leakage | No `core_machine`, `vm_session` or `vm_profile` hit in `src/lib`; all public headers nevertheless use `type.h`. | Retain no machine leak; S2 eliminates `type.h` from the public corpus. |
| Manifest | `MANIFEST.sha256` lists UX only. | S5 publishes the full source-identical corpus and neutral consumer build. |

## Binding Boundary

The library public ABI permits only copied values, opaque library-owned handles
and product callbacks. A product binding adapts its local session/machine types
into callback context privately; no public library structure contains a
product, machine, VM, profile, controller, renderer or guest-memory pointer.

NXVM binds existing session turns, debugger policy and VADP snapshots outside
`src/lib`. SoftPC will bind its own lifecycle, renderer snapshots and action
policy outside the copied corpus. Neither binding is a library component and
neither product links against the other.

## S1 Completion Predicate

S1 is complete when the table has a disposition for every proposed shared
surface and no stop condition is met. S2 consumes the first four rows only:
portable base, frame-neutral mailbox wake, native wait removal and their
static/product conformance proof.

## Verification

The initial CTest tree had two registered but missing Win32 UX executables.
`cmake --build build/mingw-gcc-x64 -j 4` restored the configured tree; the two
tests pass serially, and a later full `ctest --test-dir build/mingw-gcc-x64 -L
unit --output-on-failure -j 4` passes `308/308`. The final `LastTest.log` has
no `Test Failed`, `Not Run`, or missing-executable marker. Documentation
governance and `git diff --check` pass.
