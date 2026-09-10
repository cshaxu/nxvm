# M5 T525: Canonical SoftPC Library Adoption

## Closure

T525 closes the NXVM-side canonical SoftPC library adoption. The exact
S1-frozen `src/lib` corpus is retained without a local fork; the product now
uses one NXVM coordinator over its public Window, Console and Console-broker
contracts.

`d6f8578a` is the accepted implementation delivery. It removes the retired
unified-presenter route and binds product Console ownership through one logical
monitor/broker, while each Window stays session-local. Direct composition is
valid without a product Console binding and therefore runs headless rather
than fabricating a second Console path.

The closure evidence is [T525 S3 canonical product binding](../etc/evidence/t525-s3-canonical-product-binding.md).

## Acceptance

Coordinator actual-diff review confirms that the canonical corpus remains
unchanged; the retained production route has no `ux_presenter`, `ux_run`,
product-native SDK call or compatibility presenter. Console broker ownership,
Window ownership, source-retirement release and headless execution each have
one owner. No Core, profile, firmware, media or SoftPC application source was
added.

Verification: 306/306 repository-only unit, 44/44 external-asset integration,
canonical manifest, platform/session/build boundary gates, `git diff --check`,
and optimized stripped x64/x86 `0524` artifacts. The artifact hashes and full
commands are retained in the S3 evidence.

The retained proposal is [M5 T525 proposal](M5-T525-canonical-softpc-library-adoption-proposal.md).
