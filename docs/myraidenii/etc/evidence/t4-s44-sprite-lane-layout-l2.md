# T4 S44 Sprite Lane Layout (L2)

## Reference

The selected external Raiden II board implementation was inspected at revision
`ab2d1ca215c6e14dbe83fd2d0a70a4862fd16253`, path `raiden2.cpp`. It describes
four 2 MiB sprite inputs as two 32-bit little-endian word-lane regions: sources
one and two form the first 4 MiB region, and sources three and four form the
second.

## Adopted Boundary

Video is the sole graphics-copy owner. It retains each 16-bit source word's
byte order and writes one four-byte stream group as source-one word followed by
source-two word. The same rule applies to the upper region using sources three
and four. This fixes only copied ingress layout; it does not claim a sprite
decode, decryption, object protocol, priority rule, or playable-game result.

## Verification

The owning synthetic Video test marks two consecutive words in every source
and checks both regions' exact eight-byte stream order. The Board smoke retains
its public graphics bundle route and verifies copied input remains independent
of released source storage. No external ROM or reference payload is used.
