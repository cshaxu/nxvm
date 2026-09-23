# T484 S16 Xebec Type-2 Selection

The owner selected IBM Xebec drive Type 2 on 2026-08-26.  The copied
`ibm-5160-model-268` topology now carries the sole Xebec HDC configuration:
`320h`--`323h`, DMA3, IRQ5, one primary media identifier and Type 2's
`615 x 4 x 17 x 512` geometry (41,820 sectors).  Core accepts that Xebec
configuration only when this complete immutable descriptor is present.

IBM 6139790 supplies Type 2's 615 cylinders and four heads.  The accepted
S12 source contract records 17 sectors per track as corroborated Other-L3
86Box behavior; the sector count is the exact product of those five values.
No third-party code, firmware, media, or local path is introduced.

The profile supplies a construction-time descriptor only.  The Core media
provider remains the single owner of inserted bytes and current media state;
there is no runtime drive-type command, ATA alias, duplicate CHS cache, or
new generic XT layer.

Focused proof is `vm-xt-5160-268-profile-smoke` marker
`M5:T484:S16:XT-TYPE2:OK`, plus retained Xebec and HDC smoke coverage.
Logical-media command/DMA transfer, DREQ/IRQ mask behavior, hardware status,
ROM, and physical service time remain later S receivers on this one path.
