# T526 S12 Presentation Leaf Convergence

## Result

`vm/presentation` is the sole NXVM owner of the shared-lib binding and of the
process Console broker. It creates, leases and destroys the one active native
Console or Window leaf. `vm/product` no longer constructs, retains or calls a
Console-host object; it only parses CLI text, asks the presentation boundary to
write or read a monitor line, and formats copied lifecycle notices.

`vm/events/presentation_plan.h` is the one copied plan ABI. `vm/session`
produces it, and `vm_presentation_apply_plan()` consumes target, title, mouse
capture/release and latest-frame fields in one route. Presentation input and
cooked Console lines return through the session FIFO's value sinks. The leaf
does not include or retain a `vm/session` object, a Core object or a native SDK
handle in its public interface.

The former product presentation, frame and Console-host files, their aliases,
and their mirrored smoke files are deleted. There is no compatibility route.

## Verification

- Focused presentation and Console checks: 4/4 passed.
- Full repository unit suite: 299/299 passed.
- `verify-current-specialized-gates`: 67/67 passed, including dependency DAG,
  single-session, lifecycle, build-ownership and test-route gates.
- Documentation governance and `git diff --check` passed.
- `vm-0-5-0526` built as stripped x64 and x86 Release artifacts.
