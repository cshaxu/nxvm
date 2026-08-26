# M5 T482 VM Session Profile YAML Cutover

T482 replaces the retained hard-coded profile-selection route with one
validated `nxvm-session` request path. YAML may select a built-in profile and
that profile's declared session choices; it may not author hardware.

| Subtask | Accepted result |
| --- | --- |
| S1 | Accepted: the v1 catalog plus `--profile` parallel selector is frozen for replacement by one root-only immutable request. |
| S2 | Accepted: one strict root-only YAML parser owns syntax, rejects duplicate/unknown forms, and returns a copied immutable request without profile policy. |
| S3 | Accepted: Console passes only the copied catalog request to the resolver; no production CLI re-encoding remains. |
| S4 | Accepted: the all-profile reconciliation retains one YAML request route and command authority; parser/request/Console regressions, the 296-test current gate and stripped Release 0480 pass. |

T482 is closed. The next receiver is the independent IBM 5160 capability
audit; it may consume the retained YAML route but cannot expand it.
