# M5 T531 S8 P1 NXVM Console-Control receiver

## Scope and result

This P implements the NXVM half of the shared `display` / `console_control`
presentation contract.  The reference behavior is the SoftPC plan: display
`window` always uses a Window while running; display `console` uses the raw
guest Console for text, and uses a Window for graphics.  In the latter case,
`console_control=0` keeps the raw guest Console current and
`console_control=1` keeps the cooked monitor current.  A paused machine keeps
its Window when one is required; stopped or hidden presentation releases all
surfaces through the same ordered reconciliation path.

`console_control` is an optional root YAML scalar.  Only `0` and `1` are
accepted; absence is the compatible `0` default.  The parsed value is copied
once into `vm_session_request`, then passed once at profile composition to
Common Session.  NXVM does not derive the text/graphics policy itself.

## Sole-owner route

```
session YAML -> vm_session_request (copied) -> Common Session policy
    -> Common UI action plan -> existing Lib Console/Window leaves
```

- `common/session` owns policy facts, last frame class, lifecycle-sensitive
  surface selection, and ordered raw-Console/Window reconciliation.
- `vm/product` owns only YAML reading, profile composition and the product
  decision to pause on a Window-close event.  It asks Common Session to hide
  presentation; it no longer selects target `NONE` directly.
- Native handles and product callbacks do not enter Common.
- The retained `common_session_set_target()` is exercised only by legacy
  Common-contract tests and the neutral conformance fixture.  Production NXVM
  uses `common_session_set_presentation_policy()` exclusively.

## Regression evidence

- Catalog smoke accepts `console_control: 1`, rejects `2`, and proves omitted
  fields remain `0`: `M5:T515:S3:SESSION-CATALOG-PARSE:OK`.
- Common-session smoke covers Console/Window, text/graphics,
  running/paused/stopped, and both values.  Its copied `ui_frame` fixture is
  static because the public max graphics frame is about 1 MiB; this prevents a
  Windows default-stack overflow without changing production behavior.
- Focused rebuilt executable results: `common-session-smoke=0`,
  `vm-product-session-catalog-smoke=0`.
- `cmake -DCORPUS_ROOT:PATH=src/common -DCORPUS_NAME:STRING=Common -P
  src/common/verify_manifest.cmake`: pass.
- Complete repository-only unit suite: 299/299 pass under CTest with eight
  workers (the registered `common-adapter-conformance` target was built before
  the final run).

## Remaining receiver

This is intentionally not a two-product acceptance claim.  Read-only SoftPC
at `4c06235` still lacks the real paused Common Debug machine adapter recorded
by S7.  S8 remains active until SoftPC supplies that adapter and both products
are verified against the same Common corpus; this P introduces no temporary
stub or NXVM-specific substitute.
