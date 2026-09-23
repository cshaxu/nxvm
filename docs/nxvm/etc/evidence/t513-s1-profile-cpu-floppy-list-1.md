# T513 S1: Profile/CPU/Floppy Function List 1

## Coverage universe

This is the finite boot-matrix universe for T513.  A row means one frozen
session construction contract, one selected FDD geometry, and one
owner-provided DOS boot image of that same geometry.  The external image and
any firmware needed by a BYOB profile stay outside the repository.  A
successful row reaches a DOS date/input state, DOS prompt, or installer entry
screen; the latter is a successful terminal.

| Rows | Profile | CPU | FDD | Geometry (cylinders x heads x sectors x bytes) | Construction source |
| --- | --- | --- | --- | --- | --- |
| 1 | IBM 5160 Model 268 | 8088 | 360K | 40 x 2 x 9 x 512 | `vm_session_create_xt_byob`; `vm_profile_xt_5160_268_declaration_create` |
| 2 | IBM 5170 Model 339 | 80286 | 360K | 40 x 2 x 9 x 512 | `vm_session_ibm_5170_floppy_select`; `vm_profile_ibm_5170_root_declaration_create` |
| 3 | IBM 5170 Model 339 | 80286 | 1.2M | 80 x 2 x 15 x 512 | same selector; profile default |
| 4--7 | default-pc-at | 8086 | 360K, 720K, 1.2M, 1.44M | 40 x 2 x 9 x 512; 80 x 2 x 9 x 512; 80 x 2 x 15 x 512; 80 x 2 x 18 x 512 | `vm_session_default_at_floppy_select`; `vm_profile_default_at_child_resolve` |
| 8--11 | default-pc-at | 80186 | 360K, 720K, 1.2M, 1.44M | same four geometries | same selectors; default CPU contract table |
| 12--15 | default-pc-at | 80286 | 360K, 720K, 1.2M, 1.44M | same four geometries | same selectors; default CPU contract table |
| 16--19 | default-pc-at | 80386 | 360K, 720K, 1.2M, 1.44M | same four geometries | same selectors; default CPU contract table |
| 20 | Compaq DeskPro 386 Model 40 | 80386DX | 1.2M | 80 x 2 x 15 x 512 | `vm_session_create_model40_byob`; `vm_profile_model40_child_resolve` |

`src/vm/profile/device/floppy.c` is the sole FDD-kind-to-geometry and CMOS
mapping.  `src/vm/composition/session/machine_devices.c` passes that selected
geometry to the one VM FDD owner; `vm_machine_fdd_insert_for` therefore
rejects an image whose byte length does not match the selected geometry.  The
matrix has 20 boot rows, not a cartesian product invented from available
images.

## Explicit rejected construction requests

| Profile | Rejected request | Current source disposition | T513 requirement |
| --- | --- | --- | --- |
| IBM 5160 Model 268 | Any CPU/FPU/session-format override, including 720K, 1.2M or 1.44M | `vm_session_create_xt_byob` accepts no session options and fixes 8088/360K | One negative construction test per rejected class |
| IBM 5170 Model 339 | 720K or 1.44M | `vm_session_ibm_5170_floppy_select` returns invalid argument | Retain and register negative tests |
| default-pc-at | 8088 or an invalid FDD enumeration | CPU contract selection or format selector rejects it | Register representative negative tests without adding a second selector |
| Model 40 | Any requested FDD format other than its frozen 1.2M path | **Gap:** `vm_session_create_model40_byob` currently ignores `floppy_format` | S4 must reject the request before construction; no silent substitution |

The Model-40 gap is a product-contract defect, not an extra supported row.
The repair receiver remains its sole session construction owner; the runner
must test the public construction result rather than inspect private state.

## External-input boundary

The machine can only execute a row when a compatible DOS image and any
required BYOB firmware have been supplied by the owner.  Availability is
recorded at execution time as available or unavailable by logical row ID and
geometry.  This document intentionally contains no file names, paths, bytes,
hashes, or vendor firmware/media catalogue.
