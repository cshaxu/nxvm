$ErrorActionPreference = 'Stop'
Import-Module (Join-Path $PSHOME 'Modules/Microsoft.PowerShell.Utility')
$repository = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$name = "cleanup-contract-$PID"
$fixture = Join-Path $repository "build/$name"
$tool = Join-Path $repository 'tools/shared/Clear-Build.ps1'
New-Item -ItemType Directory -Path $fixture | Out-Null

function Reject([scriptblock]$Action, [string]$Expected)
{
    $rejected = $false
    try { & $Action | Out-Null }
    catch { if ($_.Exception.Message -notlike "*$Expected*") { throw }; $rejected = $true }
    if (-not $rejected) { throw "Expected refusal: $Expected" }
}

try {
    $file = Join-Path $fixture 'ordinary.txt'
    [IO.File]::WriteAllText($file, 'owned disposable fixture')
    $preview = & $tool -Targets "$name/ordinary.txt"
    if ($preview.Deleted -or -not (Test-Path $file)) { throw 'Preview deleted data.' }
    Reject { & $tool -Targets "$name/ordinary.txt" -Apply } 'task exit'
    Reject { & $tool -Targets '../outside' } 'nonliteral'
    Reject { & $tool -Targets $repository } 'nonliteral'
    Reject { & $tool -Targets '.' } 'nonliteral'
    Reject { & $tool -Targets '.ccache.' } 'nonliteral'
    Reject { & $tool -Targets '.ninja ' } 'nonliteral'
    Reject { & $tool -Targets 'CCACHE~1' } 'nonliteral'
    Reject { & $tool -Targets '.ccache' } 'persistent cache'
    Reject { & $tool -Targets '.ninja/nested' } 'persistent cache'
    Reject { & $tool -Targets "$name/ordinary.txt" -RequireClean } 'do not cover'
    Reject { & $tool -Targets "$name/ordinary.txt", "$name" } 'Overlapping'
    $media = Join-Path $fixture 'fixture.img'
    [IO.File]::WriteAllText($media, 'synthetic, not real media')
    Reject { & $tool -Targets "$name/ordinary.txt", "$name/fixture.img" -Apply -TasksExited } 'Unreviewed'
    if (-not (Test-Path $file)) { throw 'Partial deletion before preflight completed.' }
    $reviewed = @{ "$name/fixture.img" = (Get-FileHash $media).Hash }
    $wrong = @{ "$name/fixture.img" = ('0' * 64) }
    Reject { & $tool -Targets "$name/fixture.img" -ReviewedFiles $wrong } 'Unreviewed'
    $git = Join-Path $fixture '.git'
    New-Item -ItemType Directory -Path $git | Out-Null
    Reject { & $tool -Targets $name -ReviewedFiles $reviewed } 'Git data'
    [IO.Directory]::Delete($git)
    $link = Join-Path $fixture 'junction'
    try {
        New-Item -ItemType Junction -Path $link -Target (Join-Path $repository 'src') | Out-Null
    }
    catch [System.ComponentModel.Win32Exception] {
        # Some non-elevated Windows hosts forbid creating a junction.  Keep
        # the rejection assertions on hosts that can create the fixture.
        Write-Output 'Build cleanup junction fixture: unavailable on this host.'
    }
    if (Test-Path -LiteralPath $link) {
        Reject { & $tool -Targets $name -ReviewedFiles $reviewed } 'link/junction'
        Reject { & $tool -Targets "$name/junction/lib" } 'link/junction'
        [IO.Directory]::Delete($link)
    }
    $deleted = @(& $tool -Targets "$name/ordinary.txt", "$name/fixture.img" -Apply -TasksExited -ReviewedFiles $reviewed)
    if ($deleted.Count -ne 2 -or @($deleted | Where-Object {-not $_.Deleted}).Count -or
        (Test-Path $file) -or (Test-Path $media)) { throw 'Reviewed Apply failed.' }
    & $tool -Targets $name -Apply -TasksExited | Out-Null
    Write-Output 'Build cleanup contract: OK'
}
finally {
    # Only test-owned sentinels/links may need disposal after an assertion.
    if (Test-Path (Join-Path $fixture 'junction')) { [IO.Directory]::Delete((Join-Path $fixture 'junction')) }
    if (Test-Path (Join-Path $fixture '.git')) { [IO.Directory]::Delete((Join-Path $fixture '.git')) }
    if (Test-Path $fixture) {
        $reviewed = @{}
        Get-ChildItem $fixture -File | ForEach-Object { $reviewed["$name/$($_.Name)"] = (Get-FileHash $_.FullName).Hash }
        & $tool -Targets $name -Apply -TasksExited -ReviewedFiles $reviewed | Out-Null
    }
}
