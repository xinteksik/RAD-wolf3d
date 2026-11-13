# GPL-3.0-or-later. Independent Windows launcher for the wolfmidi executable.
[CmdletBinding()]
param([string]$GameDirectory = '')
$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = New-Object Text.UTF8Encoding($false)
$OutputEncoding = [Console]::OutputEncoding
$names = @('CORNER','DUNGEON','WARMARCH','GETTHEM','HEADACHE','HITLWLTZ',
    'INTROCW3','NAZI_NOR','NAZI_OMI','POW','SALUTE','SEARCHN','SUSPENSE',
    'VICTORS','WONDERIN','FUNKYOU','ENDLEVEL','GOINGAFT','PREGNANT','ULTIMATE',
    'NAZI_RAP','ZEROHOUR','TWELFTH','ROSTER','URAHERO','VICMARCH','PACMAN')
$loops = @{
    CORNER=@(1,1); DUNGEON=@(4,12); GETTHEM=@(10,25,22); HITLWLTZ=@(8,1,2)
    NAZI_NOR=@(2,4,4); NAZI_OMI=@(10,9); NAZI_RAP=@(10,13); PACMAN=@(2,9,8)
    POW=@(6,3); ROSTER=@(10,49); SALUTE=@(4,3); SEARCHN=@(10,7,8)
    SUSPENSE=@(10,13); VICMARCH=@(8,2); VICTORS=@(2,2,3)
}
$required = @('AUDIOHED.WL6','AUDIOT.WL6','GAMEMAPS.WL6','MAPHEAD.WL6',
    'VGADICT.WL6','VGAGRAPH.WL6','VGAHEAD.WL6','VSWAP.WL6')
$work = $null
$staging = $null
$backup = $null
$published = $false
$log = New-Object System.Text.StringBuilder

function File-Hash([string]$Path) {
    $stream = [IO.File]::OpenRead($Path)
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try { return [BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-','').ToLowerInvariant() }
    finally { $algorithm.Dispose(); $stream.Dispose() }
}
function Direct-Child([string]$Parent, [string]$Child) {
    $base = [IO.Path]::GetFullPath($Parent).TrimEnd('\')
    $full = [IO.Path]::GetFullPath($Child)
    if (-not [string]::Equals([IO.Path]::GetDirectoryName($full).TrimEnd('\'), $base,
            [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Output path safety check failed.'
    }
    if ((Test-Path -LiteralPath $full) -and
        ((Get-Item -LiteralPath $full -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)) {
        throw 'Output directories cannot be symbolic links or junctions.'
    }
    return $full
}
function Remove-Owned([string]$Path, [string]$Parent, [string]$Prefix) {
    $checked = Direct-Child $Parent $Path
    if (-not [IO.Path]::GetFileName($checked).StartsWith($Prefix)) {
        throw 'Refusing to remove an unrecognized working directory.'
    }
    if (Test-Path -LiteralPath $checked) {
        Remove-Item -LiteralPath $checked -Recurse -Force
    }
}
function Validate-Midi([string]$Path) {
    $bytes = [IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 22 -or [Text.Encoding]::ASCII.GetString($bytes,0,4) -ne 'MThd' -or
        $bytes[7] -ne 6 -or $bytes[10] * 256 + $bytes[11] -lt 1) {
        throw ('The converter did not produce a valid MIDI file: ' + [IO.Path]::GetFileName($Path))
    }
    $tracks = $bytes[10] * 256 + $bytes[11]
    $offset = 14
    for ($track = 0; $track -lt $tracks; $track++) {
        if ($offset + 8 -gt $bytes.Length -or
            [Text.Encoding]::ASCII.GetString($bytes,$offset,4) -ne 'MTrk') {
            throw 'Invalid MIDI track structure.'
        }
        $size = [uint64]$bytes[$offset+4] * 16777216 + [uint64]$bytes[$offset+5] * 65536 +
            [uint64]$bytes[$offset+6] * 256 + $bytes[$offset+7]
        $offset += 8 + $size
        if ($offset -gt $bytes.Length) { throw 'Truncated MIDI file.' }
    }
    if ($offset -ne $bytes.Length) { throw 'Unexpected MIDI file length.' }
}

try {
    Write-Host 'RAD-Wolf - prepare all 27 MIDI music tracks'
    Write-Host 'Your game files and SoundFont are preserved. Files are not copied to an SD card automatically.'
    if ([string]::IsNullOrWhiteSpace($GameDirectory)) {
        foreach ($candidate in @($PSScriptRoot, (Join-Path $PSScriptRoot 'RADWOLF'),
                (Join-Path $PSScriptRoot 'SD-CONTENTS\RADWOLF'))) {
            if (Test-Path -LiteralPath (Join-Path $candidate 'AUDIOHED.WL6')) {
                $GameDirectory = $candidate
                break
            }
        }
        if ([string]::IsNullOrWhiteSpace($GameDirectory)) {
            throw 'Place all 8 WL6 files next to this BAT or in SD-CONTENTS\RADWOLF, then run it again.'
        }
    }
    $game = (Resolve-Path -LiteralPath $GameDirectory).ProviderPath
    if (-not (Test-Path -LiteralPath $game -PathType Container)) { throw 'Select a folder containing WL6 game data.' }
    Write-Host ('Game data: ' + $game)
    $missing = @($required | Where-Object {
        $path = Join-Path $game $_
        -not (Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -eq 0
    })
    if ($missing.Count) { throw ('Missing or empty file(s): ' + ($missing -join ', ')) }
    $headerPath = Join-Path $game 'AUDIOHED.WL6'
    $audioPath = Join-Path $game 'AUDIOT.WL6'
    if ((Get-Item -LiteralPath $headerPath).Length -gt 1048576 -or
        (Get-Item -LiteralPath $audioPath).Length -gt 33554432) { throw 'Unexpected audio data size.' }
    $header = [IO.File]::ReadAllBytes($headerPath)
    $audio = [IO.File]::ReadAllBytes($audioPath)
    if ($header.Length % 4 -ne 0 -or $header.Length -lt 112) { throw 'Invalid AUDIOHED.WL6.' }
    $offsets = [uint32[]]::new($header.Length / 4)
    for ($i = 0; $i -lt $offsets.Length; $i++) { $offsets[$i] = [BitConverter]::ToUInt32($header,$i*4) }
    $chunks = New-Object System.Collections.Generic.List[byte[]]
    for ($i = 0; $i -lt $names.Count; $i++) {
        $start = [int64]$offsets[$offsets.Length-28+$i]
        $end = [int64]$offsets[$offsets.Length-27+$i]
        if ($start -ge $end -or $end -gt $audio.Length -or $end-$start -lt 2) {
            throw ('Invalid music chunk boundaries: ' + $names[$i])
        }
        $size = [BitConverter]::ToUInt16($audio,[int]$start)
        if ($size -eq 0 -or $size % 4 -ne 0 -or $size + 2 -gt $end-$start) {
            throw ('Invalid IMF music data: ' + $names[$i])
        }
        $metadata = [Text.Encoding]::ASCII.GetString($audio,[int]($start+$size+2),[int]($end-$start-$size-2))
        if (-not $metadata.Contains($names[$i] + [char]0)) {
            throw ('Unsupported WL6 edition or damaged music: ' + $names[$i] +
                '. This package supports the full Carmackized WL6 edition.')
        }
        $chunk = [byte[]]::new($size+2)
        [Array]::Copy($audio,$start,$chunk,0,$chunk.Length)
        $chunks.Add($chunk)
    }
    $tools = Join-Path $PSScriptRoot 'tools'
    $converter = Join-Path $tools 'dro2midi.exe'
    foreach ($name in @('dro2midi.exe','inst.txt','drum.txt','patch.txt')) {
        if (-not (Test-Path -LiteralPath (Join-Path $tools $name) -PathType Leaf)) {
            throw ('Incomplete package: missing tools\' + $name + '. Extract the entire ZIP.')
        }
    }
    $inputHashes = @{}
    foreach ($name in @('AUDIOHED.WL6','AUDIOT.WL6')) { $inputHashes[$name] = File-Hash (Join-Path $game $name) }
    $toolHashes = @{}
    foreach ($name in @('dro2midi.exe','inst.txt','drum.txt','patch.txt')) { $toolHashes[$name] = File-Hash (Join-Path $tools $name) }
    $target = Direct-Child $game (Join-Path $game 'MIDI')
    $infoPath = Join-Path $target 'MIDI-INFO.json'
    $current = $false
    if (Test-Path -LiteralPath $infoPath -PathType Leaf) {
        try {
            $info = Get-Content -LiteralPath $infoPath -Raw -Encoding UTF8 | ConvertFrom-Json
            $current = $info.format -eq 1 -and @($info.tracks).Count -eq 27 -and
                @($info.tracks.name | Sort-Object -Unique).Count -eq 27
            foreach ($name in $inputHashes.Keys) { $current = $current -and $info.inputs.$name -eq $inputHashes[$name] }
            foreach ($name in $toolHashes.Keys) { $current = $current -and $info.tools.$name -eq $toolHashes[$name] }
            foreach ($record in $info.tracks) {
                if ($record.name -notin $names) { $current = $false; break }
                $path = Join-Path $target ($record.name + '.MID')
                $current = $current -and (Test-Path -LiteralPath $path -PathType Leaf)
                if ($current) { $current = (File-Hash $path) -eq $record.sha256 }
            }
        } catch { $current = $false }
    }
    if ($current) {
        Write-Host 'MIDI already matches your game data and converter. Existing files are unchanged.'
        Write-Host ('READY: copy ' + $target + ' to RADWOLF\MIDI on your SD card.')
        exit 0
    }
    if ((Test-Path -LiteralPath $target) -and -not (Test-Path -LiteralPath $target -PathType Container)) {
        throw 'A file named MIDI already exists. Move it elsewhere and run this tool again.'
    }
    $tempRoot = [IO.Path]::GetTempPath().TrimEnd('\')
    $work = Direct-Child $tempRoot (Join-Path $tempRoot ('radwolf-midi-' + [guid]::NewGuid().ToString('N')))
    New-Item -ItemType Directory -Path $work | Out-Null
    foreach ($name in @('inst.txt','drum.txt','patch.txt')) { Copy-Item -LiteralPath (Join-Path $tools $name) -Destination $work }
    $staging = Direct-Child $game (Join-Path $game ('.radwolf-midi-' + [guid]::NewGuid().ToString('N')))
    New-Item -ItemType Directory -Path $staging | Out-Null
    $records = @()
    for ($i = 0; $i -lt $names.Count; $i++) {
        $name = $names[$i]
        Write-Host ('[{0}/27] {1}' -f ($i+1),$name)
        [IO.File]::WriteAllBytes((Join-Path $work ($name+'.WLF')),$chunks[$i])
        $options = '-k 49 -i'
        if ($loops.ContainsKey($name)) {
            $keys = @('-l','-m','-n')
            for ($j = 0; $j -lt $loops[$name].Count; $j++) { $options += ' ' + $keys[$j] + ' ' + $loops[$name][$j] }
        }
        # Only generated ASCII names enter native arguments; Unicode stays in
        # the Windows working-directory API, not the converter's ANSI fopen.
        $startInfo = New-Object Diagnostics.ProcessStartInfo
        $startInfo.FileName = $converter
        $startInfo.WorkingDirectory = $work
        $startInfo.Arguments = $options + ' ' + $name + '.WLF ' + $name + '.MID'
        $startInfo.UseShellExecute = $false
        $startInfo.CreateNoWindow = $true
        $startInfo.RedirectStandardOutput = $true
        $startInfo.RedirectStandardError = $true
        $process = [Diagnostics.Process]::Start($startInfo)
        $stdout = $process.StandardOutput.ReadToEndAsync()
        $stderr = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(60000)) { $process.Kill(); throw ('Conversion timed out: ' + $name) }
        [void]$log.AppendLine($name + ': ' + $stdout.Result + $stderr.Result)
        if ($process.ExitCode -ne 0) { throw ('Conversion failed: ' + $name + '; exit code ' + $process.ExitCode) }
        $process.Dispose()
        $midi = Join-Path $work ($name+'.MID')
        Validate-Midi $midi
        Copy-Item -LiteralPath $midi -Destination $staging
        $records += @{name=$name; sha256=(File-Hash $midi); options=$options}
    }
    $manifest = @{format=1; converter='wolfmidi 2cf4c684d45de2a5f0d1b34fd6b9e64beb0c8e79';
        inputs=$inputHashes; tools=$toolHashes; tracks=$records}
    $manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $staging 'MIDI-INFO.json') -Encoding UTF8
    if (Test-Path -LiteralPath $target) {
        $backup = Direct-Child $game (Join-Path $game ('MIDI-backup-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N').Substring(0,6)))
        Move-Item -LiteralPath $target -Destination $backup
    }
    try {
        Move-Item -LiteralPath $staging -Destination $target
        $published = $true
    } catch {
        if ($backup -and -not (Test-Path -LiteralPath $target)) { Move-Item -LiteralPath $backup -Destination $target }
        throw
    }
    Write-Host ('READY: all 27 music tracks are in ' + $target)
    if ($backup) { Write-Host ('Previous MIDI folder preserved at: ' + $backup) }
    Write-Host 'Copy MIDI to RADWOLF\MIDI on your SD card. An optional SF2 belongs at RADWOLF\soundfont.sf2.'
    Write-Host 'Without a SoundFont the game uses original OPL music. Reboot after replacing an SF2.'
    exit 0
} catch {
    Write-Host ('ERROR: ' + $_.Exception.Message) -ForegroundColor Red
    Write-Host 'Game files are unchanged. Fix the reported problem and run this tool again.'
    exit 1
} finally {
    if ($log.Length) {
        try { [IO.File]::WriteAllText((Join-Path $PSScriptRoot 'MIDI-conversion.log'),$log.ToString()) } catch { }
    }
    if ($work) { Remove-Owned $work ([IO.Path]::GetTempPath().TrimEnd('\')) 'radwolf-midi-' }
    if ($staging -and -not $published) { Remove-Owned $staging $game '.radwolf-midi-' }
}
