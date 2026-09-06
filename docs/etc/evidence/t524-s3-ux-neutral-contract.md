# T524 S3 UX Neutral Contract

## Boundary

`lib/ux` is a copied presentation and host-input capability.  It owns frame
validation, copied mailbox publication, native event normalization, capture,
action-chord matching, configurable routing and native presenters.  It does
not emit an x86 input protocol or own product state.

Terminal input now publishes neutral `UX_KEY_*` identities with no physical
scan transition.  `src/vm/platform/ux_binding.c` is the sole NXVM consumer
that translates those identities into its input protocol.  Native Windows
input retains its own physical transition values.  No second terminal mapper
or product-specific input route remains in `lib`.

## Public Contract

- `ux_frame_is_valid` defines the minimum non-empty, bounded text or graphics
  frame accepted by `ux_mailbox_publish`.
- A mailbox has one producer and one consumer.  Publication/capture may
  overlap; publication copies the frame; wake releases a native wait without
  advancing generation; destruction follows both participants stopping.
- An action registry has one immutable entry per key/modifier chord.  Duplicate
  and full registrations return `LIB_STATUS_INVALID_STATE`.
- `ux_binding` remains a product-provided lifecycle/action binding.  The
  library does not decide the meaning of a registered action or presentation
  target.

## Consumer And Ownership Proof

`ux-contract-smoke` is a presentation-binding consumer.  It exercises text
and graphics copied frames, invalid-frame rejection, mailbox capture,
capture state, action conflict/capacity, router selection and callback
lifecycle.  `library-consumer-conformance` is an independent public-header
consumer that creates/copies a graphics frame, uses Unicode text, mailbox
wake/generation, action matching and capture.  Neither test includes a
product or machine header.

`ux-linux-input-smoke` proves terminal keys publish neutral identities;
`vm-platform-ux-frame-smoke` proves NXVM alone maps the identity at its
binding boundary.  `ux-win32-input-smoke` retains native Windows packet
normalization coverage.  `ux-mailbox-wake-smoke` verifies the native wake
handle and non-advancing wake rule.

## Similar-Issue Sweep

The defect class was native adapter or presenter code embedding product input
or machine ownership.  The sweep covered all `src/lib/ux/**`, every
`ux_event` consumer, all frame/mailbox/action/presenter callers and their
comments.  The exact forbidden-vocabulary query returned no UX hit:

```powershell
rg -n -i "\b(softpc|mvdm|ccpu|cvid|bios|rom|guest|controller|profile|vm)\b" src/lib/ux
```

The complete `src/lib` query retains only three non-UX documentation comments
in `README.md` and `host/clock.h`; S4--S8 own their removal as part of the
whole-corpus acceptance, rather than expanding S3 into an unrelated root.
The peer-root include query found only same-root headers; no root except
`base` depends on another root.

## Verification

- `cmake --build build/mingw-gcc-x64-s8-make --target ux-contract-smoke ux-linux-input-smoke ux-win32-input-smoke vm-platform-ux-frame-smoke -j 4` and the four-test focused CTest selection: pass.
- `cmake -DLIBRARY_ROOT:PATH=src/lib -P src/lib/verify_manifest.cmake`: pass.
- `cmake -S src/lib -B build/t524-s3-lib -G "MinGW Makefiles" -DBUILD_TESTING=ON`; build and CTest: pass.
- `ctest --test-dir build/mingw-gcc-x64-s8-make -L unit -j 4 --output-on-failure`: 312/312 pass in 18.97 seconds.
- `git diff --check`: pass.
