# M1 T7 S63 — COPX-D2 Contract Research Gate

## Scope and source grade

This is a source-policy-limited L2 research review.  It does not attach the
external image, add a configuration role, infer a Board lookup, retain image
data, or claim selected-set qualification.

The reviewed behavioral reference is the locally pinned
`Arcade-Raiden2_MiSTer-l2` checkout at revision
`27021027c5c3db226c5e77a27c5390022b8aa1f2`, licensed GPL-3.0.  The review
uses it only to determine whether it exposes an independently implementable
contract; no source, table, literal implementation or media data is copied.

## Contract review

The reference declares the COPX image as a separately loaded region.  Its
available public integration surface does not establish any of the facts a
local Board receiver needs:

- a request register or trigger;
- an index or address calculation;
- result width, ordering or publication point;
- reset/lifetime ownership; or
- an error or accepted-prefix contract.

The current local Product, Machine and Board likewise have no COPX consumer.
This is deliberate: attaching an external image without the five semantic
facts above would create an inert configuration role, while inventing a
lookup would be an unqualified L1 behavior.

## Independent L2 corroboration

The local MAME Raiden II reference separately declares the same external COPX
region while its available Board-facing map delegates ordinary COP work to the
existing COP device.  Static inspection exposes no independent COPX request,
index, result, reset or failure interface there either.  This corroborates the
MiSTer review; it does not upgrade either implementation to a hardware source
or license a copied lookup model.

## Disposition

No finite COPX-D2 receiver is admitted.  The existing high-priority TODO
remains correct and retains ownership in MyRaidenII Core COP/Board.  A future
receiver requires either directly applicable primary-source facts or a bounded
L2 behavior contract that supplies the complete request/index/result/reset/
failure model plus a synthetic non-ROM oracle.

## Verification

The review checked the local COP/Board ownership surface, the pinned external
revision and license, and the prior independent S27 boundary observation.  No
external image was opened or staged, no production/test/build/manifest/API or
asset changed, and no runtime claim is made.
