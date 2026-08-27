# T496 S5 XT Startup Owner Classification

`M5:T496:S5:XT-STARTUP-OWNER=INSUFFICIENT-OWNER-EVIDENCE`

The owner-authorized external IBM 5160 boot probe was rebuilt, run in Turbo
mode, and host-waited through its fixed 60-second diagnostic limit.  It did
not reach either admitted DOS semantic checkpoint.  The existing Core
observation reported a waiting state without a next deadline and classified
the POST result only as `other`.

That classification excludes the probe's separately named memory, keyboard,
and floppy categories, but it does not distinguish the remaining possible
Core/board owners.  The current public observation and trace contracts supply
no finite, owner-unique batch for that `other` category.  Therefore this S
does not select a controller repair, alter product code, or make a
compatibility claim.

The runtime-only firmware and media inputs, their locations, identifiers,
contents, and raw trace/output are deliberately not retained here.

