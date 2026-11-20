# MyOB300 external inputs

MyOB300 is being qualified for Release x64 and x86. Keep
`myob300.ini` beside the executable. The launcher resolves the INI,
the fixed System Card path and the INI media paths from that
directory, including when a shortcut has a different working
directory.

Place the raw common-memory image from your own OmniBook 300
System 1.0 ABA card at
`../../../nxvm-assets/hp-omnibook300/rom/system10-aba-common.bin`
relative to the executable. The file has 16,777,216 bytes and SHA-256
`b98001a30b79f126483f823fd4d7a5ce91a973a5ab8fb35e3d76043b74a8e099`.
Extract the raw entry from the archive before launching. MyOB300
checks the full digest of the loaded bytes before creating its worker.
Keep protected card bytes outside this Git repository.

The sample INI points to an owner-supplied C image. A ZIP containing
the owner's C files is not a sector image. Use a prepared image with an
active primary FAT16 partition and the original C files. Keep the master
read-only or work from a disposable copy. The default `overlay` mode keeps
writes off the input image. Use `direct` on a disposable image only when
you intend to persist guest writes.

The current model selects 2 MiB RAM. The original ROM DOS worker is
verified through the command prompt. Windows and the native Product
release remain separate qualification work.
