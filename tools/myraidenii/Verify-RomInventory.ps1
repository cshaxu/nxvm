param(
    [Parameter(Mandatory = $true)]
    [string]$Inventory,
    [Parameter(Mandatory = $true)]
    [string]$Archive
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$imageByteLimit = 2097152
$imageCountLimit = 15
$archiveByteLimit = 33554432

function Require([bool]$Condition, [string]$Message)
{
    if (-not $Condition) { throw $Message }
}

function Require-ImageRecord($Image)
{
    foreach ($property in @('name', 'bytes', 'sha1', 'sha256')) {
        Require ($null -ne $Image.PSObject.Properties[$property]) "Missing image field: $property"
    }
    Require ($Image.name -is [string] -and $Image.name -cmatch '^[A-Za-z0-9_.-]+$' -and
        $Image.name -notin @('.', '..')) 'Invalid image name'
    Require (($Image.bytes -is [int] -or $Image.bytes -is [long]) -and
        $Image.bytes -ge 0) "Invalid image length: $($Image.name)"
    Require ($Image.bytes -le $imageByteLimit) "Image exceeds supported ROM size: $($Image.name)"
    Require ($Image.sha1 -is [string] -and $Image.sha1 -cmatch '^[0-9a-f]{40}$') `
        "Invalid SHA1: $($Image.name)"
    Require ($Image.sha256 -is [string] -and $Image.sha256 -cmatch '^[0-9a-f]{64}$') `
        "Invalid SHA256: $($Image.name)"
}

function Verify-Image($Entry, $Image)
{
    Require ($Entry.Length -eq $Image.bytes) "Image length mismatch: $($Image.name)"
    $sha1 = [System.Security.Cryptography.SHA1]::Create()
    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    $stream = $null
    try {
        $stream = $Entry.Open()
        $buffer = New-Object byte[] 8192
        [long]$total = 0
        while (($count = $stream.Read($buffer, 0, $buffer.Length)) -ne 0) {
            $total += $count
            Require ($total -le $Image.bytes) "Image expands beyond declared length: $($Image.name)"
            [void]$sha1.TransformBlock($buffer, 0, $count, $buffer, 0)
            [void]$sha256.TransformBlock($buffer, 0, $count, $buffer, 0)
        }
        Require ($total -eq $Image.bytes) "Image stream length mismatch: $($Image.name)"
        [void]$sha1.TransformFinalBlock($buffer, 0, 0)
        [void]$sha256.TransformFinalBlock($buffer, 0, 0)
        $actualSha1 = [BitConverter]::ToString($sha1.Hash).Replace('-', '').ToLowerInvariant()
        $actualSha256 = [BitConverter]::ToString($sha256.Hash).Replace('-', '').ToLowerInvariant()
        Require ($actualSha1 -ceq $Image.sha1) "SHA1 mismatch: $($Image.name)"
        Require ($actualSha256 -ceq $Image.sha256) "SHA256 mismatch: $($Image.name)"
    }
    finally {
        if ($null -ne $stream) { $stream.Dispose() }
        $sha256.Dispose()
        $sha1.Dispose()
    }
}

$manifest = Get-Content -Raw -LiteralPath $Inventory | ConvertFrom-Json
Require ($null -ne $manifest) 'Empty inventory'
foreach ($property in @('schema', 'archive', 'runtime_images', 'extra_nonruntime_images')) {
    Require ($null -ne $manifest.PSObject.Properties[$property]) "Missing inventory field: $property"
}
Require ($manifest.schema -ceq 'myraidenii-rom-inventory-v1') 'Unsupported inventory schema'
Require ($null -ne $manifest.archive) 'Missing archive record'
foreach ($property in @('bytes', 'sha256', 'entry_count')) {
    Require ($null -ne $manifest.archive.PSObject.Properties[$property]) "Missing archive field: $property"
}
Require (($manifest.archive.bytes -is [int] -or $manifest.archive.bytes -is [long]) -and
    $manifest.archive.bytes -gt 0) 'Invalid archive length'
Require ($manifest.archive.bytes -le $archiveByteLimit) 'Archive exceeds supported size'
Require ($manifest.archive.sha256 -is [string] -and
    $manifest.archive.sha256 -cmatch '^[0-9a-f]{64}$') 'Invalid archive SHA256'
Require (($manifest.archive.entry_count -is [int] -or $manifest.archive.entry_count -is [long]) -and
    $manifest.archive.entry_count -gt 0) 'Invalid archive entry count'
Require ($manifest.archive.entry_count -le $imageCountLimit) 'Archive contains too many images'
Require ($manifest.runtime_images -is [array] -and $manifest.runtime_images.Count -gt 0) `
    'Missing runtime image array'
Require ($manifest.extra_nonruntime_images -is [array]) 'Invalid extra image array'
$images = @($manifest.runtime_images) + @($manifest.extra_nonruntime_images)
$imageNames = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
foreach ($image in $images) {
    Require ($null -ne $image) 'Null image record'
    Require-ImageRecord $image
    Require ($imageNames.Add($image.name)) "Duplicate inventory image: $($image.name)"
}
Require ($images.Count -eq $manifest.archive.entry_count) 'Inventory entry count mismatch'
$archiveStream = [System.IO.File]::Open([System.IO.Path]::GetFullPath($Archive),
    [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::Read)
$zip = $null
try {
    Require ($archiveStream.Length -eq $manifest.archive.bytes) 'Archive length mismatch'
    $archiveHasher = [System.Security.Cryptography.SHA256]::Create()
    try {
        $archiveHash = [BitConverter]::ToString($archiveHasher.ComputeHash($archiveStream)).Replace('-', '').ToLowerInvariant()
    }
    finally { $archiveHasher.Dispose() }
    Require ($archiveHash -ceq $manifest.archive.sha256) 'Archive SHA256 mismatch'
    $archiveStream.Position = 0
    $zip = New-Object System.IO.Compression.ZipArchive($archiveStream,
        [System.IO.Compression.ZipArchiveMode]::Read, $true)
    Require ($zip.Entries.Count -eq $manifest.archive.entry_count) 'Archive entry count mismatch'
    $entries = New-Object 'System.Collections.Generic.Dictionary[string,object]' ([StringComparer]::OrdinalIgnoreCase)
    foreach ($entry in $zip.Entries) {
        Require ($entry.FullName -cmatch '^[A-Za-z0-9_.-]+$' -and
            $entry.FullName -notin @('.', '..')) 'Invalid archive member name'
        Require (-not $entries.ContainsKey($entry.FullName)) "Duplicate archive member: $($entry.FullName)"
        $entries.Add($entry.FullName, $entry)
    }
    foreach ($image in $images) {
        Require ($entries.ContainsKey($image.name)) "Missing image: $($image.name)"
        Require ($entries[$image.name].FullName -ceq $image.name) "Image name case mismatch: $($image.name)"
        Verify-Image $entries[$image.name] $image
    }
}
finally {
    if ($null -ne $zip) { $zip.Dispose() }
    $archiveStream.Dispose()
}

[pscustomobject]@{
    marker = 'MYRAIDENII:ROM-INVENTORY:OK'
    archive_sha256 = $archiveHash
    runtime_images = $manifest.runtime_images.Count
    extra_nonruntime_images = $manifest.extra_nonruntime_images.Count
} | ConvertTo-Json -Compress
