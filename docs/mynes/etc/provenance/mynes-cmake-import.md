# MyNES CMake Import

The MyNES product traversal in `cmake/mynes/MyNesProduct.cmake` derives from
the owner-provided MyNES root CMake entry at sibling revision
`9ee5f902d82fda72059cb992dde00817cd1ff5af`.

The imported parts are the strict-warning function, MyNES App/Core and test
subdirectory traversal, the native-window fixture stack reservation, and the
MyNES x64/x86 preset inputs. The root `project(nxvm)` declaration and shared
Lib/Common traversal are intentionally not duplicated: the merged repository
owns them once at its root. This is a composition adaptation, not a product
source fork. The source is owner-authorized MIT material under the root
`LICENSE`; no ROM, media, firmware, or external binary was imported.
