# T522 S9 Storage Closure Evidence

## Ownership

`lib/storage/image` is the sole owner of a mounted image's byte allocation.
It exposes exactly two modes: `direct-readonly`, which has no writable byte
view, and `overlay`, which owns one private writable copy.  FDD/HDD borrow the
image byte view and retain device geometry, address-mark and controller
semantics.  `lib/storage/commit` is the sole media persistence route; the
native replace/remove operations are implemented beneath `storage/win32` and
`storage/linux`.

## Verification

- `unit.storage-image-smoke` proves overlay isolation and the absence of a
  mutable direct-readonly view.
- `unit.vm-media-direct-readonly-smoke` proves FDD/HDD direct-readonly media
  reads correctly and rejects writes.
- `unit.vm-media-provider-smoke` proves atomic single/pair commit, collision
  handling, malformed-sidecar rejection and failure cleanup through the shared
  storage route.
- Complete unit gate: 307 passing unit rows.
- External YAML volatile-overlay rows: Model 40 reaches `installer-running`
  in 175.74 seconds; IBM 5170 reaches `installer-running` in 79.22 seconds;
  default PC/AT 80386 reaches `dos-prompt` in 3.98 seconds.

## Asset Immutability Audit

After the YAML rows, the external-master SHA-256 values match their media
manifest: `fdd_1200k_msdos_500a_01.img` is
`0f51d92b482253fc468a2b470ffab82db43898d1c8b44e504808b7a3ef3d4bde` and
`hdd_40m_deskpro_386_blank.img` is
`2bbc68e612a72290a5181e070494a7580cbece1f528f92f009a269dc05819e73`.
