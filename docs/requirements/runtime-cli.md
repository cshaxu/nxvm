# Runtime CLI Requirements

This document defines the user-visible contract for the non-invasive
`ntvdm64 run` product path. It is a requirements record; it does not claim that
the options are implemented before their scheduled milestone.

## M6 Design Gate

Before M7 implementation, this document is completed with the program-path to
guest-drive mapping, exact guest/host/cancellation exit-status table, option
error table, handle-based filesystem-containment algorithm, DOS normalization,
dot/reparse/UNC/device/race rules, Windows 7 through Windows 11 matrix,
Console/window state machine, display-mode table, debugger grammar, input
routing, and restoration behavior. M1 through M5 use NXVM Console loading and
do not implement this product CLI. That pre-M7 workflow is distinct from the
product debugger, which has no runtime `load` command.

## Command Shape

```text
ntvdm64 run [--display=auto|console|window]
            [--debug]
            [--drive <letter>]... [--hide-drive <letter>]...
            [<program> [args...]]
```

`<letter>` is a case-insensitive host drive letter without a colon, for example
`C` or `d`. A selected host drive is exposed to the DOS guest at the same drive
letter. Later mount aliases may extend this grammar, but must not weaken the
access policy defined here.

Without `--debug`, omitting `<program>` is a usage error and returns before a
machine is created. `--debug` is the explicit entry-break request for a program:
with `<program>`, it stops before the first guest program instruction and makes
single-step and other debugger commands available.

## Display Policy

`--display` controls the DOS display surface. Its default is `auto`.

- `console`: always use the calling Win32 Console for the guest text display.
  This mode supports only the text modes declared supported by the runtime. A
  guest request for an unsupported graphics mode must fail clearly and return a
  defined nonzero result; it must not silently discard or corrupt graphics.
- `window`: always create a dedicated Win32 application window. It renders both
  guest text modes and every graphics mode implemented by the runtime.
- `auto`: begin with the Console text backend. When the guest selects an
  implemented graphics mode that the Console backend cannot display, create the
  dedicated Win32 application window and move the guest display there. Once
  moved, the display remains in that window until the program exits, including
  if the guest later returns to text mode.

The guest display, host `stdout`, host `stderr`, and diagnostic logging are
separate channels. Selecting `window` must not break normal command-line
redirection or pipe behavior for host output.

## Debug Product Experience

`--debug` enables an interactive CPU-debugger session. It is a developer mode;
it must not alter the default, non-debug program execution experience.

When `--debug` is enabled, display resolution is deterministic:

1. `--display=window` remains `window`.
2. An omitted `--display` or `--display=auto` resolves to `window`.
3. `--display=console` is invalid and must fail before guest execution. The
   guest display and debugger prompt must never compete for the same Console.

In a debug session, the dedicated Win32 application window owns only the DOS
guest display and guest keyboard/mouse input. The Console owns only the
interactive debugger. It presents register state, stop reasons, and a command
prompt supporting at least register inspection, disassembly, memory inspection,
execution breakpoints, single-step, continue, trace control, reset, and quit.
The debugger controls guest execution through a synchronized command boundary;
it must not concurrently mutate CPU state from the Console or window thread.

`ntvdm64 run --debug` without `<program>` is valid. It creates a reset machine
and stops at its initial execution point without loading a DOS program or an
implicit command interpreter. The debugger reports that no program is loaded.
There is no debugger `load` command: program selection is exclusively the
command-line `<program>` argument. In this no-program state, `continue` returns
to the host Console after normal debugger cleanup. A debug run with a program
stops before the guest's first program instruction.

If `--debug` is not enabled and `--display=window` is selected, the inherited
Console is a silent wait surface: it must not render guest display output,
status banners, or a debugger prompt. On an interactive Console, the runtime
must temporarily suppress echo and discard buffered input while the guest
window owns input, then restore the original Console mode and clear pending
input before returning to the shell. This prevents type-ahead entered during a
windowed run from becoming an unintended shell command.

A normal guest exit returns its defined guest exit status to the calling shell.
A request to close the guest window must take the controlled-cancellation path:
stop guest execution, close guest resources, restore host Console state, and
return a defined cancellation result. Normal operation remains silent; startup
failures and abnormal termination may write one clear diagnostic to `stderr`.

If no parent Console exists, `--debug` must create one for the interactive
debugger. If the selected standard handles cannot support interaction because
they are redirected, interactive `--debug` must fail clearly rather than read
or write through an unintended pipeline. `Ctrl+Break` requests a debugger
pause. The surface currently running the guest owns guest `Ctrl+C`: Console for
Console display, and the guest window for window display. During a windowed run,
the inherited Console remains the ntvdm64 control surface and receives `Ctrl+C`
only through ordinary Windows Console default handling; it does not interrupt or
manage the guest. M6 assigns the exact exit-status values and event sequences.

## Host Drive Visibility

The DOS guest has no host-drive access outside its resolved visible-drive set.
The runtime resolves that set exactly once before loading `<program>`:

1. If one or more `--drive <letter>` options are present, the visible set is
   exactly the selected drive letters. All other host drives are absent to the
   guest. `--hide-drive` options do not enlarge or override that selected set.
2. Otherwise, if one or more `--hide-drive <letter>` options are present, the
   visible set is all eligible host drives except the hidden letters.
3. Otherwise, all eligible host drives are visible.

Repeated options are allowed. Repeating a letter is idempotent. A drive that is
not available on the host, not eligible for the runtime, or not in the visible
set must appear nonexistent to DOS operations; it must not be substituted with
another host path.

For this contract, an eligible drive is an available, locally mounted,
drive-letter Windows volume that the caller can open through the platform
filesystem adapter. UNC-only paths, mapped network drives, and device
namespaces are never eligible drives.

The program file and every DOS file operation must be resolved inside an
exposed drive. If `<program>` is outside the visible set, loading fails before
guest execution with a clear nonzero result. The platform filesystem adapter
must enforce the set after canonicalization, including reparse-point traversal,
and must reject escape to a hidden drive, UNC path, device namespace, or another
host path not explicitly exposed.

Drive visibility governs guest filesystem access only. It does not grant
administrative access, alter the host system, or imply direct hardware-disk
access.

## Acceptance Cases

1. `ntvdm64 run --display=console text.com` uses the calling Console for a
   supported text-mode program.
2. `ntvdm64 run --display=window text.com` opens a dedicated application
   window for the same program.
3. `ntvdm64 run --display=auto graphics.com` starts in the Console only until
   the supported graphics-mode transition, then keeps rendering in one window.
4. `ntvdm64 run --drive D program.com` makes only `D:` available; `C:` and all
   other drives fail to open as nonexistent from the guest.
5. `ntvdm64 run --hide-drive C program.com` exposes eligible drives except
   `C:`.
6. `ntvdm64 run --drive D --hide-drive D program.com` exposes `D:` because the
   selection rule has precedence.
7. `ntvdm64 run --display=console graphics.com` fails clearly before graphics
   output is discarded when `graphics.com` requests an unsupported graphics
   mode.
8. `ntvdm64 run --debug` creates a reset, paused machine with no DOS program
   loaded; `ntvdm64 run --debug program.com` stops before its first guest
   instruction.
9. `ntvdm64 run --debug --display=console program.com` fails before guest
   execution; `ntvdm64 run --debug program.com` uses the dedicated guest window
   and an interactive debugger Console.
10. A path that reaches a hidden drive or a device/UNC namespace through a
    reparse point fails as nonexistent to the guest.
11. Closing a windowed guest returns the defined cancellation result only after
    guest resources and any inherited Console state are restored.
12. `ntvdm64 run` without a program and without `--debug` returns a usage error;
    `ntvdm64 run --debug` pauses with no program, and `continue` returns to the
    host Console without loading an image.
13. During a windowed guest run, `Ctrl+C` in the inherited Console follows normal
    Windows Console behavior and does not control the guest; guest `Ctrl+C` is
    owned by the guest window.
