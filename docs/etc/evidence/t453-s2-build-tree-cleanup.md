# T453 S2: Build-Tree Cleanup

S2 classified every immediate non-default directory under `build/`: four
`audit-current-head*` trees, forty `tNNN-*` task/experiment trees and the root
`Testing` residue. No current task referenced them and no CMake, Ninja, GCC or
CTest process remained active. Their measured total was 11,567,487,110 bytes.

Each resolved path was verified below `build/` before deletion. The cleanup
preserved `build/mingw-gcc-x64` as the fresh default tree and `build/output` as
the developer-artifact directory. Afterwards these are the only immediate
build directories; their combined remaining content is 796,740,533 bytes.

No source, asset, documentation evidence, user data or directory outside
`build/` was removed.
