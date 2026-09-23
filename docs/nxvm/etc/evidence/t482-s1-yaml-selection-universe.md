# T482 S1 YAML Selection Universe

`M5:T482:S1:SELECTION-UNIVERSE:OK`

| Current producer/consumer | Observed role | S2/S3 disposition |
| --- | --- | --- |
| `vm_product_session_catalog_parse()` | Ad-hoc line parser accepts `schema: nxvm-session/v1` and nested `machine.profile`, plus CPU/FPU/memory/display/boot/media/firmware fields. | Replace; no v1 or nested-profile compatibility grammar remains. |
| Console catalog selection | Lists an accepted catalog entry and translates it to legacy `--profile` arguments. | Consume one immutable parsed request directly. |
| `session_factory.c` | Separately parses `--profile` and builds `vm_session_config`. | Delete the production selector/parser branch during cutover. |
| `vm_session_create()` | Has profile-kind branches but each resolves a frozen built-in declaration before Core construction. | Retain as the one resolved request consumer; S3 removes selector duplication, not profile-local construction. |
| `profile_resolver.c` and profile declarations | Validate/copy immutable Core inputs and declared requested options. | Remain the unique machine-authority boundary; YAML may not name a CPU, FPU, port, IRQ, DRQ, ROM or timing field. |

## Frozen Grammar And Choice Matrix

The sole new identity fields are root `schema: nxvm-session` and root
`profile: <built-in-name>`. Legacy `nxvm-session/v1`, `machine.profile`, and
all nested hardware fields are rejected. A profile may declare only its own
session choices: display policy, boot order, BYOB manifest and media bindings;
the parser validates syntax into one immutable request, then the selected
profile validates whether that choice is permitted.

Current catalog evidence also proves why this is a replacement rather than an
adapter: it currently embeds profile-specific CPU/FPU/memory/firmware rules
beside parsing and then re-enters `--profile`. S2 must move only common
request syntax into one parser and leave profile policy in the existing
resolver/profile owners.

`M5:T482:S1:GRAMMAR:OK`

`M5:T482:S1:CHOICES:OK`
