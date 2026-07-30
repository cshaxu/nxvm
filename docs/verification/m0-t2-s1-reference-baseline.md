# M0 T2 S1 Reference Baseline Verification

## Commands

```powershell
git -C ../nxvm rev-parse HEAD
git -C ../nxvm remote -v
Get-FileHash ../nxvm/LICENSE -Algorithm SHA256
```

## Result

The observed commit was `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`; the
remote was `https://github.com/cshaxu/nxvm.git`; and the LGPL license hash
matched the baseline record. No NXVM file was changed and no NXVM source entered
ntvdm64. This is an upstream-checkout observation at capture time; M0 T8 records
the NXVM copyright holder's authorization for MIT imports into ntvdm64.
