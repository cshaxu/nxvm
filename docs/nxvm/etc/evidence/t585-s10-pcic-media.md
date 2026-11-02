# T585 S10 — PC110 memory-card media lifecycle

The MyPC110 monitor accepts stopped-state `pccard insert <a|b>
<readonly|overlay|direct> <common-image>|<attribute-image>` and `pccard eject
<a|b>`. The two explicit image paths may contain spaces. A change is staged
for the next cold start/reset; it is not persisted in configuration. Common
space follows the selected mode. Attribute space is read-only for readonly
cards and uses a temporary overlay for writable cards, so its source image is
never modified. Direct mode writes the common image only at explicit request.

Session serializes monitor and lifecycle calls; Machine changes its state to
STARTING before the worker handles reset. The product publishes pending leases
while stopped, and the existing owner worker transfers them to the PCIC before
reset. An active lease is detached only after worker shutdown during teardown.
Failed image opening does not replace the existing card or pending request.
This is memory-card backing, not a claim that an I/O card or physical card is
implemented or that a DOS driver recognizes a particular image.

The product-composition integration drives the real App extension, executor,
reset, debug bus and PCIC window on both host widths. It checks both sockets,
failed open, pending replacement and lease release, stopped-state admission,
ejection, and writes through an attribute overlay. It uses only disposable
test-owned images. Existing PCIC tests cover mapping, protection and reset.

Both cached Release builds completed and deployed the MyPC110 pair; their
SHA-256 values are x64 `233C7D91EC183D56691BF6B91AD1462E7FB815CA11D81E4189C3AF657093EC8D`
and x86 `9AEA60023FA3D202C461776EADD0BA1430B760FF48A76F5360666439B82B809D`.
The complete repository unit suites passed 712/712 on each architecture.
The isolated-desktop PC110 external-firmware integration suites passed 9/9
on x64 and 9/9 on x86. After adding the final lease-replacement and unchanged
attribute-file assertions, the product-composition integration was rebuilt
and passed again separately on each architecture.
An initial simultaneous pair of full runs timed out in the existing Emulator
negative verifier (711/712 each); independent reduced-concurrency full reruns
passed that verifier and all remaining cases without changing its budget.
No Lib or Emulator code was modified. Interactive RDP and physical media are
not asserted by this fixture.
