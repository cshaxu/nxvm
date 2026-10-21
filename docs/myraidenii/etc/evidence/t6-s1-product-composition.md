# M1 T6 S1 — Product Composition Foundation

## Delivered Boundary

S1 adds the one MyRaidenII Product composition owner.  It consumes the
already parsed adjacent INI and its copied INI-directory fact, gives the
existing Machine image transaction the one resolution base, builds the
existing copied Core Board, creates its existing PCM stream and Machine
adapter, and supplies that one driver to Emulator Product.

The construction/teardown order is:

```text
Product config -> Machine images -> Core Board -> PCM stream -> Machine
    -> Emulator Product -> Emulator Machine/Session/UI

UI -> Session -> Emulator Machine shutdown -> Product unbind
    -> Machine -> PCM stream -> Core Board
```

The image bundle is released immediately after successful Board construction,
because Board copies its supplied configuration.  Product never opens an image
directly and Core never receives a path.

## Failure Contract

- A failed image transaction creates no Board, stream, Machine, Session or UI.
- A failed Board or stream step releases only prior Product-owned values.
- A failed Machine create destroys the still Product-owned stream and Board.
- A failed driver-descriptor request preserves that original error after a
  successful reverse cleanup; a cleanup failure remains terminal and retains
  its ownership as required by the existing component contract.
- The configuration parser's text-only entry has no valid INI location; the
  runtime loader alone supplies the copied directory.  Composition therefore
  never resolves image paths against the process working directory.

## Presentation Boundary

The existing Emulator Product remains the sole Session/UI/monitor owner.
S1 injects its shared monitor callbacks and leaves save, load and debug
honestly unsupported until their product capabilities exist.  The initial
MyRaidenII driver emits graphics only.  A requested `display=console` is
therefore explicitly rejected at composition rather than silently substituting
Window or pretending a text console can display gameplay.

The Product-local identity follows the approved two-line form:

```text
MyRaidenII [0.0.0006]
Copyright (C) 2026 Xu Ha.
```

Emulator Product remains the sole owner of the shared `Built on` framing.

## Evidence Grades

This task adds no hardware-model fact.  The fixed external image set remains
the prior asset identity contract, not an L3 device claim.  The governing
grade rule remains: an actually inspected primary-manual PDF must state the
applicable formula, constant or rule for L3; reference/range evidence is L2;
unsupported local policy is L1.

## Verification

- x64: `build-unit-tests` required no additional work; `ctest -L myraidenii`
  passed **12/12**.
- x86: `build-unit-tests` required no additional work; `ctest -L myraidenii`
  passed **12/12**.
- Both widths built the Product library and the focused configuration and
  composition tests successfully.
- `Verify-DocumentationGovernance.ps1 -Product myraidenii` and
  `git diff --check` passed.

No external ROM was opened, no desktop/RDP run occurred, and no EXE was
created or claimed.  S1 is a composition foundation, not a runnable-product
or gameplay qualification.
