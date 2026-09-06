# M5 T524 S12: Storage API Surface Cleanup

## Scope and ownership

The complete current call sweep identifies no production caller for
`file_write_exclusive`, `file_exists`, `file_replace`, `file_remove`,
`medium_mode_of` or `medium_flush`. The native exclusive-create, replace and
remove functions exist only to implement those public helpers, so they are the
same dead surface and are removed with it.

`medium_discard` is different: FDD and HDD eject both call it. It remains the
single operation that destroys a medium and nulls the owner lease.

## Persistence contract

Direct `medium_write_at` writes the requested range and calls `fflush` before
it returns. That is the one public persistence contract. A separate `flush`
operation would either repeat that work or require a second caller-selected
persistence policy, so it is removed. Readonly and discard-only overlay retain
their existing behavior.

## Test disposition

Storage fixtures create and remove their own temporary files with local C
runtime operations; they no longer preserve a shared public file API solely
for setup or cleanup. The Direct test proves persistence by closing and
reopening the file-backed medium.

## Verification and simplicity

- Focused storage/file/FDD/HDD/debug checks pass 6/6.
- A fresh standalone `src/lib` Windows CMake build and its public-header
  neutral-consumer CTest pass 2/2.
- The changed Linux storage implementation passes UCRT GCC C11
  `-Wall -Wextra -Wpedantic -Werror -fsyntax-only` checking.
- Complete repository unit passes 311/311 in 59.02 seconds; manifest and
  diff-hygiene checks pass.
- The tracked source/test change adds 21 and removes 120 lines, net minus 99.
  The retained paths are file reading/text recording, file-backed byte-medium
  I/O, immediate Direct persistence, replacement and lease discard only.

## P2 Linux gate repair

The existing Linux platform-contract gate still read the removed
`linuxcon.c` and asserted the pre-library pthread ownership model. It now
checks the real split: root CMake selects `linux.c`, library CMake owns
Curses/Threads, and `linux.c` owns its host-sync task, UX presenter, one
execution-stop request, join and finalize path. This is a gate-only correction:
no Linux runtime source or behavior changes.

## P3 line-reader removal

The complete production sweep finds no consumer of `reader_open`,
`reader_next` or `reader_close`; the sole test consumer exists only to retain
that API. P3 removes the reader type, declarations and implementation. The
file smoke keeps only live behavior: writer output is observed through the
production `read_owned` route, using newline-free text so it does not assert a
host-specific text line-ending conversion.
