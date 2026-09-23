# T476 S3 IBM 5170 Root Cutover

`M5:T476:S3:IBM5170-ROOT-CUTOVER:OK`

## Retained Production Path

An IBM session now owns one `vm_profile_default_pc_at_resolved_root` for its
entire lifetime. Its `profile` points into that owned deep-copy snapshot, while
the Core configuration and controller timing rules are copied directly from
the same root's resolved values to the existing Core-plan input. The old
`vm_session_profile_select` Model-339 branch and its second materialization
step are deleted.

The static Model-339 descriptor remains only as the S2 root-declaration source
until a later subtask replaces that source representation; no session reaches
it. Default PC/AT remains the distinct direct profile route. No Core code sees
a profile name or root pointer.

## Parity And Sweep

The frozen S1 composition, clock, CGA and firmware/FDC smokes pass unchanged
apart from assertions that now require session identity `pc-at-5170`, reject a
static descriptor pointer, and prove the Core input equals the session-owned
resolved values. The former pointer assertions were legacy-route assertions,
not hardware behavior.

`rg` over VM session construction finds no
`vm_profile_ibm_5170_model_339_descriptor_get` call. Remaining descriptor
references are root-source and parity-oracle reads in the profile module/tests;
they do not construct a session. The session factory and public profile-kind
retain the legacy visible name intentionally: S3 has no CLI/YAML rename scope.

## Transfer

The next T476 work may replace the retained static root source with the
normalized root declaration after proving no profile identity or catalog
consumer needs the legacy descriptor name. DeskPro and default-at remain later,
ordered migrations.
