#ifndef MyAppVersion
  #define MyAppVersion "0.1.0"
#endif

[Setup]
AppId={{4E4C5E2A-BC64-4A5E-94B6-40B5E9ECF5BA}
AppName=SCREAMBACK
AppVersion={#MyAppVersion}
AppPublisher=Ilya Tolstoukhov
AppPublisherURL=https://github.com/myldy20/SCREAMBACK
AppSupportURL=https://github.com/myldy20/SCREAMBACK/issues
DefaultDirName={autopf}\SCREAMBACK
DisableProgramGroupPage=yes
LicenseFile={#SourcePath}\..\..\LICENSE
OutputDir={#SourcePath}\..\..\dist\windows
OutputBaseFilename=SCREAMBACK-{#MyAppVersion}-Windows-x64-Setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayName=SCREAMBACK VST3
CloseApplications=no

[Files]
Source: "{#SourcePath}\stage\SCREAMBACK.vst3\*"; DestDir: "{commoncf}\VST3\SCREAMBACK.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourcePath}\..\..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourcePath}\..\..\ADDITIONAL_TERMS.md"; DestDir: "{app}"; Flags: ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\SCREAMBACK.vst3"
