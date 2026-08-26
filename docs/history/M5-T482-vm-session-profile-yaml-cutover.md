# M5 T482 VM Session Profile YAML Cutover

T482 replaces the retained hard-coded profile-selection route with one
validated `nxvm-session` request path. YAML may select a built-in profile and
that profile's declared session choices; it may not author hardware.

| Subtask | Accepted result |
| --- | --- |
| S1 | Accepted: the v1 catalog plus `--profile` parallel selector is frozen for replacement by one root-only immutable request. |
| S2 | Accepted: one strict root-only YAML parser owns syntax, rejects duplicate/unknown forms, and returns a copied immutable request without profile policy. |
| S3 | Active: prove the immutable request is the only Console-to-resolver selection route and remove any remaining CLI re-encoding. |
| S4 | Planned: reconcile all current profiles, parser negatives and gates; publish closure. |
