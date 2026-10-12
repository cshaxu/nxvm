# T555 S6 Shared Constant Naming Audit

## Scope And Rule

Owner feedback on 2026-10-12 requires explicit DEFAULT names for fallback
values and direct names for actual fixed layout dimensions. This audit covers
`src/{lib,emulator,x86}` and `test/{lib,emulator,x86}` at the S6 worktree over
`468e191ff`. Core is a receiving adapter, not one of those six trees.
The scanned C/H inventory is 95 Lib, 21 Emulator, 19 x86 source files and
43 Lib, 18 Emulator, 9 x86 test files (205 files across the six trees).

The review searches macro definitions, enum/constant declarations, DEFAULT,
MAX/MIN, geometry/capacity names and fallback expressions, then follows the
relevant consumers. Header guards, function-like assembler macros and native
Lib Types aliases are not project default dimensions. This is a constant-role
audit, not a new all-code or dynamic qualification claim.

The normative rule is in `docs/rules/CODING.md`: DEFAULT is a default;
fixed dimensions/strides use their actual domain name; buffer capacity uses
CAPACITY; growing allocation starts use INITIAL. Numeric/protocol limits and
native vocabulary retain their actual meaning. No compatibility aliases are
needed for a source rename.

## Confirmed Findings And Approved Repairs

1. Lib Window glyph names were wrong. `KVM_WINDOW_FONT_HEIGHT=16` was used
   only for the default active height; `KVM_WINDOW_FONT_MAX_HEIGHT=32` was
   the actual slot stride. S6 replaces them with
   the Base-owned `KVM_TEXT_FONT_DEFAULT_HEIGHT=16` and
   `KVM_WINDOW_FONT_HEIGHT=32` after the owner approved the single default owner.
   Definition, both arrays, validator, renderer, geometry and direct/receiving
   tests are updated together. No value or behavior changes from the approved
   capacity implementation. Core's analogous copied-slot/default-input names
   and byte counts are updated at the receiving boundary.
2. `src/lib/console/console_interface.h`: `LIB_CONSOLE_LINE_MAX=1024`
   sizes stored buffers including NUL. The accepted payload is at most 1023,
   so the name confuses storage capacity with maximum content length.
   Implemented repair: `LIB_CONSOLE_LINE_CAPACITY`, with existing Console,
   broker and test references renamed; keep values and length checks unchanged.
   Current seven references are in the Console public header, Console owner,
   Win32 broker and direct Lib Console test. Expected source/test diff:
   +7/-7, plus manifests.
3. `src/x86/debug/command.c`: `DEBUG_MAXNARG=256` sizes the argument pointer
   array; it is not a default. Implemented repair: private
   `X86_DEBUG_ARGUMENT_CAPACITY`, updating the direct output test's two uses.
   `DEBUG_MAXNASMARG=4` has no caller in any of the six trees or Core/Apps;
   it is removed, rather than renamed. Initial expected source/test diff:
   +4/-5, plus manifests. Preserve the original command table/code style.
4. The same zero-height default is unnamed in
   `src/lib/kvm-console/console.c` (`: 16u`) and its retirement test (`: 16`).
   The KVM Base text contract already specifies this zero-height meaning.
   Implemented follow-up: make that existing common contract the single owner of
   one DEFAULT-named font-height constant, consumed by Window and Console;
   remove the Window-owned duplicate instead of adding an alias chain.
   The owner explicitly approved this boundary and the other two repairs on
   2026-10-12 before implementation. Affected files: KVM Base frame header, Window frame/geometry/
   render, Console conversion and their current default-height tests, plus the
   Core fallback consumer and manifests. Expected approximately +9/-8;
   no new function/type/state, and the default remains exactly 16.

Items 2--4 are now implemented under that approval. They are naming/ownership
issues, not evidence of a guest runtime failure. Existing tests independently
assert the unchanged 16/1024/256 values; component-owned compilation checks
reject reintroduced old names or compatibility aliases. No new test framework,
public function/type, helper, state, thread or object is added. The actual shared
rule is separately pushed as Td S178 `ccca1540a`, with restoration of the S6
packet in `7e6f781ce`, rather than folding a rule edit into an implementation P.

## Reviewed And Retained

- Lib `KVM_TEXT_COLUMNS/ROWS` and `LIB_CONSOLE_TEXT_COLUMNS/ROWS` describe
  actual 80-by-50 storage, not default visible 80-by-25. Retain their direct
  names. `WIN32_WINDOW_DEFAULT_WIDTH/HEIGHT` are genuine initial/fallback
  sizes; their DEFAULT names are correct.
- `KVM_WINDOW_GRAPHICS_MAX_WIDTH/HEIGHT` reject variable active dimensions;
  they are real accepted bounds, not default sizes. The derived
  `KVM_WINDOW_GRAPHICS_MAX_PIXELS` is also used for fixed backing capacity;
  a stricter capacity spelling could be `KVM_WINDOW_GRAPHICS_PIXEL_CAPACITY`
  (one definition, array and validator plus any callers), without changing
  the genuine active-width/height bounds. This is a low-priority naming
  refinement, not another default/slot pair defect.
- Lib numeric extrema, `LIB_WIN32_MAXIMUM_WAIT_OBJECTS`, SDK DEFAULT/MIN/MAX
  tokens and feature-test `_DEFAULT_SOURCE` preserve external meanings.
  Do not rename them as defaults or strip MAX mechanically.
- Emulator queue INITIAL_CAPACITY is a growing allocation's starting size;
  TEXT/PROMPT/PATH/REQUEST/RESPONSE/INPUT_QUEUE CAPACITY names are actual
  storage. The fixed monitor prompt is prescribed behavior, not a configurable
  default. No second default/dimension pair defect was confirmed there.
- x86 XASM32 MAX_CODE_BYTES is the actual 15-byte instruction limit;
  MAX_STATEMENT_BYTES is the accepted 255-byte content bound, with a separate
  NUL byte in storage. DEBUG LINE/PROMPT/ACCESS CAPACITY names describe storage.
  Opcode/register enum values are protocol identities, not default sizes.
- The three test trees use the production owner's dimension/default symbols
  for the affected glyph fixtures; literal case values such as 8/14/24/32 are
  deliberate input vectors, not parallel production default definitions.

## Verification Boundary

This audit does not add a regex gate that blindly bans MAX or DEFAULT tokens;
their meaning depends on the owner and callers. The S6 glyph repair plus the
approved naming/ownership batch is rebuilt and dynamically qualified under its
delivery evidence. All three retired repair names and the unused definition
are swept in all source/test trees; none remains in production. Explicit test
vectors and independent expected-height assertions keep literal 16 values.
MyNES's explicit active text-frame height is not a zero-height fallback or
another default definition; it remains unchanged. Numeric/platform limits and
the optional graphics-capacity spelling are deliberately retained as described
above. No sibling source, App semantics, INI, media or snapshot is changed.
