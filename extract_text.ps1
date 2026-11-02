param (
    [Parameter(Mandatory=$true)]
    [string]$InputFile,

    [Parameter(Mandatory=$true)]
    [string]$OutputFile
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $InputFile)) {
    Write-Error "Input file does not exist: $InputFile"
    exit 1
}

Write-Host "Parsing PE file: $InputFile"
$bytes = [System.IO.File]::ReadAllBytes($InputFile)

# PE Header offset is located at 0x3C in the DOS header
$peOffset = [BitConverter]::ToInt32($bytes, 0x3C)
if ($peOffset -ge $bytes.Length -or $peOffset -lt 0) {
    Write-Error "Invalid PE offset. Is this a valid PE file?"
    exit 1
}

# Number of sections is at PE Header + 6
$numSections = [BitConverter]::ToInt16($bytes, $peOffset + 6)

# Optional Header Size is at PE Header + 20
$optHeaderSize = [BitConverter]::ToInt16($bytes, $peOffset + 20)

# Section table starts immediately after the Optional Header (PE Header + 24 + OptHeaderSize)
$sectionTableOffset = $peOffset + 24 + $optHeaderSize

Write-Host "Found $numSections sections. Searching for '.text'..."

for ($i = 0; $i -lt $numSections; $i++) {
    $secOffset = $sectionTableOffset + ($i * 40)
    $secName = [System.Text.Encoding]::ASCII.GetString($bytes, $secOffset, 8).TrimEnd([char]0)
    
    if ($secName -eq '.text') {
        $rawSize = [BitConverter]::ToInt32($bytes, $secOffset + 16)
        $rawPtr = [BitConverter]::ToInt32($bytes, $secOffset + 20)
        
        Write-Host "Found '.text' section at offset 0x$($rawPtr.ToString('X')) (Size: $rawSize bytes)"
        
        $textBytes = New-Object byte[] $rawSize
        [Array]::Copy($bytes, $rawPtr, $textBytes, 0, $rawSize)
        
        [System.IO.File]::WriteAllBytes($OutputFile, $textBytes)
        Write-Host "Successfully extracted to: $OutputFile"
        exit 0
    }
}

Write-Error "Could not find a '.text' section in the provided PE file."
exit 1
