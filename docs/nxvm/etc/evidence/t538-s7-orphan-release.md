# T538 S7 Input Repair Investigation

## Admission And Scope

Baseline ea250fa56. On 2026-09-27 the owner reviewed and approved the exact
Shared orphan-release change before implementation. It consumes a key break
absent from the existing source-local held ledger without flushing pending
modifiers. No API, second ledger, BIOS condition or product-specific branch is
added. NXVM and MyNES are receiving products; siblings remain read-only.

The investigation below preserves interim observations. Final qualification and
owner dispositions are recorded afterward; Current owns active/accepted status.

Similar-issue review searched `held_count`/`kvm_hotkey_matcher_submit` in Lib,
the Console/Window input adapters, and Common's delivered-key/reset paths.
Both leaves already use this matcher; no duplicate guard was added to Common
or the XT/AT mapper. Common retains its distinct delivered-key ledger for reset
and retirement. A search for `active` in the NXVM product command owner found
the INFO report as the relevant misuse; the machine information field itself
keeps its worker-lifetime meaning. Pause producers (product hotkey, debugger,
Common Window-close and driver debug stop) were inspected without claiming a
cause for the unclassified default case.

## Repairs And Regression Ownership

- Lib kvm-base owns physical input lifetime. Its unmatched-release branch now
  returns success without delivery. Matched release, repeat, hotkey consumption
  and sink failure retain their existing paths. The contract distinguishes
  matched releases from orphan releases.
- The existing Lib keyboard lifetime test covers initial orphans, pending
  modifier preservation, ledger discard, and both actual native adapters before
  and after input reset, including scan-less and scan-bearing input. Existing
  repeat, chord permutation, physical identity and failure cases still pass.
- NXVM INFO incorrectly used executor `active` (worker lifetime) as Running.
  The command now uses the authoritative Common session state already supplied
  to its callback. Its repository-only test crosses six lifecycle states with
  both active values; no external ROM, INI or media is loaded.
- The deployed observer now requires the current monitor prompt and a fresh
  INFO response reporting Running: No before sending debugger commands. It
  records Window transitions and pause timeout contents. Observer exit alone
  still does not prove guest boot.

## Before And After Observations

Unmodified deployed XT x64 with immediate Return release reached Setup; with
500ms delayed release it reported `9C 301`. Old INFO still said Running while
paused, so the improved observer correctly withheld debugger commands.

After repair, XT x64 with delayed release reached the complete DOS 5 installer
and acknowledged pause. The first immediate-release run instead reported
`0E 301`, also acknowledging pause. The owner reports no interaction with NXVM.
This failure is retained and unexplained; it is not attributed to external
input or counted as a successful boot.

A diagnostic run attached GDB to the actual deployed EXE before START, while
the machine remained stopped. Breakpoints recorded only Lib matcher input and
NXVM guest input delivery, not instructions. Observation was bounded to 60s
plus 20s watchdog, with 128 matcher events maximum. The observer's temporary
three-second attachment wait is removed from source after the experiment.
Immediate release reached Setup without any guest-input breakpoint before the
final pause. Delayed release recorded one Enter break (scan 1Ch, neutral Enter
identity, pressed false) in the matcher and no corresponding guest delivery;
Setup was reached. This directly verifies the approved orphan-release boundary,
but does not explain the separate 0E failure.

Default x86 subsequently ran for 180s, displayed A:\> in the original-resolution
Window capture, and acknowledged the requested final pause. No earlier pause
was observed. The S6 early pause remains unclassified, not retroactively passed.

With the temporary attachment wait removed, XT x86 also reached the DOS 5
installer under the same 500ms Return-release delay and acknowledged pause.
All owned diagnostic and GDB processes exited. The S7 changes remain uncommitted
because its complete residual-batch exit is not yet satisfied; no partial P or
task closure is claimed.

Raw bounded logs/captures under ignored `build/t538-s7/` remain needed for this
active investigation. No protected media are copied into the repository.

## Verification So Far

- NXVM x64/x86: 354 non-desktop cases each (333 unit plus 21 static), followed
  by three serial desktop cases each; 336 complete unit cases per width.
- NXVM external integration: 20/20 each width.
- MyNES: 127 non-desktop plus five desktop cases each width, 132/132 total.
- Six manifests, both product documentation gates and diff/check pass.
- Eight 0538 NXVM and two current 0043 MyNES Release receivers rebuilt;
  all ten PE widths match and no compiler debug sections remain.
- All four owner NXVM.ini byte hashes match the S6 baseline. No MyNES source,
  INI, snapshot or product policy changes are made.

The final comment clarification in hotkey.c is non-executable; verification
above covers the same production behavior. No Shared source other than the
approved branch is changed. At that interim point, evidence did not satisfy
the original three-fresh-launch/eight-pair qualification.

## Owner-Revised Qualification

On 2026-09-27 the owner explicitly instructs one successful launch per EXE/INI
pair, not three, and approves submission/closure when all eight pass. The
running redundant default x86 second launch is cancelled and its owned process
tree stopped; it is neither a failed nor a passed qualification case. XT/default
first launches are reused, not repeated. Each remaining pair gets one bounded
180-second observation with 50ms start-Return release. Prior 0E 301/default
early-pause observations remain unexplained historical facts, not false passes
or proof of external interference. This is the owner's acceptance disposition,
not a claim to have proved their causes or indefinite reliability.

The full non-desktop suites were rerun unchanged: NXVM 354/354 and MyNES
127/127 per width. The already-passed serial desktop suites (3 NXVM, 5 MyNES)
and NXVM 20/20 integration per width cover the same executable source. No new
production edit occurred during this verification. All six manifests pass.

## Artifact And Source Identity

Final single-pass matrix (`final-<profile>-<width>-r1` logs under the retained
ignored S7 directory), all on the hashes below:

| Pair | Semantic terminal | Pause / Debug handoff |
| --- | --- | --- |
| XT x64 | Complete DOS 5 Setup, Console | Confirmed |
| XT x86 | Complete DOS 5 Setup, Console | Confirmed |
| AT x64 | Complete DOS 5 Setup, Console | Confirmed |
| AT x86 | Complete DOS 5 Setup, Console | Confirmed |
| Model40 x64 | Complete DOS 5 Setup, native Window | Confirmed |
| Model40 x86 | Complete DOS 5 Setup, native Window | Confirmed |
| Default x64 | DOS A:\> prompt, native Window | Confirmed |
| Default x86 | DOS A:\> prompt, native Window | Confirmed |

No current qualification row reports 301/303, early pause, exit or hang. Each
Window remains Running through observation; requested final pause yields a
fresh INFO Running: No and existing Debug register/memory access. Model40's
x86 preview appeared partial, but its bitmap and PrintWindow counterpart are
byte-identical to both x64 files, SHA-256
`7FBC8F8DBCD82B6416602987107EAAEFD8D76AF7F7CBBFE24043B614EC29E838`.
The x64 original-resolution image contains the entire installer body and key
legend; paused guest memory also contains the full DOS title and CPU F000:DF4D
is the keyboard wait. The preview discrepancy is not a product failure. No
extra boot was run to replace it. Completed pre-revision repeats (XT six total,
default x64 three) also passed, but do not create a new repetition requirement.

The owner-revised T boot predicate is satisfied 8/8. Prior S2 reset/resume and
stop/start proof and the unchanged full lifecycle regressions remain valid.
The next P deliveries contain the complete repaired batch and its receiver
artifacts; coordinator review still precedes task closure.

All ten current optimized stripped Release receivers contain the S7 source
changes reviewed here. This document's NXVM implementation P and the linked
Shared P identify those changes; MyNES has only receiving binaries/evidence.
Shared source commit is 064b9619b.
Owner subsequently requests a key-specific wording correction; 268464d49 changes
only that comment and its manifest. Receiver hashes remain valid without a
redundant rebuild; source executable behavior is unchanged.
The four adjacent INIs retain the frozen hashes in the convergence ledger.

| Artifact | SHA-256 |
| --- | --- |
| nxvm_model40_0_5_0538_x64.exe | 328CE03EDC2066A13B5DF6C9C985E43C08DA5FE278B6E433252D6E53AE60AABE |
| nxvm_model40_0_5_0538_x86.exe | C45393C3B144EDD3C44D5235CA7283977CB2974B7BF833F3FC056B321927C2E0 |
| nxvm_default_0_5_0538_x64.exe | 67D1882E318617C038D21D3D95C652A39BD1F6C76955239D3C77DAF90429EC65 |
| nxvm_default_0_5_0538_x86.exe | F5533C0811BE065F9A13957F09E238BF7C7BF05EA580B0BE73FFDD796C853193 |
| nxvm_xt_0_5_0538_x64.exe | B7A17667BF44D7AC95B283C0099CDDFA8DC7C842CC5A37ECA3D98549814BA1CA |
| nxvm_xt_0_5_0538_x86.exe | CDF29C5AE29021C94CA43CF2029F9E04EAC6DA3BD285C3C12C864255D9B8062F |
| nxvm_at_0_5_0538_x64.exe | 3A2970D5A97170E19B2C52735BD808ED0B37F91383E86A0A572EECA20967499A |
| nxvm_at_0_5_0538_x86.exe | 7B64FC4A33876069DD4DBBB386B9713B7E208B0337C096F13F501DD3A70EF5E2 |
| mynes_0_0_0043_x64.exe | C9DB0320F3FA10167D1E250BCFADFED97559FD6556AC8296DB9B906AA729DA65 |
| mynes_0_0_0043_x86.exe | D355FCF73ABC57D51942E6DAF29FD6EEE69B6657BE50FA572FD795595810ACD1 |

Source/test/build delta uses git diff --numstat plus the new 52-line INFO
fixture: six paths, +184/-15, net +169. Production alone is +4/-4; the increase
is regression coverage and semantic pause acknowledgement in the existing
observer. No new production input path, state mirror, API or platform wrapper
is introduced. The retained Common delivered-key ledger has a different owner
and lifetime from Lib's accepted-source ledger. No executable timing grade is
changed. The separate cooked-history rollback debt remains in TODO.
