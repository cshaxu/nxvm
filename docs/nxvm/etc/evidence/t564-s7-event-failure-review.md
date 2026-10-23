# T564 S7 — Event Failure Review

The Machine control paths for cold start, resume, stop, reset and debug cancel
now report or enter the existing synchronization fault path when their wake
event fails. The worker's running and paused transitions require their expected
previous state, so a concurrent fault is not overwritten by those transitions.
The existing independent task cancellation remains the only termination wake.
Synchronous completion uses the existing cancellation and join path; borrowed
state streams must remain live until worker exit.

Session's queue signals its availability while holding the queue lock before
publishing the copied event. A signal failure therefore rejects the event and
records the actual failure status instead of claiming success or calling it an
allocation failure. The pre-existing fault latch remains responsible for the
notification attempt; Machine cancellation is not a Session wake mechanism.

The owner expressly accepted that simultaneous failure of native waiting and
task cancellation is not solved by new recovery machinery. In this condition
the existing synchronous join can block; returning while borrowed data remains
in use would be unsafe. Similarly, if Session's fault notification event also
fails, an indefinitely waiting consumer is not guaranteed to wake. These are
limitations, not tested or claimed fixed here. PCM render-event reset remains
unchanged: its ignored result is a candidate, not a demonstrated normal-path audio
failure, and no combined platform-failure scenario was added to this task.

## Verification

- Existing persistent GCC/Ninja release trees: x64 and x86 builds passed.
- Full repository unit label after both source changes: 697/697 per width.
- Existing Machine and Session tests cover ordinary operation and individual
  event failure, request rejection, cancellation and non-repetition.
- Relevant MyNES lifecycle, snapshot and Debug plus DeskPro/Session integration
  selection: 9/9 per width; not a full integration-suite claim.
- The 12 receiver EXEs were rebuilt from the resulting shared implementation;
  PE widths were checked. Four PC profiles were linked sequentially using the
  same per-width build trees. MyRaidenII's two products were stripped from the
  per-width build outputs. No game, desktop or RDP manual acceptance is claimed.
- No Lib or x86 source was changed; no new API, thread, event or recovery path.
- NXVM documentation governance passed; `git diff --check` passed.

## Owner-requested complete qualification and closure

Both persistent GCC/Ninja Release trees used the repository ccache launcher
and built successfully. The full repository unit suite passed 697/697 for
each architecture. An initial attempt to run both suites concurrently at
eight jobs caused the x64 `lib.test-boundaries-selftest` to hit its own
30-second timeout; those incomplete concurrent runs were interrupted. Each
suite was then run alone at three jobs, and both completed successfully,
including that test. This is not evidence that the initial timeout did not
occur.

All registered integration cases were run for each width, sharing the
Lib/MyNES cases across the profile matrix: default PC 23/23, My5160 3/3,
My5170 5/5, DeskPro PC-specific cases 6/6, plus Lib 4/4 and MyNES 12/12.
The first x64 DeskPro batch was 21/22: the sandboxed native Window modal
test failed. Its exact test passed on a separate real Windows-session retry;
the x86 full DeskPro batch ran in that session and passed 22/22. Actual
desktop tests were exercised, but manual product, physical audio and RDP
acceptance are not claimed. No user configuration or media is intentionally
modified.

Profile switching rebuilt and deployed the default, My5160 and My5170 EXE
pairs; their build timestamp changes are included in the closing NXVM
delivery. The DeskPro, MyNES and MyRaidenII EXEs remain as delivered below.
The original delivery hashes are retained as historical evidence.

| Rebuilt product | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| My5160 | `BF5C2782E3D1054244093353A0001A70CF9D0376354B3E11BDB4ED2FD0680DCA` | `F22F9061070B74A5C8569F58D06D8948415F8FA0D498F90D784602F3615DDC35` |
| My5170 | `D5459A17554C27FFB930EC9EE31EB90B27F3B8632AF4D0161112897C601FB53B` | `D3D4EA737C947736843722BB1C1A42261EB51FF6C9AF1697C2E7D16D1A462154` |
| NXVM | `98B61677AA230CAE1185DF9BEC8A1D774B992C26695191E56CAD214D91970FD1` | `C007450508844B9E5BE9FC7A243E303ED8CB825C11E02DACEEBD6D6BF11F1CE2` |

## Delivered EXE SHA-256

| Product | x64 | x86 |
| --- | --- | --- |
| My5160 | `6FAD9E0FC4672967BA7EF74604B6B4F3878DAC95AFB91179B43585B83AED9447` | `D815561B85F8676C78D640D853E66D6FEFC5E5EC7842322D23E5914DE7FAA176` |
| My5170 | `F2EDBD3EE3BAA0C56F4A3751880B6858B2B621FE9CB785E055B7CE5583119A7C` | `1C22C9537945BA542DDFC967ACA4DA0C07BC3BAD7FE323F80A1C57308B28B876` |
| MyDeskPro386 | `3F5EBA2C309B0ECE563448B9F6155005FB2BA9D9962C83D3ADAF3CACB48C2029` | `42103241226E313F1667F8A39C501B949D0CC7819ADD7825FBBC07838EB290DD` |
| NXVM | `54F5DAC1B5BD47651485C9B9EE32C332DA34D54FC036A146E6013C0868E5F035` | `9B5D4743B03048BB13D3BE81AC689F581A4DAE9EE64D6F9405477BD228AE6D91` |
| MyNES | `86171F2A14CD698C922B60B7B5936CCF37AA9067C8710CA3BD547728C94E4A6C` | `84065D92E5E5180929EA133AD9C2B73E7FEE30D99643850E0B2D6F73D7047AB7` |
| MyRaidenII | `072AFEC02B84B22D6A70A7328FAAE152FF8231BF41A13B7AC0F19E609265B3EC` | `65A987114ECD389101025051E4262E7375983E7CB17E1E65D1EACC528FF45ACB` |
