# NXVM Governance Provenance

Source: NXVM, commit `e5e32089bf7607c4ed5039ff43250c1f16d0232c`, inspected from
a clean local checkout on 2026-09-19. License: MIT, Copyright (c) 2026 Neko;
the full source LICENSE is preserved as the MyNes root LICENSE. Owner explicitly
requested this migration and MIT. Source checkout is read-only.

## Imported Units

Each source path maps to the same destination path. Digests identify the exact
source bytes before LF normalization and MyNes adaptation.

| Source and destination | Source SHA-256 |
| --- | --- |
| `AGENTS.md` | `0d480f98f1871dc42009be75b164f41ce322ea462cc866bd74f1f93529132f9a` |
| `CONTRIBUTING.md` | `e649db58fae148e5a71b91a52064ef98d0a86c682150873ab372ab1719ffd85d` |
| `LICENSE` | `bf37952d329b48cee37b42d90b96d30dc049940902e42d69338a19cf72003543` |
| `.editorconfig` | `9131bc05008b520f85dec6f3bba4cedb8fbd20740b024f24a0460ddab820ca05` |
| `docs/README.md` | `914459da63944aa65da3f47861654380af8a934d099a632ef3a383e66a703a20` |
| `docs/rules/ARCHITECTURE.md` | `367e43cd64cae814d9fbcf941443fe92fd53cace149f24c273aeb26ccce4a6be` |
| `docs/rules/CODING.md` | `c131232083940c8198289b4dd3c70e9c15a9ab6083d5712162c2a55ca703b58f` |
| `docs/rules/DOCUMENT.md` | `cc2ead8b3be2e2c42322f5a7cbae99e5acfda3b734f431d3055d236c8e51d17c` |
| `docs/rules/EXECUTION.md` | `013d89e9ada8110a0699406cc28d82bc46d642d16853915c6d9e25df5c9224d9` |
| `tools/Verify-DocumentationGovernance.ps1` | `149c76cf15186dda89a8838ba893fe2e5da6fb0bef8c0b683b6bb789289dc1ab` |

## Adaptations And Exclusions

Identity becomes MyNes / Neko's Nes Emulator. The owner approved local-only P delivery when no remote exists and immediate push when configured. MTSP, single active packet,
authority topology, review/closure, source and coding constraints are retained.
PC/DOS artifacts become prospective mynes 0.1 artifacts; no artifact is created.
The checker gains explicit documentation-only/unborn-Git handling, empty queue
support, tracked empty history topology, updated MyNes schemas and safe fixture
cleanup. Its inherited fixture needs both architecture presets and correct
artifact names; its packetless rejection fixture must remove all retained
progress/closure rows. These are tool adaptations, not emulator implementation.

The five project design documents, status/queue/debt, next proposal and source
policy are MyNes-specific adaptations of the source governance roles. Historical
PC design, closed tasks, evidence, machine-local paths, source corpora, binaries,
ROMs and old artifact identities are not imported. Root .gitattributes is
reduced to portable text/binary handling without legacy source exceptions.
Public governance-skill links retain their source references; their linked
contents were not imported or asserted as separately read during this bootstrap.

Verification and delivery are recorded in
[bootstrap evidence](../evidence/governance-bootstrap.md).
