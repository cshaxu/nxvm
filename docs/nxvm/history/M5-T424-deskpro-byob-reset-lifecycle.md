# M5 T424: DeskPro BYOB Reset Lifecycle

T424 S1 proves the existing lawful Model-40 BYOB carrier reaches the reset
vector through its one VM-to-Core lifecycle. A project-owned synthetic ROM pair
passes the transient manifest validation, is copied into the VM-owned carrier,
executes one bounded reset instruction through the existing Core run owner, and
returns to the same copied reset-vector observation after cold reset.

The VM reset-vector API now declares its actual `type_status` return contract.
No Core firmware-provider ABI, ROM mapping owner, vendor firmware asset,
local-path retention, firmware catalogue, downloader or default dependency is
introduced. T424 uses the shared physical-cycle proposal's receiver 5 at
`original` lawful-interface tier only; it makes no claim about vendor POST,
service behavior, physical timing or Model-L3 readiness.

The remaining vendor-BYOB firmware corpus is retained in TODO under T390 S2
containment: it needs separately approved owner-supplied ROM/media outside the
repository and a bounded observation checkpoint.
