; Define your application name
!define APPNAME "Tredifarm"
!define APPNAMEANDVERSION "Tredifarm 0.1"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\TrediFarm"
InstallDirRegKey HKLM "Software\${APPNAME}" ""
OutFile "C:\Distrid\tredifarm_install.exe"
!define MUI_ICON "C:\Distrid\install\appicon.ico"
Icon "${MUI_ICON}"
WindowIcon on

; Use compression

SetCompressor LZMA

; Modern interface settings
!include "MUI.nsh"
!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\Tredifarm.exe"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_RESERVEFILE_LANGDLL

Section "Tredifarm" Section1

	SetOverwrite on
	
	SetOutPath "$INSTDIR\vcredist\"
	File "C:\Distrid\install\vcredist_sp1_x86.exe"
	
	SetOutPath "$INSTDIR\local\"
	File "C:\Distrid\install\config.ini"
	
	SetOutPath "$INSTDIR\platforms\"
	File "C:\Distrid\install\qwindows.dll"

	SetOutPath "$INSTDIR\"
	File "C:\Distrid\install\Qt5Core.dll"
	File "C:\Distrid\install\Qt5Gui.dll"
	File "C:\Distrid\install\Qt5Network.dll"
	File "C:\Distrid\install\Qt5Sql.dll"
	File "C:\Distrid\install\Qt5Widgets.dll"
	File "C:\Distrid\install\Tredifarm.exe"
	File "C:\Distrid\install\core.dll"
	File "C:\Distrid\install\transport.dll"
	
	CreateShortCut "$DESKTOP\Tredifarm.lnk" "$INSTDIR\Tredifarm.exe"
	CreateDirectory "$SMPROGRAMS\Tredifarm"
	CreateShortCut "$SMPROGRAMS\Tredifarm\Tredifarm.lnk" "$INSTDIR\Tredifarm.exe"
	CreateShortCut "$SMPROGRAMS\Tredifarm\tredifarm_uninstall.lnk" "$INSTDIR\uninstall.exe"
	
	ExecWait "$INSTDIR\vcredist\vcredist_sp1_x86.exe /passive"

SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\tredifarm_uninstall.exe"
	WriteUninstaller "$INSTDIR\tredifarm_uninstall.exe"

SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${Section1} "Œœ»—¿Õ»≈ œ–Œ√–¿ÃÃ€"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall

	;Remove from registry...
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
	DeleteRegKey HKLM "SOFTWARE\${APPNAME}"

	; Delete self
	Delete "$INSTDIR\tredifarm_uninstall.exe"

	; Delete Shortcuts
	Delete "$DESKTOP\Tredifarm.lnk"
	Delete "$SMPROGRAMS\Tredifarm\Tredifarm.lnk"
	Delete "$SMPROGRAMS\Tredifarm\Uninstall.lnk"

	; Clean up Tredifarm
	Delete "$INSTDIR\Qt5Core.dll"
	Delete "$INSTDIR\Qt5Gui.dll"
	Delete "$INSTDIR\Qt5Network.dll"
	Delete "$INSTDIR\Qt5Sql.dll"
	Delete "$INSTDIR\Qt5Widgets.dll"
	Delete "$INSTDIR\Tredifarm.exe"
	Delete "$INSTDIR\core.dll"
	Delete "$INSTDIR\transport.dll"
	Delete "$INSTDIR\config.ini"
	Delete "$INSTDIR\local\data.db"
	Delete "$INSTDIR\local\config.ini"
	Delete "$INSTDIR\platforms\qwindows.dll"
	Delete "$INSTDIR\vcredist\vcredist_sp1_x86.exe"

	; Remove remaining directories
	RMDir "$INSTDIR\vcredist"
	RMDir "$INSTDIR\local"
	RMDir "$INSTDIR\platforms"
	RMDir "$SMPROGRAMS\Tredifarm"
	RMDir "$INSTDIR\"

SectionEnd

; On initialization
Function .onInit

	;!insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

; eof