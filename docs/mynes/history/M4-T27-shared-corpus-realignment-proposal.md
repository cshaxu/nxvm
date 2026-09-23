# M4 Shared Corpus Realignment

## Outcome

Adopt SoftPC revision a2fbf5488cd0a768cbb5dc92d6e23706bf86b32e as the
byte-identical source of MyNes's four shared roots: src/lib, src/common,
test/lib and test/common. Move every compatibility adaptation into MyNes Core,
App or product-owned tests.

## Acceptance

All four MyNes roots are byte-identical to the named SoftPC revision. MyNes
Core converts its internal i32 APU mix to SoftPC Lib's i16 PCM frames and uses
its queue/query/clear contract. App starts cooked management only after Common
reports its actual stopped state. Both Windows architectures configure, build,
and pass the imported shared suites plus MyNes product regressions. SoftPC is
not modified and owner ROMs remain ignored.
