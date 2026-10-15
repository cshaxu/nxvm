# T561 S1 SoftPC Ten-Tree Import

## Baseline And Authority

Owner admission includes Lib, Emulator, X86 Debug, X86 IBM PC and X86 NXVM,
each source and test package. NXVM starts at clean `453455e59`; T560's accepted
CPU stage `b906c7138` remains frozen. Source is project-owned MIT SoftPC commit
`3af1955932fcb9e61ceec3456493580e316a12c9`, accepted split `8046b085`.
No commercial firmware, disk, snapshot, independent third-party code or App
source enters the imported packages. Existing notices remain unchanged.

SoftPC subsequently began an uncommitted removal of X86 NXVM. This S uses a
Git archive of the pinned commit, not that moving worktree. No sibling file is
modified. The exact archive and qualification logs live under ignored build/;
the commit and canonical manifests reproduce the import independently.

## File Disposition And Semantic Review

| Canonical tree | Files | Disposition |
| --- | --- | --- |
| src/lib | 109 | Already identical; untouched. |
| test/lib | 49 | Already identical; untouched. |
| src/emulator | 30 | Already identical; untouched. |
| test/emulator | 22 | Already identical; untouched. |
| src/x86-debug | 15 | Old X86 Debug/assembler moved to split namespaces. |
| test/x86-debug | 9 | Matching six mechanism smokes plus independent gates. |
| src/x86-ibmpc | 14 | PC interaction/entry, without NXVM-specific capabilities. |
| test/x86-ibmpc | 6 | PC command, keyboard, entry and independent gates. |
| src/x86-nxvm | 6 | NXVM-only information/speed declarations. |
| test/x86-nxvm | 4 | Information/speed contract smoke and independent gates. |

All 264 files, including manifests, are required to match the pinned tree by
relative path and raw SHA-256. The old src/x86 and test/x86 are retired, with
no alias targets, alternate implementation or old-header forwarding.

Before implementation, compare old sources after only the declared include/
symbol namespace replacements: aasm32 (10,431 lines), dasm32 (6,822), Debug
command (3,296), PC command (187), keyboard (109), and entry (87) are identical.
Line counts include the final split element. No table-style rewrite or CPU
instruction/timing algorithm change is involved.

## Receiver And Lifetime

Core factory now produces the imported X86 NXVM machine descriptor, preserving
its original information/speed callbacks, binding context and actual driver.
Core startup keeps that descriptor on the stack for the complete synchronous
PC process body. The PC shell receives a neutral composition whose lifetime
callbacks unwrap the descriptor; the driver's own context remains the actual
Core machine. Extension configuration borrows the same live descriptor.

The three local callbacks perform necessary lifetime/context translation;
there is no added heap allocation, global, persistent flag, worker, public
compatibility interface or duplicated device state. Core's existing creation
and destroy functions remain the actual resource owners. Banner-capacity
failure now releases the successfully constructed candidate before returning.

The Core startup regression checks context preservation, extensions, normal
bind/unbind/destroy, failed bind without spurious unbind, configuration failure
without construction, and banner-capacity failure without leaking a candidate.
Factory assertions keep all information and speed capability checks. Existing
real INI/Console tests remain product receivers rather than shared fixtures.
The existing real-EXE INI identity test also sends INFO, SPEED, SPEED TURBO and
SPEED STANDARD before HELP/EXIT, proving these callbacks through the actual
PC shell and Core adapter rather than only the stubbed factory unit. Its
missing/wrong-name INI rejection and relative-media assertions are retained.

## Build And Boundary Sweep

Root registration and unit aggregates name each split package once.
Test registration normalizes hyphens in target-list variable names. The
generic test include gate uses explicit inward sets: X86 NXVM is parallel to,
not above, X86 Debug/IBM PC. Its negative probes verify that distinction.
Core dependency declaration, DAG audit and debugger path checks follow the new
headers without weakening the Core-to-App or lower-Core boundaries.

Current architecture/layout and shared test ownership maps follow the split.
The existing TODO for historical shared-rule vocabulary is updated; docs/rules
remains unchanged because implementation S tasks cannot rewrite that authority.
No CPU successor work, Core extraction or production 486 admission is claimed.

## Qualification Record

Initial x64 focused build and all 28 imported-package tests plus the existing
Core factory regression passed. An initial full-gate attempt correctly rejected
the incomplete 0561 artifact set before unit execution; it is not a passed
qualification. Execution order is corrected to build all eight replacements,
retire only superseded tracked 0560 files, then run complete units/gates and
the profile integration matrix. No assertion or timeout is relaxed.

Final qualification and artifacts are recorded below. Manual RDP, full Windows
GUI and PC110 validation are outside this import's proof.

The sandboxed whole-unit run reached 433/621 without an individual failure
before its unchanged 300-second aggregate deadline. The native rerun retained
the same graphs, `-j8` build, four CTest jobs and budgets: x64 621/621 in
76.24 seconds and x86 621/621 in 76.08 seconds, with all specialized gates.
Each width includes the 100 public-package units, Core and all Apps, including
the new startup regression. Three desktop-labelled unit cases per width pass
automatically. The aborted run is not counted as qualification and no OS,
timeout, assertion or public source workaround was introduced.

## Final Receiver And Artifact Qualification

| Qualification | x64 | x86 |
| --- | --- | --- |
| Complete repository units | 621/621 | 621/621 |
| Specialized gates | Pass | Pass |
| Default profile, including MyNES integration | 35/35 | 35/35 |
| My5160 integration | 3/3 | 3/3 |
| My5170 integration | 5/5 | 5/5 |
| DeskPro integration | 5/5, 128.34 seconds total | 5/5, 167.99 seconds total |
| Restored-default identity | 1/1 | 1/1 |
| Strengthened published INFO/SPEED/INI receiver | All four Assets EXEs pass | All four Assets EXEs pass |
| Final Core manifest/corpus/factory/startup checks | 5/5 | 5/5 |

This is 48 integration executions plus restored-default identity per width.
The final published-receiver run independently exercises the actual deployed
files after strengthening the existing integration test. Both original
Release Ninja/ccache graphs end restored to default. Units include three
automated desktop-labelled cases per width; default integration includes two
more. These are automated results, not manual desktop/RDP acceptance.

Logs use the task-owned ignored build/t561- prefix: width-specific
units-gates.log, profile integration.log, restored-identity.log,
final-core-checks.log, published-cli.log, ten-tree.log and final-artifacts.json.
The terminal orchestration and final-receiver processes both exit zero.
The final equality check still covers all 264 pinned files. No source under
Lib, Emulator, Core CPU or MyNES changed; owner INI files and MyNES 0044 EXEs
remain byte-identical. No source/asset workaround or stale-path alias remains.

Each Assets root retains only its 0561 pair. Existing deployment validates
native architecture, and final verification checks PE machine 0x8664/0x014C
and embedded version 0.5.0561. Superseded 0560 EXEs remain recoverable in Git.

| App | Width | SHA-256 |
| --- | --- | --- |
| NXVM | x64 | A01BCDCCCA2FB8E22BDA337BF56734BE8D585B5E92B889251E15D3D5F29F72CC |
| NXVM | x86 | 45B402E1DE27B0DD3CA56ED1B6E9DBF69DA8E5D3787DA9E204ADE7E30A506737 |
| My5160 | x64 | 3F88297F86A6F0048990F2057318EA46B699E9DF553FFF96565A8E2377BECA1A |
| My5160 | x86 | FCBAD7C872FEF3D777E8DF8397E162910FA1A9A7A73D302ED4D67FC596FD937B |
| My5170 | x64 | C9A7FCC3B769B9E067A5E35C4989D398F57633A823DE7523CF1F35D07D0D56E8 |
| My5170 | x86 | 10A86410B9E08C4AF2607571A37E4F61FAE4AFCA973E76242AEFA77A8F680881 |
| MyDeskPro386 | x64 | EBA3C62F8C1BFC84F3E87009DF12A10D313E2422C2DB474BC5C9E7ADF66412A0 |
| MyDeskPro386 | x86 | C0452FC035E07D041868A3278194C4F98AC058E40284F2AF2F6F1EBE33A4B305 |

## Actual-Change Review

Coordinator review rereads the actual import, receiver, tests, build/checker,
documentation and binary changes, not only this report. Canonical source is
the pinned upstream bytes; actual Core CPU/Lib/Emulator/MyNES diffs are empty.
NXVM source/test changes including their manifests are +235/-83, net +152;
the 112-line startup regression is the only newly authored source/test file.
The three production lifetime/configuration callbacks are necessary context
adaptation, with no new state/API/framework. The INFO/SPEED conversions retain
their old behavior. Shared changes are committed separately from NXVM
receivers/artifacts; the coordinated pair is the qualified repository cutover,
not a claim that an intermediate cross-target commit alone runs every App.

Retain the existing Debug format-warning exemption and historical CPU warning
exceptions honestly. Passing qualification does not mean exception-free
strict compilation or that all CPU qualification debt is complete.

Shared implementation P1 is `15d86fc8e`: 71 paths, +1,287/-683, net +604,
including canonical split verifiers/manifests and root test/build ownership.
NXVM P2 supplies the receiving Core changes, evidence and eight artifacts.
