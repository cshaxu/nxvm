# M1 T3: Final Shared Refresh Import

## Scope And Convergence

Owner approved a final unchanged four-root refresh and M1 closure, followed by
M2 App/Core design. Source is fixed SoftPC
`0fb40f482f5aa81464e46e360d0b3f26853ba0ae`. The frozen universe is 180
files: Lib 91, Common 23, Lib tests 46, Common tests 20. All four source and
destination inventories match; direct binary Git-blob SHA-256 verification
passes for every row of the [ledger](../etc/provenance/softpc-shared-units.csv).
No local shared-root delta, additional root, license exception or sibling write.

## Review And Verification

Reviewed all eleven upstream changed paths, including four manifests. The seven
code/test changes move Common's run generations onto unsigned atomic operations,
put the unsigned integer limit in Lib vocabulary, and test maximum unsigned
generation round trips. Common no longer directly includes limits.h. The MSVC
branch is preserved but not certified by these GCC runs. No new dependency or
independent notice was found; the existing owner MIT authorization applies.

Both Windows aggregate suites passed 62/62. Each architecture independently built
src/lib and src/common, then passed Lib 44/44 and Common 18/18. The
[case ledger](../etc/evidence/m1-t3-tests.csv) reconciles all 62 named cases
against four actual aggregate/independent logs. Three desktop cases are included;
Linux-native execution and real NES gameplay are not claimed. Standalone runner
checks pointer width, C11 and Werror; Release tests retain assertions. Commands
are shared-x64/shared-x86 configure/build/CTest presets and the standalone runner
for each architecture. The existing finite 180-second static-layout allowance is
retained; no failing case was excluded. Documentation governance and diff checks
are required before delivery.

Git numstat for changed shared .c/.h/.cmake/CMakeLists.txt paths: 7 files,
+52/-17, net +35. Positive growth is the upstream unsigned atomic
vocabulary and regression cases, not a second runtime mechanism. Production
state ownership remains Common machine/UI; no MyNes-specific shared code exists.

## M1 Closure Boundary

The owner explicitly replaced the former shared reimplementation plan with
unchanged upstream adoption. The revised Roadmap exit is met by the frozen
identity/rights ledger, native aggregate suites and four independent build roots.
Former neutral-debug, lifecycle, input/presentation and transfer candidates are
withdrawn as separate MyNes shared-development work; their old proposals remain
in Git history. This does not certify future NES adapters, exhaustive host
behavior, or receiver integration. Those have real product acceptance owners in
M2 (execution/loading/debug), M3 (dual display/input), M4 (pacing/audio), and M5
(persistence/release). M1 has no remaining source work under the revised scope.
Implementation acceptance and M1 closure are recorded by the following P.

## Coordinator Acceptance And Cleanup

Reviewed actual implementation commit `9d3142d`, its eleven-path source delta,
180 unchanged blob identities, per-case proof and owner-approved milestone
boundary. No outstanding in-scope defect or unplanned debt was found. Accept S1,
T3 and M1. Retired candidates are preserved as supporting historical evidence,
not reassigned numeric tasks. The ten named build trees and two T3 logs are
removed after their results are retained; no unrelated files are cleaned.
No remote is configured, so both deliveries use the approved local-only rule.
