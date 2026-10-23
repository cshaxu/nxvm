# Atomic persistence writes

Candidate, not admitted. Battery `.sav` and snapshot `.mns` paths truncate the
destination before writing; failed write or close can lose the previous copy.
The existing medium replacement transfers a lease and is not file replacement.

First review and separately authorize a neutral Shared Storage replacement
capability and all affected consumers, then adopt it for both MyNES save paths.
Exit: write, close and replacement failures retain the old file, success replaces
it correctly, and both widths pass relevant tests. Do not bypass Storage with a
product-local native implementation or assume an unimplemented API exists.
