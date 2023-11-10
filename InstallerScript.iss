[Setup]
AppId={{D67217CB-1166-4023-BB9C-02E986B0B425}
AppName=Master of Mana
AppVerName=1.09
DefaultDirName={reg:HKLM\SOFTWARE\Firaxis Games\Sid Meier's Civilization 4 - Beyond the Sword,INSTALLDIR}\Mods\Master of Mana\
OutputBaseFilename=MoM Tweakmod
Compression=lzma
SolidCompression=yes
DisableProgramGroupPage=yes
SetupIconFile=F:\Games\Sid Meier's Civilization 4\Beyond the Sword\Mods\Master of Mana\icon.ico

[Types]
Name: "full"; Description: "Enjoy the glory of Master of Mana in a new way"

[Components]
Name: "MoM"; Description: "MoM Files"; Types: full; Flags: fixed

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags:

[Files]
Source: "F:\Games\Sid Meier's Civilization 4\Beyond the Sword\Mods\Master of Mana\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: MoM
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{userdesktop}\Master of Mana tweakmod"; Filename: "{reg:HKLM\SOFTWARE\Firaxis Games\Sid Meier's Civilization 4 - Beyond the Sword,INSTALLDIR}\Civ4BeyondSword.exe"; Parameters: "mod=\Master of Mana"; Tasks: desktopicon; IconFilename: "{app}\icon.ico"

[Run]
Filename: "{reg:HKLM\SOFTWARE\Firaxis Games\Sid Meier's Civilization 4 - Beyond the Sword,INSTALLDIR}\Civ4BeyondSword.exe"; Parameters: "mod=\Master of Mana"; Description: "{cm:LaunchProgram,Master of Mana}"; Flags: nowait postinstall skipifsilent