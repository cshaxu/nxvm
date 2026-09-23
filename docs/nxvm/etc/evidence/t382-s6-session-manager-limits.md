# T382 S6: Session-Manager Limits

`M5:T382:S6:SESSION-MANAGER-LIMITS:OK`

The session manager now has one pre-provider admission check for both maximum
session count and its representable final ID. Its generic limits constructor
is a product resource-policy interface, not a test-only control: normal
creation retains the largest representable defaults, while bounded hosts can
declare a smaller session/ID policy.

The core smoke proves capacity rejection and final-ID rejection before provider
open, with count and provider-open count unchanged. The VM session-manager and
multi-window smokes now explicitly open their initial session, preserving the
frozen zero-session manager contract. A single-worker current aggregate passes
all 250 current-gate tests; its log records both formerly failing targets as
passed.

| Artifact | SHA-256 |
| --- | --- |
| `build/output/nxvm_0_5_0382.exe` | `4869611AF7A9657CAC0FB74C45CBBA3632742F19C616EFC371BDDE79EC4C0471` |
