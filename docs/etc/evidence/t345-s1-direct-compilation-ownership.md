# T345 S1 Direct-Compilation Ownership And Warning Baseline

## Fixed Input

T345 consumes the generated T344 direct-compilation matrix after a fresh GCC
configure.  Its fixed scope is 305 direct source/target rows: 130 retained
strict and 175 deferred.  The 175 deferred rows are classified mechanically
by `verify-t345-deferred-direct-ownership` as follows.

| Ownership class | Rows | Admission mechanism |
| --- | ---: | --- |
| Project-owned owner-test source | 121 | T345 S2 owner-test strict cohort |
| Embedded production source in an owner-test executable | 6 | T345 S3 production-owner warning remediation |
| Type-foundation production source | 1 | T345 S3 type-facade warning remediation |
| Mixed or inherited production source | 47 | T345 S3 ownership separation and warning remediation |

The categories intentionally distinguish a test executable from the source it
directly compiles.  The six embedded rows are not promoted by adding strict
flags to their smoke targets, because doing so would claim inherited product
source coverage without resolving its warning baseline.

## Warning-Baseline Method

`audit-t345-deferred-direct-warnings` obtains each source's actual Ninja
command for its owning target and reruns that direct compile with
`-Wall -Wextra -Wpedantic` in the configured build directory.  It writes the
reproducible build-local file `t345-deferred-direct-warning-baseline.txt` with
one row per deferred target/source pair, its ownership class/mechanism,
compile result, warning count, and warning-option classes.  It deliberately
does not add `-Werror`, alter a committed target, or turn the audit command
into a claim that a source is strict.

## First Whole-Tree Observation Outside The Fixed Matrix

An exploratory `all` build of the separate audit configuration stopped at
`tests/platform/vm_request_bridge_smoke.c`: it still names the removed
`VM_PLATFORM_REQUEST_KEY_PRESS` enum and `key_press` union member.  This
target is neither production nor current-gate and is therefore outside T344's
305-row input.  It is not folded into the 175-row quality claim.  T345 must
transfer it as an exact dormant-target interface-drift debt unless the owner
expands the active task to repair that test contract.

The same incomplete audit run observed warning clusters in inherited/mixed
CPU executor, xasm/debug, console, firmware/session, Win32, and several
owner-test smokes.  Those observations guide later cohorts but are not an
excuse to alter runtime source during S1.

## Completed Direct Warning Baseline

The audit ran all 175 source/target pairs from actual Ninja commands.  Every
direct invocation completed without a compile error when warning options were
enabled without `-Werror`.  Its generated baseline records 225 warnings:

| Ownership class | Rows | Warning count | Rows with warnings |
| --- | ---: | ---: | ---: |
| Project-owned owner-test source | 121 | 18 | 11 |
| Embedded production source in owner-test executable | 6 | 67 | 4 |
| Type-foundation production source | 1 | 0 | 0 |
| Mixed or inherited production source | 47 | 140 | 9 |

The warning families are bounded and searchable: unused parameters/functions
or variables, implicit fallthrough, missing field initializers, empty bodies,
signed comparisons, parentheses/indentation, labels, enum comparison, and one
overlength string.  The largest clusters are inherited CPU executor,
xasm/debug, and Console sources; owner-test warnings are limited to eleven
sources.  Later S units must use this baseline to group a genuinely common
owner mechanism, rather than applying per-file warning suppressions.
