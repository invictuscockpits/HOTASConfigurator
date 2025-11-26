; Inno Setup Script for Invictus HOTAS Configurator
; Installer with automatic upgrade support for previous versions

#define INNO_SETUP
#include "src\version.h"
#define MyAppName "Invictus HOTAS Configurator"
#define MyAppVersion APP_VERSION
#define MyAppPublisher "Invictus Cockpit Systems"
#define MyAppURL "https://invictuscockpits.com"
#define MyAppExeName "InvictusHOTASConfigurator.exe"
#define SourceDir "deploy"


[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; IMPORTANT: Keep this GUID consistent across versions to enable automatic upgrades.
AppId={{DE67475C-2799-4D59-8A23-27F028E18C42}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL=https://github.com/invictuscockpits/HOTASConfigurator/releases
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
; Allow installation without admin rights
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
; Automatically use previous installation directory if upgrading
UsePreviousAppDir=yes
; Output configuration
OutputDir=installer_output
OutputBaseFilename=InvictusHOTASConfigurator-v{#MyAppVersion}-Setup
SetupIconFile=src\Images\icon.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
; Compression settings
Compression=lzma2/max
SolidCompression=yes
; UI settings
WizardStyle=modern
; Version info
VersionInfoVersion={#MyAppVersion}
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} Setup
VersionInfoCopyright=Copyright (C) 2025 {#MyAppPublisher}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Dirs]
Name: "{app}\configs"

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files


[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

