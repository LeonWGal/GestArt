param([string]$ReleaseDirectory='',[string]$OutputDirectory='', [string]$Version='')
$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
if(!$ReleaseDirectory){$ReleaseDirectory=Join-Path $projectRoot 'release'}
if(!$OutputDirectory){$OutputDirectory=Join-Path $projectRoot 'dist'}
$releaseRoot=[IO.Path]::GetFullPath($ReleaseDirectory)
$outputRoot=[IO.Path]::GetFullPath($OutputDirectory)
if($outputRoot.Equals($releaseRoot,[StringComparison]::OrdinalIgnoreCase) -or $outputRoot.StartsWith($releaseRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'The output directory must be outside the release directory'}
if(!$Version){$match=[regex]::Match((Get-Content -LiteralPath (Join-Path $projectRoot 'CMakeLists.txt') -Raw),'project\(GestArtUltimate VERSION ([0-9.]+)');$Version=$match.Groups[1].Value}
if($Version -notmatch '^\d+\.\d+\.\d+$'){throw 'Invalid release version'}
$icon=Join-Path $projectRoot 'assets/app-icon.ico'
if(!(Test-Path -LiteralPath $icon)){throw 'Application icon not found'}
foreach($required in @('bin/GestArtUltimate.exe','bin/Qt6Core.dll','bin/QtWebEngineProcess.exe','resources/icudtl.dat','LICENSE','THIRD_PARTY_NOTICES.md')){
    if(!(Test-Path -LiteralPath (Join-Path $releaseRoot $required))){throw ('Missing release file: '+$required)}
}
foreach($privatePath in @('data','GestArtUltimate-data','browser','test-data','settings.json','gestart.sqlite')){
    if(Test-Path -LiteralPath (Join-Path $releaseRoot $privatePath)){throw ('Do not package user data: '+$privatePath)}
}
$sensitive=Get-ChildItem -LiteralPath $releaseRoot -File -Recurse -Force | Where-Object { $_.Name -like '.env*' -or $_.Name -eq 'gestart.sqlite' -or $_.Name -eq 'download-library.json' }
if($sensitive){throw 'The release contains private configuration or a user profile'}
New-Item -ItemType Directory -Path $outputRoot -Force | Out-Null
# A portable package keeps its settings and downloaded sets in its own data directory.
Set-Content -LiteralPath (Join-Path $releaseRoot 'portable.flag') -Value $Version -Encoding ascii
$zip=Join-Path $outputRoot ('GestArt-Ultimate-'+$Version+'-windows-x64-portable.zip')
if(Test-Path -LiteralPath $zip){throw 'Portable ZIP already exists; choose an empty output directory'}
Add-Type -AssemblyName System.IO.Compression.FileSystem
[IO.Compression.ZipFile]::CreateFromDirectory($releaseRoot,$zip,[IO.Compression.CompressionLevel]::Optimal,$false)
$hash=(Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant()
$compiler=Join-Path $env:WINDIR 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
if(!(Test-Path -LiteralPath $compiler)){throw '.NET Framework C# compiler not found'}
$temporaryRoot=Join-Path $outputRoot ('build-'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $temporaryRoot -Force | Out-Null
$source=Join-Path $temporaryRoot 'PortableLauncher.cs'
$template=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'portable/PortableLauncher.cs') -Raw
$template=$template.Replace('__PAYLOAD_SHA256__',$hash).Replace('1.8.0',$Version)
[IO.File]::WriteAllText($source,$template,[Text.UTF8Encoding]::new($false))
$exe=Join-Path $outputRoot ('GestArt-Ultimate-'+$Version+'-windows-x64-portable.exe')
& $compiler /nologo /optimize+ /target:winexe /platform:x64 "/out:$exe" "/win32icon:$icon" "/win32manifest:$(Join-Path $PSScriptRoot 'portable/app.manifest')" /reference:System.IO.Compression.dll /reference:System.IO.Compression.FileSystem.dll /reference:System.Windows.Forms.dll /reference:System.Drawing.dll "/resource:$zip,payload.zip" $source
if($LASTEXITCODE -ne 0){throw 'Portable launcher compilation failed'}
# Only remove the private build directory just created under this output directory.
$resolvedTemporary=[IO.Path]::GetFullPath($temporaryRoot)
if(!$resolvedTemporary.StartsWith($outputRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Invalid temporary build path'}
Remove-Item -LiteralPath $resolvedTemporary -Recurse -Force
$lines=@($exe,$zip)|ForEach-Object{((Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash.ToLowerInvariant())+'  '+[IO.Path]::GetFileName($_)}
[IO.File]::WriteAllLines((Join-Path $outputRoot 'SHA256SUMS.txt'),$lines,[Text.UTF8Encoding]::new($false))
Get-Item -LiteralPath $exe,$zip | Select-Object Name,Length
