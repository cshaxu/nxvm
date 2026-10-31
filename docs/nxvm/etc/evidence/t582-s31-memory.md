# T582 S31 — memory routing

Shared commit `b2c17ba68` reuses the decoded physical address within one RAM
route resolution. Provider queries are observational; after an operational
provider callback declines an access, the RAM fallback still decodes again.
The plain-span cache check uses its input directly only under the existing
zero-address-mask and no-mapping conditions. Provider ordering, mappings,
side effects, timing and public interfaces are unchanged. Production diff:
8 added, 6 removed; the source manifest changes one hash.

Existing memory registration, address-line/port, inspection and CPU cache
tests passed on x64 and x86 (four tests each). The cached qualification ran
712 unit tests and 48 default integration tests per architecture, plus the
selected 5160/5170/DeskPro integration sets (3/5/6 per architecture); all
passed. The selected-set MyRaidenII startup and deployed Console probes also
passed on each width; these do not establish game playability. The fourteen
release files and PE widths are recorded in [the artifact inventory]
(t582-s31-memory-artifacts.json). MyNES binaries match the existing builds and
do not depend on this X86 Core change.

The fixed-work private-media probe executed ten million instructions with
79,822,202 guest ticks and PC `00039119`. Sequential baseline/candidate then
candidate/baseline wall times were 19048/18245 and 18763/18714 ms on x64;
23870/23363 and 23325/23104 ms on x86. These mixed samples do not establish
a stable speedup, complete machine-state equivalence or interactive PC110
usability. The change eliminates a repeated calculation; the broader
performance objective remains open. No interactive desktop/RDP acceptance was
performed in this S.

Task-owned log SHA-256: qualification
`07D79CF654B85E2664D47249D76E65F2D0BC170A128810EDF420A8F146063462`;
external receiver
`D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`;
forward measurement
`5943AE3AF225BAAAA9138C669689DDB620ACFCA91D6CCF4E11D8748BD54255EA`;
reverse measurement
`B7B08D12FE987FA8149DCD0FBC6A9E20F4734481AC27F1D0ED48F2EA595C5C5D`.
