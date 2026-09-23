# M5 Td S156: Output YAML Preservation

`M5:TD:S156:OUTPUT-YAML-PRESERVATION:OK`

Release artifact post-build handling previously copied repository session
templates into `build/output`, overwriting user-managed YAML. Commit `8b25ceb6`
removes every such copy operation; it continues to publish only the executable.
No YAML is restored, created, edited or otherwise managed by this Td.
