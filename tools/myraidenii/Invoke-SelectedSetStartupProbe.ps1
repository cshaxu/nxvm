param(
    [Parameter(Mandatory = $true)]
    [string]$Inventory,
    [Parameter(Mandatory = $true)]
    [string]$Archive,
    [Parameter(Mandatory = $true, ParameterSetName = 'Probe')]
    [string]$Probe,
    [Parameter(Mandatory = $true, ParameterSetName = 'ProductLaunch')]
    [Parameter(Mandatory = $true, ParameterSetName = 'ProductSmoke')]
    [string]$Product,
    [Parameter(Mandatory = $true, ParameterSetName = 'ProductSmoke')]
    [string]$ProductSmoke,
    [Parameter(Mandatory = $true)]
    [string]$TemporaryRoot,
    [Parameter(ParameterSetName = 'Probe')]
    [switch]$CoinStartHeld,
    [Parameter(ParameterSetName = 'Probe')]
    [switch]$CoinStartPulse,
    [Parameter(ParameterSetName = 'Probe')]
    [switch]$CoinStartSequence,
    [Parameter(ParameterSetName = 'Probe')]
    [switch]$CoinStartFrameSequence,
    [Parameter(ParameterSetName = 'Probe')]
    [ValidateRange(2000000, 100000000)]
    [UInt64]$InstructionBudget = 2000000,
    [Parameter(ParameterSetName = 'ProductLaunch')]
    [ValidateRange(1, 60)]
    [int]$ObserveSeconds = 10
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

function Require([bool]$Condition, [string]$Message)
{
    if (-not $Condition) { throw $Message }
}

function Invoke-StagedChild([string]$FilePath, [string[]]$ArgumentList,
    [string]$WorkingDirectory, [string]$Name)
{
    $stdoutPath = Join-Path $WorkingDirectory "$Name.stdout"
    $stderrPath = Join-Path $WorkingDirectory "$Name.stderr"
    $process = Start-Process -FilePath $FilePath -ArgumentList $ArgumentList -WindowStyle Hidden `
        -WorkingDirectory $WorkingDirectory -PassThru -Wait `
        -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath

    if (Test-Path -LiteralPath $stdoutPath -PathType Leaf) {
        Get-Content -LiteralPath $stdoutPath
    }
    if (Test-Path -LiteralPath $stderrPath -PathType Leaf) {
        Get-Content -LiteralPath $stderrPath
    }
    if ($process.ExitCode -ne 0) {
        throw "$Name failed: $($process.ExitCode)"
    }
}

$verifier = Join-Path $PSScriptRoot 'Verify-RomInventory.ps1'
$inventoryPath = [System.IO.Path]::GetFullPath($Inventory)
$archivePath = [System.IO.Path]::GetFullPath($Archive)
$rootPath = [System.IO.Path]::GetFullPath($TemporaryRoot)
Require (Test-Path -LiteralPath $verifier -PathType Leaf) 'ROM inventory verifier is missing'
if ($PSCmdlet.ParameterSetName -ceq 'Probe') {
    $probePath = [System.IO.Path]::GetFullPath($Probe)
    Require (Test-Path -LiteralPath $probePath -PathType Leaf) 'Startup probe executable is missing'
}
else {
    $productPath = [System.IO.Path]::GetFullPath($Product)
    Require (Test-Path -LiteralPath $productPath -PathType Leaf) 'Product executable is missing'
    Require ([System.IO.Path]::GetExtension($productPath) -ceq '.exe') `
        'Product must be a Windows executable'
    if ($PSCmdlet.ParameterSetName -ceq 'ProductSmoke') {
        $productSmokePath = [System.IO.Path]::GetFullPath($ProductSmoke)
        Require (Test-Path -LiteralPath $productSmokePath -PathType Leaf) `
            'Product Console smoke executable is missing'
        Require ([System.IO.Path]::GetExtension($productSmokePath) -ceq '.exe') `
            'Product Console smoke must be a Windows executable'
    }
}
$controlModes = [int][bool]$CoinStartHeld + [int][bool]$CoinStartPulse +
    [int][bool]$CoinStartSequence + [int][bool]$CoinStartFrameSequence
Require ($controlModes -le 1) 'CoinStartHeld, CoinStartPulse, CoinStartSequence and CoinStartFrameSequence are exclusive'

# The verifier owns identity validation; its structured success record has no ROM payload.
& $verifier -Inventory $inventoryPath -Archive $archivePath | Out-Null

$manifest = Get-Content -Raw -LiteralPath $inventoryPath | ConvertFrom-Json
$requiredRoles = @(
    'program-even', 'program-odd', 'characters', 'background-1', 'background-2',
    'sprites-1', 'sprites-2', 'sprites-3', 'sprites-4', 'sound-program',
    'samples-1', 'samples-2'
)
$required = @()
foreach ($role in $requiredRoles) {
    $matches = @($manifest.runtime_images | Where-Object { $_.role -ceq $role })
    Require ($matches.Count -eq 1) "Inventory does not define exactly one $role image"
    $required += $matches[0]
}
[System.IO.Directory]::CreateDirectory($rootPath) | Out-Null
$stageName = 'selected-set-startup-' + [Guid]::NewGuid().ToString('N')
$stagePath = [System.IO.Path]::Combine($rootPath, $stageName)
Require ($stagePath.StartsWith($rootPath + [System.IO.Path]::DirectorySeparatorChar,
    [System.StringComparison]::OrdinalIgnoreCase)) 'Unsafe temporary path'
[System.IO.Directory]::CreateDirectory($stagePath) | Out-Null

try {
    $archiveStream = [System.IO.File]::Open($archivePath, [System.IO.FileMode]::Open,
        [System.IO.FileAccess]::Read, [System.IO.FileShare]::Read)
    try {
        $zip = [System.IO.Compression.ZipArchive]::new($archiveStream,
            [System.IO.Compression.ZipArchiveMode]::Read, $true)
        try {
            $images = @{}
            foreach ($image in $required) {
                $entry = $zip.GetEntry([string]$image.name)
                Require ($null -ne $entry -and $entry.FullName -ceq $image.name) `
                    "Missing verified image: $($image.name)"
                $destination = [System.IO.Path]::Combine($stagePath, [string]$image.name)
                $input = $entry.Open()
                try {
                    $output = [System.IO.File]::Open($destination, [System.IO.FileMode]::CreateNew,
                        [System.IO.FileAccess]::Write, [System.IO.FileShare]::None)
                    try { $input.CopyTo($output) } finally { $output.Dispose() }
                }
                finally { $input.Dispose() }
                Require ((Get-Item -LiteralPath $destination).Length -eq $image.bytes) `
                    "Staged image length mismatch: $($image.name)"
                $images[$image.role] = $destination
            }
        }
        finally { $zip.Dispose() }
    }
    finally { $archiveStream.Dispose() }
    if ($PSCmdlet.ParameterSetName -ceq 'Probe') {
        $probeArguments = @($images['program-even'], $images['program-odd'], `
            $images['sound-program'], $images['characters'], $images['background-1'], `
            $images['background-2'], $images['sprites-1'], $images['sprites-2'], `
            $images['sprites-3'], $images['sprites-4'], $images['samples-1'], `
            $images['samples-2'])
        if ($CoinStartHeld) { $probeArguments += '--coin-start-held' }
        if ($CoinStartPulse) { $probeArguments += '--coin-start-pulse' }
        if ($CoinStartSequence) { $probeArguments += '--coin-start-sequence' }
        if ($CoinStartFrameSequence) { $probeArguments += '--coin-start-frame-sequence' }
        $probeArguments += "--instruction-budget=$InstructionBudget"
        Invoke-StagedChild $probePath $probeArguments $stagePath 'startup-probe'
    }
    else {
        $iniNames = [ordered]@{
            'program-even' = 'program_even'
            'program-odd' = 'program_odd'
            'sound-program' = 'sound_program'
            'characters' = 'characters'
            'background-1' = 'background_1'
            'background-2' = 'background_2'
            'sprites-1' = 'sprites_1'
            'sprites-2' = 'sprites_2'
            'sprites-3' = 'sprites_3'
            'sprites-4' = 'sprites_4'
            'samples-1' = 'samples_1'
            'samples-2' = 'samples_2'
        }
        $iniLines = @('display=window', 'console_control=0')
        foreach ($role in $iniNames.Keys) {
            $iniLines += "$($iniNames[$role])=$([System.IO.Path]::GetFileName($images[$role]))"
        }
        [System.IO.File]::WriteAllLines((Join-Path $stagePath 'myraidenii.ini'), $iniLines,
            [System.Text.UTF8Encoding]::new($false))
        $stagedProduct = Join-Path $stagePath ([System.IO.Path]::GetFileName($productPath))
        [System.IO.File]::Copy($productPath, $stagedProduct, $false)
        if ($PSCmdlet.ParameterSetName -ceq 'ProductSmoke') {
            Invoke-StagedChild $productSmokePath @($stagedProduct, $stagePath) $stagePath `
                'product-console-smoke'
        }
        else {
            $process = Start-Process -FilePath $stagedProduct -WorkingDirectory $stagePath -PassThru
            try {
                if ($process.WaitForExit($ObserveSeconds * 1000)) {
                    throw "Product exited before observation: $($process.ExitCode)"
                }
                "MYRAIDENII:PRODUCT-LAUNCH:OK pid=$($process.Id) seconds=$ObserveSeconds"
            }
            finally {
                if (-not $process.HasExited) {
                    Stop-Process -Id $process.Id -Force
                    $process.WaitForExit()
                }
            }
        }
    }
}
finally {
    if (Test-Path -LiteralPath $stagePath) {
        $item = Get-Item -LiteralPath $stagePath -Force
        Require ($null -eq $item.LinkType) 'Refusing linked temporary path'
        Remove-Item -LiteralPath $stagePath -Recurse -Force
    }
}
