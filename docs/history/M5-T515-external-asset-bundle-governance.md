# M5 T515: External Asset-Bundle Governance

T515 closes the owner-governed external asset consumption boundary. Its
retained [proposal](M5-T515-external-asset-bundle-governance-proposal.md)
records the admitted scope.

## Closure

| Subtask | Result | Implementation P |
| --- | --- | --- |
| S1 | Replaced profile-specific and singular session grammar with YAML-relative `firmware` and ordered `media` request fields; retained VM composition and Core controller owners. | Pending S2 closure |
| S2 | Repaired every complete-unit baseline failure at its owner or corrected stale fixture contract; seven copied-template Release opens, full unit and integration gates pass. | This closure P |
| Corrective S4 | Restored the complete external YAML integration surface. Production sessions consume only their declared external ROM/CMOS assets; unit fixtures moved under `test/.../rom/`; the Model-40 static CMOS fallback is removed. | This closure P |

## Corrective S4 closure audit

The owner redefined T515's completion boundary to external-asset governance:
all integration session construction is YAML-driven; declared external asset
paths resolve; product code has no embedded firmware/CMOS fallback or parallel
asset-consumption path; unit fixtures remain repository-only. The static
boundary verifier passes and the complete unit suite is **302/302**.

The complete external integration suite was still run rather than reduced. It
is **41/44**: `compaq-deskpro-386-model-40.yaml`,
`ibm-5170-model-339-360k.yaml`, and `ibm-5170-model-339.yaml` reach bounded
real-ROM timeout diagnostics. The assets did load (Model 40 reports its Compaq
EGA signature `55AA`; 5170 reports its declared BIOS and CMOS state); these
are emulator boot-chain defects, not missing assets or fallback firmware. They
do not become pass evidence and transfer to the debt ledger for a source-led
ROM/board boot repair.

No protected asset bytes, `build/output` YAML mutation, asset discovery,
guest-drive semantics, or host-path dependency was added.
