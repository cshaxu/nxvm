# T546 S14 Descriptor Query Design

## Scope

This packet repairs only the existing x86 CPU owner and its owner-local
regressions.  It changes no public interface, time formula, device wiring,
ordinary segment-load path, profile heuristic, or product path.

## Sources And Disposition

Intel's *80386 Programmer's Reference Manual*, LAR and LSL instruction
descriptions, specifies LAR's `00FxFF00H` defined-bit mask and lists the valid
system descriptor types.  It admits LAR types 1--7, 9, B, C, E and F; LSL
admits only 1--3, 9 and B.  The latter five LAR types and the latter two LSL
types are 80386 forms, so they are not admitted by the 80286 profile.

Intel's *80286 and 80287 Programmer's Reference Manual* specifies VERR/VERW
as selector-validation queries: selector invalidity produces ZF=0 rather than
a selector-derived protection exception.  The same query rule applies to
LAR/LSL.  A TI selector with an invalid LDTR is therefore a negative query,
not permission to read stale LDTR base/limit values.

Bochs 2.6 protected-mode control-transfer code was read only as a secondary
behavioral cross-check.  No external implementation or source text is
imported.  Existing LAR/LSL/VERR/VERW timing allocations have exact existing
ledger coverage and are unchanged by this semantic repair.

## One Owner And Similar-Issue Sweep

`_s_check_selector()` has exactly four callers: VERR, VERW, LAR and LSL.
They are all query-only paths, so its invalid-LDTR rejection cannot alter an
ordinary selector load.  The LAR and LSL system-type switches remain their
existing table-driven handlers; the repair makes their generation predicate
explicit instead of adding a descriptor framework or a second query path.

Queries neither set accessed nor busy bits.  Regressions retain byte-for-byte
descriptor images before and after both successful and negative queries.

## Direct Regression Matrix

- Every valid LAR and LSL system type is checked on both 80286 and 80386.
- Types 0, 8, A and D remain invalid on both profiles; an LSL gate remains
  invalid.
- LAR/LSL/VERR/VERW each receive a TI selector with an invalid LDTR whose
  stale base and limit would otherwise resolve a present descriptor.  Both
  profiles must halt normally with ZF=0; the destination remains unchanged for
  LAR/LSL.
- A 386 operand-size LAR proves the undefined high-limit nibble is cleared.
- Successful LAR/LSL and VERR/VERW queries preserve descriptor bytes.

Before the production change, the added invalid-LDTR regressions failed: stale
LDTR state was consulted, and the fixture entered shutdown while trying to
deliver the resulting fault.  After the one-owner repair, focused x64 and x86
LAR/LSL/VERR/VERW/selector tests and the complete x86-labelled suites pass;
both x86 manifests pass.  Complete dual-width unit qualification and receiving
artifact review remain required before S14 can close.

## Rebuilt Receiving Products

The repaired Shared CPU input changes every fixed PC product.  All four
profiles were rebuilt by their existing fixed-profile CMake trees; each
post-link architecture assertion passed.  The resulting optimized stripped
0.5.0546 artifact identities are:

| App | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| My5160 | `095236A7FC9D7B5D092502AEBBDE5A7AD39BBE77D591328EC962586CD2DC67F2` | `6C54FDE52D2325B6FDBE7D55B04CFFB54590B4F80207249713A3201A444D6334` |
| My5170 | `1AF7AD03F4F9E55C9BD34A2A692CE2E5B02FAA58C59A7FC1749E9E46EEFE13A5` | `AC4EDB5D290398AE855E7E9953CBA997DB38DAF7F3D416E4D62500CA7EDD6DD5` |
| MyDeskPro386 | `00A956129A8FFD19440BA7E386E59C9795FB61D26A4943B4DB81173C63131342` | `1D82D40ED90399D1002E34BB1B5DCB23DF16366BD191DEA048C523E3BDE1E75C` |
| NXVM | `80E3914BFE354C2E481804B7E207633E17EFA0A40202A376DB093885AEA19686` | `485874752F53EEDE54698A48A354C09B17D1ED7BFC8C48D87638C02236C6B573` |

MyNES is not a receiver of x86 CPU code and remains untouched.
