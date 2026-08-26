# M5 T482 VM Session Profile YAML Cutover

T482 replaces the retained hard-coded profile-selection route with one
validated `nxvm-session` request path. YAML may select a built-in profile and
that profile's declared session choices; it may not author hardware.

| Subtask | Accepted result |
| --- | --- |
| S1 | Active: audit the current parser, console/profile selection and resolver inputs; freeze one root-only grammar and permitted-choice matrix. |
| S2 | Planned: implement and test one strict YAML request parser. |
| S3 | Planned: route the immutable request through the existing resolver and remove legacy parallel selection. |
| S4 | Planned: reconcile all current profiles, parser negatives and gates; publish closure. |
