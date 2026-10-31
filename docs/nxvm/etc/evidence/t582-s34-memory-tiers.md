# T582 S34 — memory provider tier consolidation

The existing memory route collects already-matching providers and then repeats
the same query, success and terminal-error handling in three loops. A scratch
candidate uses one loop over replacement, ordinary and fallback tiers, retaining
registration order within each tier. Raw-decode and selected-alias routes,
matching, decoded addresses, provider callbacks, failure notifications and
RAM fallback remain at their existing positions. The source change is
21 additions and 54 deletions; there is no new public API, persistent state,
alternate executor or cached result.

An independent 20,736-case four-provider model compared tier selection,
query ordering and success/decline/terminal-error results. Separately, the
actual compiled original and candidate functions were linked under distinct
names and compared on 100,000 seeded eight-provider states on each width. The
direct comparison covered raw decode, CPU-only providers, overlapping
replacement/fallback flags, accepted, declined and failed queries, output
fields and failure callback order. These tests do not exhaust all addresses,
aliases or external providers, and the model is not the production code.

With private media and configured CMOS, fixed 10,000,000-instruction
baseline/candidate/candidate/baseline startup samples were
20947/18494/17783/20016 ms on x64 and 23883/24448/24089/22863 ms on x86;
all reached 79,964,294 guest ticks and PC `0003dbc5`. Both x64 pairs favored
the candidate; both x86 pairs favored the baseline by small amounts. These
sequential startup samples are subject to host drift and matching endpoints
do not prove complete-state equivalence. In a same-process direct-call
synthetic workload, 20 million routes per run measured 1083/1083/1172/1315
ms on x64 and 1053/1096/1070/1227 ms on x86; the input mix is not the real
guest. It did not establish a stable material regression or game speedup.
Fixed and direct log SHA-256 respectively:
`987008526EFDFAE7B20AF14C5EEF2C4E661BF23354439E52273DD0158F0459CB`,
`9168CB4A4DA4326E27F8A5B48D0C6341DCE6F3DB139428E4419DC2504084781D`.

This is selected for the approved reduction of repeated production logic;
it is not evidence that PC110 is now playable.

The actual Shared diff matches the 21 added / 54 removed source lines and
updates the source manifest. Existing memory, cache and reset-ROM alias
regressions passed 5/5 per width. The cached receiver passed 712/712 unit and
48/48 default integration tests per width, followed by 5160, 5170 and
DeskPro profile integrations at 3/3, 5/5 and 6/6 respectively per width. The
existing selected-set MyRaidenII startup and product Console checks also
passed per width; the default configuration was restored in both build trees.

All fourteen deployed EXEs passed PE-width verification and appear in the
[artifact inventory](t582-s34-memory-artifacts.json). NXVM, MyPC110, MyNES
and MyRaidenII deployment hashes match their current build outputs; MyNES was
byte-identical and MyRaidenII's two updated products were deployed. Other PC
profiles were checked by their product identity integrations. No native RDP
gameplay, physical-device session or complete interactive usability was
established by these tests.

SHA-256 (qualification log, external receiver log, artifact inventory,
direct comparison fixture source):
`6CBE18D91769B65D65D9B3E77E0FAB95F86D8EA47552BF0CDAEA59568F63784B`,
`D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`,
`11C8A35404E10F018F6B9DDF60E144D4CC522FB07D3686A382F894631AF6FB3B`,
`851DB1E5843519077A227EC158A3A20EE08B8FCA233C969ED4F0927FB051AC1D`.

## Personaware startup observation

After receiving the Shared change, a task-only x64 harness linked the prior
and current `memory.c` sequentially against the same current libraries. Each
executable and the same configured CMOS seed were placed in its private run
directory with private disk copies. The harness sent the same menu-selection
inputs through Machine and stopped on its existing game-board and score-area
pixel markers. The original reached that marker in 279.41 host seconds
(sample 274); the candidate in 275.14 seconds (sample 270). Interpolation
between recorded quanta gives approximately 167.28 versus 160.93 host seconds
for guest ticks 100 million through 600 million, with approximately
98,200,391 versus 98,207,705 retired instructions. Input and frame sampling
are asynchronous, quantum endpoints are not identical, these are sequential
single runs, and host scheduling may vary. Neither comparison proves a
repeatable speedup or identical complete guest state.

Both recorded images show the game board and score area; their top window areas
are at different partial redraw stages. This is a stronger visual checkpoint
than the original two-pixel marker, but is not proof of a fully rendered page,
input response, native Window/RDP operation, or playable speed. The roughly
four-and-a-half-minute arrival time confirms that the overall usability target
remains open. No owner media or production source was changed by this probe.
SHA-256 (summary, original log, candidate log, original PPM, candidate PPM):
`9EDD81889FF58FD80B6B4D00297046ED79807C98287F02C4B9DD64F489C77336`,
`07B439666787D43CF2435A4B0A95BAFF33317001CD8374671A5C109C00DD2422`,
`475176512DF368A8FB96555338D759548736C95A4BB30B2BF58D15E5CD8862CE`,
`2F2371A21DA9434993DE671F7E4866AA6E55169B83C79625B69D93A82EE4B33A`,
`74ED829C7538E240ABD709C2B95E4D755315D7CF9EC05635A4182BD02898FD63`.
