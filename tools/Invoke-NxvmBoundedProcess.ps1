[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$FilePath,
    [string[]]$ArgumentList = @(),
    [Parameter(Mandatory = $true)]
    [int]$DeadlineSeconds,
    [string]$WorkingDirectory = '',
    [string]$DiagnosticPrefix = 'NXVM:BOUNDED-PROCESS'
)

$ErrorActionPreference = 'Stop'
if ($DeadlineSeconds -le 0) {
    throw "$DiagnosticPrefix deadline must be positive."
}

function ConvertTo-CommandLineArgument([string]$value) {
    if ($value -notmatch '[\s"]') {
        return $value
    }
    return '"' + [regex]::Replace($value, '(\\*)"', '$1$1\\"') + '"'
}

if (-not ('NxvmT382Job' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class NxvmT382Job {
    [StructLayout(LayoutKind.Sequential)] public struct BasicLimitInformation {
        public long PerProcessUserTimeLimit, PerJobUserTimeLimit;
        public uint LimitFlags;
        public UIntPtr MinimumWorkingSetSize, MaximumWorkingSetSize;
        public uint ActiveProcessLimit;
        public UIntPtr Affinity;
        public uint PriorityClass, SchedulingClass;
    }
    [StructLayout(LayoutKind.Sequential)] public struct IoCounters {
        public ulong ReadOperationCount, WriteOperationCount, OtherOperationCount;
        public ulong ReadTransferCount, WriteTransferCount, OtherTransferCount;
    }
    [StructLayout(LayoutKind.Sequential)] public struct ExtendedLimitInformation {
        public BasicLimitInformation BasicLimitInformation;
        public IoCounters IoInfo;
        public UIntPtr ProcessMemoryLimit, JobMemoryLimit;
        public UIntPtr PeakProcessMemoryUsed, PeakJobMemoryUsed;
    }
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern IntPtr CreateJobObject(IntPtr attributes, string name);
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool SetInformationJobObject(IntPtr job, int informationClass,
        ref ExtendedLimitInformation information, int informationLength);
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool AssignProcessToJobObject(IntPtr job, IntPtr process);
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool TerminateJobObject(IntPtr job, uint exitCode);
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool CloseHandle(IntPtr handle);
}
'@
}

$job = [NxvmT382Job]::CreateJobObject([IntPtr]::Zero, $null)
if ($job -eq [IntPtr]::Zero) {
    throw "$DiagnosticPrefix could not create its Windows job object."
}
$jobInformation = [NxvmT382Job+ExtendedLimitInformation]::new()
$jobInformation.BasicLimitInformation.LimitFlags = 0x00002000
if (-not [NxvmT382Job]::SetInformationJobObject(
    $job, 9, [ref]$jobInformation,
    [Runtime.InteropServices.Marshal]::SizeOf($jobInformation))) {
    [NxvmT382Job]::CloseHandle($job) | Out-Null
    throw "$DiagnosticPrefix could not enable job-object tree cleanup."
}

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $FilePath
$startInfo.Arguments = (($ArgumentList | ForEach-Object {
    ConvertTo-CommandLineArgument $_
}) -join ' ')
$startInfo.UseShellExecute = $false
$startInfo.RedirectStandardOutput = $true
$startInfo.RedirectStandardError = $true
$startInfo.CreateNoWindow = $true
if (-not [string]::IsNullOrWhiteSpace($WorkingDirectory)) {
    $startInfo.WorkingDirectory = $WorkingDirectory
}

try {
    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $startInfo
    if (-not $process.Start()) {
        throw "$DiagnosticPrefix could not start $FilePath."
    }
    if (-not [NxvmT382Job]::AssignProcessToJobObject($job, $process.Handle)) {
        throw "$DiagnosticPrefix could not assign pid $($process.Id) to its cleanup job."
    }
    $stdoutTask = $process.StandardOutput.ReadToEndAsync()
    $stderrTask = $process.StandardError.ReadToEndAsync()
    if (-not $process.WaitForExit($DeadlineSeconds * 1000)) {
        if (-not [NxvmT382Job]::TerminateJobObject($job, 124)) {
            throw "$DiagnosticPrefix could not terminate its owned job tree."
        }
        $process.WaitForExit()
        $stdout = $stdoutTask.GetAwaiter().GetResult()
        $stderr = $stderrTask.GetAwaiter().GetResult()
        Write-Output $stdout
        throw "${DiagnosticPrefix}:DEADLINE: pid=$($process.Id); seconds=$DeadlineSeconds`n$stderr"
    }
    $stdout = $stdoutTask.GetAwaiter().GetResult()
    $stderr = $stderrTask.GetAwaiter().GetResult()
    Write-Output $stdout
    if ($process.ExitCode -ne 0) {
        throw "${DiagnosticPrefix}:FAILED: exit=$($process.ExitCode); pid=$($process.Id)`n$stderr"
    }
    if (-not [string]::IsNullOrWhiteSpace($stderr)) {
        Write-Output $stderr
    }
}
finally {
    if ($job -ne [IntPtr]::Zero) {
        [NxvmT382Job]::CloseHandle($job) | Out-Null
    }
}
