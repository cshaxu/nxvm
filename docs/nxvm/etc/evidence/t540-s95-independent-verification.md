# T540 S95 Independent Component Verification

Consume S94's reviewed tree; this verifies the receiver, not implementation
acceptance or product boot behavior. No partial P is delivered before S97.

## Configuration And Graph

Use the standalone test/x86 project, Debug, Ninja, eight jobs, sequential width
suites. Tools-on trees are build/t540-s4-shared-x64 and
build/t540-s4-shared-x86; tools-off trees are
build/t540-s93-shared-no-tools-x64 and build/t540-s93-shared-no-tools-x86.
The x64 compiler reports x86_64-w64-mingw32; x86 uses the explicit mingw32 GCC.

Read the actual source/test CMake and complete corpus/negative verifiers.
Neutral Core links Types, CPU and FPU, not IBM-PC wiring. Board composition
selects family/chip owners but deliberately does not select a Core variant;
the test/product composition selects one actual Core. Observation variants
compile the same sources for trace-contract tests, not a second implementation.
Actual Ninja queries contain 753 commands per tools-on width and 700 per
tools-off width, each with zero App source/test references. The x86 compiler
reports i686-w64-mingw32. Both tools-off configurations regenerate against
the current source; their complete generated test names differ from tools-on
only by the six existing optional cases below (298 versus 292).

## Current Execution

- Tools-on x64: independent build succeeds; all 298 registered cases pass in
  151.03s (294 unit-labelled plus four corpus/manifest/negative checks).
- Tools-on x86: independent build succeeds; all 298 registered cases pass in
  142.50s (294 unit-labelled plus four corpus/manifest/negative checks).
- Tools-off x64: independent build succeeds after the documented link retry;
  all 292 registered cases pass in 134.36s (288 units plus four static checks).
- Tools-off x86: independent build succeeds; all 292 registered cases pass in
  123.89s (288 units plus four static checks).

Tools-on registers six optional Debug/xasm32 cases: debug_output, debug_linear,
xasm32, xasm32_contract, xasm32_bounds and debug_machine. Whole-name comparisons
confirm these are the only tools-off exclusions on either width.

## Manifest Identity

All six complete manifest verifiers pass against current files. Only the two
changed x86 corpora advance from provisional S93 to shared-m5-t540-s95.
Unchanged Lib/Common revisions are retained; no artificial six-way revision
equality is claimed. Hashes below identify the manifest files themselves.

| Corpus | Manifest SHA-256 |
| --- | --- |
| src/lib | 57349CF17B15EEE25C03D5A9A449E1E9754391BF408CCD29D0E3E1EF145A9135 |
| src/common | 34B0D65856DE6B5749F2EA887B8A8C016D01847CD38C50BC243BF590E5AC4DDF |
| src/x86 | 955B2CDB36C764327E31357217246D4F6B5E02D07304DC213FAD49166885EF5E |
| test/lib | 60806E4B2938FA8F2BAD0B821987254C93E81F24C612AB99A3399F858CE5227C |
| test/common | EF8ADB031AA0CC78598B7DCADE9B683DE0DB7AC399BBF2F356429AA3ACEF20ED |
| test/x86 | 7832C251CF3838DB1BD395BE1BD6D155E5BE19EF388B93049F7AA56226174E22 |

Root S94 production/test/build inputs are unchanged except manifest revision
comments. Retest all six registered manifest checks: x64 passes 6/6 in 0.99s.
S94 complete 492/492 units and specialized gates remain valid for their unchanged
execution inputs; no CPU runtime repetition is required by a comment-only
manifest identity update. x86 affected manifest checks pass 6/6 in 1.22s.

Tools-off x64 first build fails linking core-machine-timeline-s2-smoke.exe:
linker cannot open the output file. A concurrent read-only CTest registration
query was still active; no running test executable is found in that tree.
The precise lock owner is not proven. Once the x64 query finishes, retry the
remaining 20 link steps without source/assertion changes: all links succeed.
Its full registered runtime suite subsequently passes 292/292.
The slow read-only tools-off x86 enumeration is deliberately cancelled once
equivalent generated-registration comparison and actual command inspection are
complete. No executing test is interrupted, restarted or skipped. Avoid querying
unbuilt executables merely to compare registrations, and do not issue concurrent
registration queries against a suite whose executable/log files are in use.

No MyNES build, owner INI, external ROM/media or production ABI change occurs.

## Coordinator Result Review

Review all four completed commands and exact registered sets. Tools-on/off
each preserve their whole current suite, including source/test manifest,
corpus and verifier-negative execution; actual negative source rejects private
headers, reversed chip/Core/App dependencies, forbidden platform/CRT tokens
and composition selecting a Core variant. These are static boundary controls,
not hardware timing or product boot qualification.

Objdump of the entry-plan test in all four independent trees confirms x64
pei-x86-64 and x86 pei-i386. These Debug test binaries intentionally retain
debug information and are not deployed product artifacts. Six complete
manifest checks, actual command graph, registration exclusions, unchanged
root execution inputs, documentation governance and diff checks qualify
S95's verification result. No production source or test assertion changes.
The two x86 manifest revision comments change one line each; all file-hash
entries remain identical. No partial implementation commit is made.

Independent x64 tools-off and x86 tools-off retain full LastTest.log output;
x86 additionally retains s95-test.log. Earlier tools-on command outputs are
recorded by the executing tool transcript and above. A later CTest show-only
query rewrites its LastTest.log even without executing cases; do not represent
that rewritten file as runtime proof or rerun successful suites merely to
recreate a log. Current holds result acceptance; S96 owns eight product artifacts
and semantic boot checkpoints, and S97 owns complete implementation delivery.
