[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$OutputDirectory,

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z0-9][A-Za-z0-9._-]*$')]
    [string]$Name,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$Marker,

    [Parameter(Mandatory = $true)]
    [ValidateRange(0, 255)]
    [int]$ExitCode
)

$markerBytes = [System.Text.Encoding]::ASCII.GetBytes($Marker)
if ($Marker -cmatch '[^\x00-\x7F]') {
    throw 'Marker must contain only ASCII characters.'
}
if ($markerBytes -contains [byte][char]'$') {
    throw 'Marker must not contain $. INT 21h AH=09h uses it as a terminator.'
}

# COM programs begin at offset 0100h. The fixed instruction sequence is 12
# bytes: mov dx, marker; mov ah, 09h; int 21h; mov ax, 4cxxh; int 21h.
$markerOffset = 0x100 + 12
$program = [System.Collections.Generic.List[byte]]::new()
$program.AddRange([byte[]](0xBA, ($markerOffset -band 0xff), (($markerOffset -shr 8) -band 0xff)))
$program.AddRange([byte[]](0xB4, 0x09, 0xCD, 0x21))
$program.AddRange([byte[]](0xB8, $ExitCode, 0x4C, 0xCD, 0x21))
$program.AddRange($markerBytes)
$program.Add(0x24)

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$comPath = Join-Path $OutputDirectory "$Name.com"
$manifestPath = Join-Path $OutputDirectory "$Name.json"
[System.IO.File]::WriteAllBytes($comPath, $program.ToArray())

$manifest = [ordered]@{
    format = 'ntvdm64-dos-probe-v1'
    name = $Name
    kind = 'com'
    origin = '0x0100'
    interruptContract = @('INT 21h AH=09h', 'INT 21h AH=4Ch')
    marker = $Marker
    exitCode = $ExitCode
    sha256 = (Get-FileHash -Algorithm SHA256 $comPath).Hash.ToLowerInvariant()
}
$manifest | ConvertTo-Json | Set-Content -Encoding utf8 -NoNewline $manifestPath
