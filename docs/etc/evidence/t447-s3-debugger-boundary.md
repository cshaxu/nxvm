# M5 T447 S3 Debugger Boundary

Td S126 D found that the public Core debugger header exposed mutable parser,
command, target, input, and cursor state which VM embedded directly.

## Disposition

- Core now owns the complete interpreter layout behind `core_product_debugger`.
  Its only lifecycle operations are create, run, and destroy.
- Each VM session owns one opaque debugger handle, creates it after successful
  Core-machine construction, and destroys it before that machine is destroyed.
  Default PC/AT and Model-40 use the same route.
- The console adapter keeps its pause/wait behavior and invokes the retained
  single Core interpreter with the same target, input provider, and wait scope.
  No command grammar or debugger UX path was added.
- The input-failure smoke no longer reads private parser fields; it proves
  repeatable invocation and allocation failure through public behavior. The
  two-session test retains VM-machine isolation without cross-owner debugger
  cursor inspection.

## Proof

- `M5:T447:S3:DEBUGGER-BOUNDARY:OK` rejects the public layout, VM embedded
  context, and old interpreter entry point, while requiring the opaque API and
  VM invocation.
- Focused debugger-input-failure and two-session-isolation smokes pass.
- All 73 specialized gates pass after updating the T446 lifecycle check to
  require the VM-owned opaque handle rather than the retired embedded layout.
- Full `current-gate` completes 293 tests with no failed-test log in `112.48`
  seconds.
- Current artifact `nxvm_0_5_0447.exe` SHA-256 is
  `D3473D442BADAE8A7102D16B55FC1BEBECDDA477DD8C4BCB0A435FE391F73568`.

## Minimalism

There is one interpreter and one VM-held handle per session; Core remains the
sole owner of every mutable interpreter field. The implementation/build/test
change is +117/-55 lines before this evidence: the added lifecycle is the
smallest explicit replacement for a publicly embedded 19-field layout, with
no compatibility alias or forwarding wrapper.
