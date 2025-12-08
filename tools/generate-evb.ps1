# Generate Enigma Virtual Box project file (.evb) for bundling Qt DLLs into a single executable
# Usage: .\generate-evb.ps1 -InputExe "path\to\input.exe" -OutputExe "path\to\output.exe" -EvbFile "path\to\project.evb"

param(
    [Parameter(Mandatory=$true)]
    [string]$InputExe,
    
    [Parameter(Mandatory=$true)]
    [string]$OutputExe,
    
    [Parameter(Mandatory=$true)]
    [string]$EvbFile
)

# Get the directory containing the input exe (where windeployqt put the DLLs)
$inputDir = Split-Path -Parent $InputExe
$inputExeName = Split-Path -Leaf $InputExe

# Function to recursively build file entries for the EVB XML
function Build-FileEntries {
    param(
        [string]$Path,
        [string]$ExcludeExe
    )
    
    $entries = ""
    
    Get-ChildItem -Path $Path | ForEach-Object {
        if ($_.PSIsContainer) {
            # Directory
            $subEntries = Build-FileEntries -Path $_.FullName -ExcludeExe ""
            $entries += @"
          <File>
            <Type>3</Type>
            <Name>$($_.Name)</Name>
            <Action>0</Action>
            <OverwriteDateTime>False</OverwriteDateTime>
            <OverwriteAttributes>False</OverwriteAttributes>
            <HideFromDialogs>0</HideFromDialogs>
            <Files>$subEntries</Files>
          </File>
"@
        } elseif ($_.Name -ne $ExcludeExe) {
            # File (exclude the main exe)
            $entries += @"
          <File>
            <Type>2</Type>
            <Name>$($_.Name)</Name>
            <File>$($_.FullName)</File>
            <ActiveX>False</ActiveX>
            <ActiveXInstall>False</ActiveXInstall>
            <Action>0</Action>
            <OverwriteDateTime>False</OverwriteDateTime>
            <OverwriteAttributes>False</OverwriteAttributes>
            <PassCommandLine>False</PassCommandLine>
            <HideFromDialogs>0</HideFromDialogs>
          </File>
"@
        }
    }
    
    return $entries
}

# Build the file entries (excluding the main exe)
$fileEntries = Build-FileEntries -Path $inputDir -ExcludeExe $inputExeName

# Generate the EVB XML content
$evbContent = @"
<?xml version="1.0" encoding="windows-1252"?>
<>
  <InputFile>$InputExe</InputFile>
  <OutputFile>$OutputExe</OutputFile>
  <Files>
    <Enabled>True</Enabled>
    <DeleteExtractedOnExit>True</DeleteExtractedOnExit>
    <CompressFiles>True</CompressFiles>
    <Files>
      <File>
        <Type>3</Type>
        <Name>%DEFAULT FOLDER%</Name>
        <Action>0</Action>
        <OverwriteDateTime>False</OverwriteDateTime>
        <OverwriteAttributes>False</OverwriteAttributes>
        <HideFromDialogs>0</HideFromDialogs>
        <Files>$fileEntries</Files>
      </File>
    </Files>
  </Files>
  <Registries>
    <Enabled>False</Enabled>
    <Registries>
      <Registry>
        <Type>1</Type>
        <Virtual>True</Virtual>
        <Name>Classes</Name>
        <ValueType>0</ValueType>
        <Value/>
        <Registries/>
      </Registry>
      <Registry>
        <Type>1</Type>
        <Virtual>True</Virtual>
        <Name>User</Name>
        <ValueType>0</ValueType>
        <Value/>
        <Registries/>
      </Registry>
      <Registry>
        <Type>1</Type>
        <Virtual>True</Virtual>
        <Name>Machine</Name>
        <ValueType>0</ValueType>
        <Value/>
        <Registries/>
      </Registry>
      <Registry>
        <Type>1</Type>
        <Virtual>True</Virtual>
        <Name>Users</Name>
        <ValueType>0</ValueType>
        <Value/>
        <Registries/>
      </Registry>
      <Registry>
        <Type>1</Type>
        <Virtual>True</Virtual>
        <Name>Config</Name>
        <ValueType>0</ValueType>
        <Value/>
        <Registries/>
      </Registry>
    </Registries>
  </Registries>
  <Packaging>
    <Enabled>False</Enabled>
  </Packaging>
  <Options>
    <ShareVirtualSystem>False</ShareVirtualSystem>
    <MapExecutableWithTemporaryFile>True</MapExecutableWithTemporaryFile>
    <TemporaryFileMask/>
    <AllowRunningOfVirtualExeFiles>True</AllowRunningOfVirtualExeFiles>
    <ProcessesOfAnyPlatforms>False</ProcessesOfAnyPlatforms>
  </Options>
  <Storage>
    <Files>
      <Enabled>False</Enabled>
      <Folder>%DEFAULT FOLDER%\</Folder>
      <RandomFileNames>False</RandomFileNames>
      <EncryptContent>False</EncryptContent>
    </Files>
  </Storage>
</>
"@

# Write the EVB file
$evbContent | Out-File -FilePath $EvbFile -Encoding UTF8

Write-Host "Generated EVB project file: $EvbFile"
Write-Host "Input executable: $InputExe"
Write-Host "Output executable: $OutputExe"
