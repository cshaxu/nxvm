param(
    [Parameter(Mandatory = $true)][string]$Verifier,
    [Parameter(Mandatory = $true)][string]$TemporaryRoot
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$root = [System.IO.Path]::GetFullPath($TemporaryRoot)
if (Test-Path -LiteralPath $root) { throw 'Temporary directory already exists' }
New-Item -ItemType Directory -Path $root | Out-Null
$caseCount = 0

function Require([bool]$Condition, [string]$Message)
{
    if (-not $Condition) { throw $Message }
}

function Hash-Bytes([byte[]]$Bytes, [string]$Algorithm)
{
    $hasher = [System.Security.Cryptography.HashAlgorithm]::Create($Algorithm)
    try { return [BitConverter]::ToString($hasher.ComputeHash($Bytes)).Replace('-', '').ToLowerInvariant() }
    finally { $hasher.Dispose() }
}

function New-Case([string]$Name, [array]$Members)
{
    $directory = New-Item -ItemType Directory -Path (Join-Path $root $Name)
    $archive = Join-Path $directory.FullName 'input.zip'
    $zip = [System.IO.Compression.ZipFile]::Open($archive, [System.IO.Compression.ZipArchiveMode]::Create)
    try {
        foreach ($member in $Members) {
            $entry = $zip.CreateEntry($member.name)
            $stream = $entry.Open()
            try { $stream.Write($member.bytes, 0, $member.bytes.Length) }
            finally { $stream.Dispose() }
        }
    }
    finally { $zip.Dispose() }
    $images = @()
    foreach ($member in $baseMembers) {
        $images += [pscustomobject]@{
            name = $member.name; bytes = $member.bytes.Length
            sha1 = Hash-Bytes $member.bytes 'SHA1'
            sha256 = Hash-Bytes $member.bytes 'SHA256'
        }
    }
    return [pscustomobject]@{
        archive = $archive
        inventory = Join-Path $directory.FullName 'inventory.json'
        manifest = [pscustomobject]@{
            schema = 'myraidenii-rom-inventory-v1'
            archive = [pscustomobject]@{
                bytes = (Get-Item -LiteralPath $archive).Length
                sha256 = Hash-Bytes ([System.IO.File]::ReadAllBytes($archive)) 'SHA256'
                entry_count = $baseMembers.Count
            }
            runtime_images = @($images[0], $images[1])
            extra_nonruntime_images = @($images[2])
        }
    }
}

function Run-Case($Case, [string]$ExpectedError = '')
{
    $Case.manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $Case.inventory -Encoding UTF8
    $failure = ''
    $result = $null
    try { $result = & $Verifier -Inventory $Case.inventory -Archive $Case.archive | ConvertFrom-Json }
    catch { $failure = $_.Exception.Message }
    if ($ExpectedError) {
        Require ($failure.Contains($ExpectedError)) "Wrong failure for $($Case.archive): $failure"
    }
    else {
        Require (-not $failure) "Unexpected verifier failure: $failure"
        Require ($result.marker -ceq 'MYRAIDENII:ROM-INVENTORY:OK' -and
            $result.runtime_images -eq 2 -and $result.extra_nonruntime_images -eq 1) 'Wrong successful result'
    }
    $script:caseCount++
}

$baseMembers = @(
    [pscustomobject]@{ name = 'even.bin'; bytes = [byte[]](1, 2, 3, 4) },
    [pscustomobject]@{ name = 'odd.bin'; bytes = [byte[]](5, 6, 7, 8) },
    [pscustomobject]@{ name = 'logic.jed'; bytes = [byte[]](9, 10) }
)

try {
    Run-Case (New-Case 'valid' $baseMembers)
    $case = New-Case 'schema' $baseMembers
    $case.manifest.schema = 'old-format'
    Run-Case $case 'Unsupported inventory schema'
    $case = New-Case 'missing' @($baseMembers[0], $baseMembers[2],
        [pscustomobject]@{ name = 'other.bin'; bytes = [byte[]](5, 6, 7, 8) })
    Run-Case $case 'Missing image: odd.bin'
    $case = New-Case 'length' @($baseMembers[0],
        [pscustomobject]@{ name = 'odd.bin'; bytes = [byte[]](5, 6, 7) }, $baseMembers[2])
    Run-Case $case 'Image length mismatch: odd.bin'
    $case = New-Case 'content' @($baseMembers[0],
        [pscustomobject]@{ name = 'odd.bin'; bytes = [byte[]](5, 6, 7, 0) }, $baseMembers[2])
    Run-Case $case 'SHA1 mismatch: odd.bin'
    $case = New-Case 'sha256' $baseMembers
    $case.manifest.runtime_images[0].sha256 = '0' * 64
    Run-Case $case 'SHA256 mismatch: even.bin'
    $case = New-Case 'duplicate' @($baseMembers[0], $baseMembers[0], $baseMembers[2])
    Run-Case $case 'Duplicate archive member: even.bin'
    $case = New-Case 'case-collision' @($baseMembers[0],
        [pscustomobject]@{ name = 'EVEN.BIN'; bytes = [byte[]](5, 6, 7, 8) }, $baseMembers[2])
    Run-Case $case 'Duplicate archive member: EVEN.BIN'
    $case = New-Case 'path' @($baseMembers[0],
        [pscustomobject]@{ name = '../odd.bin'; bytes = [byte[]](5, 6, 7, 8) }, $baseMembers[2])
    Run-Case $case 'Invalid archive member name'
    $case = New-Case 'name-case' @($baseMembers[0],
        [pscustomobject]@{ name = 'ODD.BIN'; bytes = [byte[]](5, 6, 7, 8) }, $baseMembers[2])
    Run-Case $case 'Image name case mismatch: odd.bin'
    $case = New-Case 'hash-format' $baseMembers
    $case.manifest.runtime_images[0].sha1 = 'not-a-hash'
    Run-Case $case 'Invalid SHA1: even.bin'
    $case = New-Case 'size-bound' $baseMembers
    $case.manifest.runtime_images[0].bytes = 2097153
    Run-Case $case 'Image exceeds supported ROM size: even.bin'
    $case = New-Case 'inventory-duplicate' $baseMembers
    $case.manifest.runtime_images[1].name = 'even.bin'
    Run-Case $case 'Duplicate inventory image: even.bin'
    $case = New-Case 'container' $baseMembers
    $case.manifest.archive.sha256 = '0' * 64
    Run-Case $case 'Archive SHA256 mismatch'
    $case = New-Case 'entry-count' $baseMembers
    $case.manifest.archive.entry_count = 4
    Run-Case $case 'Inventory entry count mismatch'
    $case = New-Case 'member-bound' $baseMembers
    $case.manifest.archive.entry_count = 16
    Run-Case $case 'Archive contains too many images'
    $case = New-Case 'container-bound' $baseMembers
    $case.manifest.archive.bytes = 33554433
    Run-Case $case 'Archive exceeds supported size'
    $expanded = New-Object byte[] 20000
    ([byte[]](5, 6, 7, 8)).CopyTo($expanded, 0)
    $case = New-Case 'expansion-bound' @($baseMembers[0],
        [pscustomobject]@{ name = 'odd.bin'; bytes = $expanded }, $baseMembers[2])
    $container = [System.IO.File]::ReadAllBytes($case.archive)
    $patched = $false
    for ($index = 0; $index -le $container.Length - 46; ++$index) {
        if ([BitConverter]::ToUInt32($container, $index) -ne 0x02014b50) { continue }
        $nameLength = [BitConverter]::ToUInt16($container, $index + 28)
        if ($index + 46 + $nameLength -gt $container.Length) { continue }
        $name = [System.Text.Encoding]::ASCII.GetString($container, $index + 46, $nameLength)
        if ($name -cne 'odd.bin') { continue }
        # Lie only in the synthetic central directory; decompressed data still exceeds four bytes.
        [BitConverter]::GetBytes([uint32]4).CopyTo($container, $index + 24)
        $patched = $true
        break
    }
    Require $patched 'Expansion fixture was not patched'
    [System.IO.File]::WriteAllBytes($case.archive, $container)
    $case.manifest.archive.sha256 = Hash-Bytes $container 'SHA256'
    Run-Case $case 'Image expands beyond declared length: odd.bin'
    "MYRAIDENII:ROM-INVENTORY-UNIT:OK ($caseCount cases)"
}
finally {
    # All generated files belong to this fresh test root; never delete a caller's existing directory.
    $resolved = (Resolve-Path -LiteralPath $root).Path
    Require ([System.IO.Path]::GetFullPath($resolved).TrimEnd('\', '/') -ceq $root.TrimEnd('\', '/')) `
        'Temporary cleanup target changed'
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
