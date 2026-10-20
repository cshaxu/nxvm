# Third-Party Notices

## Release Status

This file records the current source-distribution notice set. It is not a
binary-release manifest and does not authorize a release. A release must also
produce the selected product's manifest and complete its product-local release
checklist.

## Project-Owned Material

Project-owned source and documentation are licensed under the root MIT License:

```text
Copyright (c) 2026 Neko
```

## NXVM-Derived Material

The NXVM copyright holder authorizes the NXVM-derived source imported into this
repository under the root MIT License. The initial foundation is NXVM commit
`6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`; its M1 import covered 75 `src/`
files. Later moves and adaptations remain traceable through the provenance
index and Git history. The historical source attribution below is retained as
an NXVM provenance record; it is not a claim that each current file has an
identical source header.

```text
Copyright (c) 2012-2014 Neko
Copyright (c) 2012-2026 Neko
```

See `docs/nxvm/etc/research/provenance/nxvm-imports.md` for import scope and traceability.

## MyNES Governance And Shared Material

MyNES product source is project-owned material under the root MIT License. Its
governance and shared-corpus provenance is retained in
`docs/mynes/etc/provenance/`; this repository contains no game ROM or other
protected MyNES input. The root MIT License is the sole license text for both
products.

## Shipped Third-Party Components

### Raiden II Sprite Transform Constants

MyRaidenII M1 T4 S47 retains the finite Raiden II sprite-transform constants
derived from two BSD-3-Clause MAME source units at revision
`ab2d1ca215c6e14dbe83fd2d0a70a4862fd16253`:

- `src/mame/seibu/r2crypt.cpp`, copyright Andreas Naive and Olivier Galibert;
- `src/mame/seibu/seibu_helper.cpp`, copyright Ville Linde and Nicola Salmoria.

The product implementation is independently written and does not link MAME.
The constants and transform are an explicitly recorded L2 compatibility model,
not a claim that the original hardware implementation is known.

The BSD 3-Clause License permits redistribution and use in source and binary
forms with retained copyright, conditions and disclaimer. The authoritative
license and full provenance are recorded in
`docs/myraidenii/etc/evidence/t4-s47-sprite-transform-l2.md`.

```text
Copyright (c) Andreas Naive, Olivier Galibert, Ville Linde and Nicola Salmoria.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES.
```

Other third-party projects, firmware, guest media, research tools, Microsoft
files, and toolchain runtime components are not covered by this notice set. A
future release may include an external component only after its license, notice
text, source provenance, redistributability, packaging role, and binary
dependency have been recorded in the release manifest.
