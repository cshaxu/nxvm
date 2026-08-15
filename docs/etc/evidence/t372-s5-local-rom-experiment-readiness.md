# T372 S5: Local-ROM Experiment Readiness

## Decision

The owner-authorized local research corpus is ready outside the repository.
It contains a verified two-part IBM 5170 Rev.3 ROM pair for the selected
8-MHz Type-3 baseline and a separate Phoenix 80286 clone pair. The IBM pair is
the only candidate firmware for a Model-339 experiment; the clone is retained
only for later compatibility contrast and cannot supply a Model-339 value.

No ROM, guest media, checksum, local pathname, filename, byte content, or
asset catalogue is present in Git. The worktree sweep confirms that the corpus
has not entered NXVM. This is a local research readiness result, not a product
firmware admission and not a 5170 L3 result.

## Experiment configuration gate

| Requirement | Result | Disposition |
| --- | --- | --- |
| Firmware provenance | Owner-authorized public IBM Rev.3 pair acquired as a matched two-chip set; a clone pair remains segregated. | IBM pair is eligible for isolated experiment use only. |
| Baseline match | Rev.3 firmware corresponds to Models 319/339 and adds 101-key and 1.44-MB support. | Satisfies firmware revision only; a reference machine must still prove Type-3, 8 MHz, 512 KB, CGA, no fixed disk and field-upgrade configuration. |
| Guest stimulus | Owner states that local DOS and 1.44-MB boot media are available. | May be used only outside Git in a later experiment S with a bounded public/program-authorship record. |
| Reference runtime | No approved local 86Box, MAME or PCjs execution harness is presently configured. | No observation or trace is collected in S5. A later S must install/configure a selected reference outside the repository and pin its revision. |
| Primary applicability | S4 confirms no direct Model-339 project-domain scalar in the manuals. | Any reference result remains a secondary corroboration for a named primary-open boundary, never hardware authority. |
| Leakage prevention | ROM/media remain external; repository scan has no ROM/media addition, absolute asset path, checksum or asset filename. | Passed. |

## Next observation plan

The next T372 continuation must create a local, revision-pinned reference
machine with the IBM pair and exactly the selected configuration. It must use
a minimal owner-authorized stimulus that exposes one named primary-open
boundary, retain only neutral aggregate observations in Git, and discard raw
traces from the worktree. It must compare the same stimulus with the clone
only as an incompatibility control, not as an IBM timing source.

## Similar-issue sweep

S5 swept ROM/media leakage, the complete Model-339 configuration tuple, IBM
versus clone provenance class, the S2/S3 reference restrictions, and every S1
physical residual. No external asset, local path, hash, default dependency,
runtime route, ABI, test, timing scalar or L3 claim changed.
