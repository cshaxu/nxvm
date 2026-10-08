# T546 S17 GDTR/IDTR Physical Table Route

## Authority and scope

The source authority is *Intel 80386DX Programmer's Reference Manual* (1990),
`9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1`, held
outside the repository at the owner-managed NXVM manual archive.  Pages 5-15
and 5-16 define the GDTR and IDTR table-base fields as physical addresses.
Section 5.3.4.3 defines publication of the paging directory/page-table
Accessed bits during a page walk.

This S corrects only that reference class.  LDT and TSS descriptor bases remain
logical references and retain their existing paging route.  No TLB, timing
model, board input, firmware workaround, public interface or persistent cache
is introduced.

## One route

`_ksa_read_idt`, `_ksa_read_gdt`, and `_ksa_write_gdt` now call the private
`_ksa_*_physical_table` helpers, which use the existing CPU physical-memory
route and current access provenance.  The helpers are not exported.  For the
pre-80286 fixed IVT path, the existing physical-zero interpretation remains
explicit; 8086/8088/80186 behavior is therefore unchanged.

The former logical GDT preflight in task-transition availability publication is
removed: a GDT busy-bit write is a physical table write, and the existing
descriptor bound check plus the sole physical write own that operation.

Before this correction, an incidental page walk through a descriptor/IDT
reference could publish the PDE Accessed bit.  The page-walk owner now publishes
that bit immediately after successfully reading a present PDE, before later PTE
protection rejection, as specified by 80386 section 5.3.4.3.  Preview mode
still publishes neither page-table state nor fault diagnostics.

## Similar-issue sweep

The S17 sweep searched descriptor, gate and task-table callers of the logical,
physical, preflight and commit helpers.  GDTR/IDTR are the physical-table
exceptions and were redirected.  LDT/TSS remain segment-based, so their logical
routes are retained.  No remaining GDTR/IDTR caller directly invokes the
logical access or test helpers.

The S16 task-paging fixture had temporarily mapped the target linear GDT page.
S17 removes that receiver accommodation: the new regression keeps the table
only at physical `0x8000`, maps only linear page zero, loads a descriptor, and
proves that the physical descriptor Accessed bit is published while the linear
PTE for the table page remains untouched.

## Verification

- Direct x86 descriptor physical-table, page-fault delivery, paging and IBM PC
  task-transition regressions pass on x64 and x86.
- Complete repository-only units pass 506/506 on x64.  The same complete x86
  set passes in non-overlapping CTest partitions; the one initial concurrent
  firmware-smoke timeout was rerun alone and passed in 16.66 seconds, proving
  host contention rather than a product failure.
- x86 and IBM PC manifests pass, as do the CPU authority negative gates on both
  widths.  The authority gate itself now searches every forbidden dependency
  token independently; the former alternation-like CMake expression could miss
  a real single-token violation.

The four receiving PC Apps were rebuilt as stripped 0546 binaries; PE-header
inspection confirms the named width and SHA-256 records the deployed bytes:

- My5160: x64 `8F2DA18151DDA1FE5E35FC43A5112A2659B49063F5DE5546DC2D5B2C4D154E5D`,
  x86 `7346D056553A29F8FE329ADE714C2F9D3A67AC3B413A6D74B9423DD8C82D5BD6`.
- My5170: x64 `10389EB24696024E5DC03142D6858A39840DFEA83082D55B2FD0017565BF6515`,
  x86 `BD4501B9C25AE098A54E6C70DF6F8A4AD5ED6F2B750E4CBDA88339F9C14A29EC`.
- MyDeskPro386: x64 `D8454E5DC098140B5EAFCF370A9BA2A11E6E22634178DB46E9600FA0B7B68634`,
  x86 `B687068AC08D1DBB36CCC395B7E3171220C9AED0D865BC2E713F1B372671F2A7`.
- NXVM: x64 `5B70233B6C72CA9C74EF379634DDC8DCBB2885F063C8663725951EDF989B5F1F`,
  x86 `70E52BCD6C7A5D8D4DE46B6FEF4BA35A607E3F11D34CB92A245D1F888A4BFD31`.

This record closes S17 only; T546 remains open for its later CPU receivers.
