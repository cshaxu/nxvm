# T553 S4 — Four PC App Qualification

## Result

The four fixed PC Apps pass their complete registered App-labelled CTest
routes on both supported host widths.  This is an App qualification result;
it does not make a broader manual-desktop, external-firmware or hardware claim.

| App | x64 routes | x86 routes | Route composition |
| --- | ---: | ---: | --- |
| My5160 | 2/2 | 2/2 | 2 unit |
| My5170 | 6/6 | 6/6 | 6 unit |
| MyDeskPro386 | 29/29 | 29/29 | 28 unit, 1 integration |
| NXVM | 55/55 | 55/55 | 35 unit, 20 integration |
| **Total** | **92/92** | **92/92** | **71 unit, 21 integration** |

The two width-specific App selections were configured before qualification.
Their static App-boundary, App-test-support and selected-composition checks
pass. `verify-product-artifact-roots`, `verify-current-artifact-target`,
`verify-build-ownership` and `verify-documentation-governance` also pass on
both widths.

## Artifact disposition

S2 removes only a duplicated CMake link-list item.  Rebuilding the current
MyDeskPro386 artifact target reported no work, so executable bytes did not
change and no artifact pair was rewritten.  The current artifact-root and
build-ownership gates independently confirm that the deployed four-App roots
remain the selected current roots.

## Limits

The 21 registered integration routes exercise their declared local test inputs.
No separate manual desktop interaction, real external-firmware boot session, or
hardware qualification was performed by S4.
