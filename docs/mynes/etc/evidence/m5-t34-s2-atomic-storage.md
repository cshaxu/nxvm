# M5 T34 S2 - Atomic Storage Evidence

Lib Storage now provides `lib_storage_file_replace_atomic`: it writes a
same-directory `.new` sibling, flushes and closes it, then invokes the platform
replacement primitive. A write, flush or close failure returns before the
destination is touched. A replacement failure returns I/O error and retains
the old destination bytes.

Windows uses write-through replacement; Linux flushes the file descriptor then
renames. The public interface remains format-neutral and Storage depends only
on Lib Types.

The Storage writer smoke directly proves successful replacement and an injected
replace failure retaining the prior bytes. Both x64 and x86 runs passed.
