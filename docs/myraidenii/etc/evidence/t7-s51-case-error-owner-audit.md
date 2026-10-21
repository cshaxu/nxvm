# T7 S51 Case-Error Owner Audit

## Existing X86 Core Contract

`VCPUINS_EXCEPT_CE` is intentionally an internal invariant/transport stop, not
a guest architectural exception.  Its detail field is not a stable enum: CPU
paths use it for physical provider failures, port identifiers, decoded fields,
widths and timing/status invariants.  The finite number reported by S50 must
therefore be interpreted with the producer path, not as an opcode identity.

The current Core memory-read owner records the physical location when a device
provider rejects an access.  The S50 detail matches the local COP command
window's mapped start.  No new X86 Core observation, public interface or CPU
decoder path is necessary to establish this owner.

## App-Level Defect

The MyRaidenII COP query admits both read and write access for the command
window, but its read callback rejects every command-window read as invalid.
That provider rejection is converted by the unchanged Core memory owner into
the S50 internal case-error terminal.  The COP failure observation remains
clear because it only records failed writes/pending admission, not this read.

The external MAME Raiden II address-map reference is L2 evidence that the
same command window is declared write-only.  It does not establish a physical
open-bus value, timing or microcode claim.  It does establish that a CPU read
of that window must not be treated as an App-internal fatal invariant.

## Receiving Boundary

The next receiver belongs solely to `app-myraidenii/core/cop`: define one
explicit L2 read outcome for the write-only command window, keep writes and
pending admission unchanged, and test the actual Board/CPU provider path.
It must not alter X86 Core, add a second memory route, suppress CE globally, or
special-case a selected ROM location.  The output value must be documented as
an L2 open-bus model, not promoted to hardware-exact behavior without a
primary source.

## Verification

This was a source-only audit.  It inspected all current CE detail producer
classes, the copied diagnostic route, the MyRaidenII COP query/read path and
the independently maintained external L2 map.  No external asset was opened,
no dynamic trace was retained, and no source/test/build/manifest/API/artifact
file changed.
