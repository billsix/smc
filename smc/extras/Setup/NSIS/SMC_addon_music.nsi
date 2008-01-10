; ########################################################
; # Secret Maryo Chronicles Addon : Music NSIS Installer #
; # http://nsis.sourceforge.net                          #
; ########################################################


;----------- Configuration ---------------------


; Installer Name
Name "Secret Maryo Chronicles Addon : Music Pack"
; Setup Filename
OutFile "SMC_Music_Setup_4.0.exe"

; Set Compression
SetCompressor lzma
; Set Icon
Icon "icon.ico"

; default music installation directory
InstallDir "$PROGRAMFILES\Secret Maryo Chronicles"

; Registry key to check for the SMC directory
InstallDirRegKey HKLM "Software\secretmaryo" "install_dir"


;----------- Include Modern UI ---------------------

!include "MUI.nsh"

;------------ Interface Settings --------------------

!define MUI_ICON ${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico
!define MUI_UNICON ${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "installer_logo.bmp" ; optional

; Components
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_COMPONENTSPAGE_CHECKBITMAP ${NSISDIR}\Contrib\Graphics\Checks\simple-round2.bmp

;---------- Pages ----------------------

!insertmacro MUI_PAGE_LICENSE "..\..\..\docs\license.txt"
;!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;----------- Languages ---------------------
; first language is the default language

!insertmacro MUI_LANGUAGE "English"

;---------- Functions ----------------------

; on init
Function .onInit

 	; create mutex to check if only only one installer runs
 	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "SMC_Mutex") i .r1 ?e'
 	Pop $R0
 
 	StrCmp $R0 0 +3
   		MessageBox MB_OK|MB_ICONEXCLAMATION "Secret Maryo Chronicles installer is already running."
   		Abort

FunctionEnd

;---------- install types ----------------------

InstType "Default"

;---------- The stuff to install ----------------------

; Music Addon
Section "!Secret Maryo Chronicles Addon : Music" Sec_Music

  	; required
  	SectionIn 1 RO
  
  	; Set output path to the installation data directory
  	SetOutPath "$INSTDIR\data"

  	; Installation Directories
  	FIle /r "..\..\..\data\music"

 	; Set output path to the installation docs directory
	SetOutPath "$INSTDIR\docs"
  	FIle "Addon - Music Readme.txt"

	; Set default installation directory output path
	SetOutPath $INSTDIR

  	; Write the installation path into the registry
  	WriteRegStr HKLM SOFTWARE\secretmaryo\music "install_dir" "$INSTDIR"
  
  	; Write the uninstall keys for Windows
  	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\secretmaryo_music" "DisplayName" "Secret Maryo Chronicles Addon : Music"
  	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\secretmaryo_music" "UninstallString" '"$INSTDIR\uninstall_music.exe"'
  	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\secretmaryo_music" "NoModify" 1
  	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\secretmaryo_music" "NoRepair" 1
  	WriteUninstaller "uninstall_music.exe"
  
SectionEnd

;------------- Descriptions -------------------

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${Sec_Music} "SMC Addon : Music files (required)"
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;------------ Uninstaller --------------------


Section "Uninstall"
  
	; Delete Music
	Delete "$INSTDIR\data\music\game\*.ogg"
	Delete "$INSTDIR\data\music\game\*.txt"
	Delete "$INSTDIR\data\music\land\*.ogg"
	Delete "$INSTDIR\data\music\land\*.txt"
	Delete "$INSTDIR\data\music\overworld\*.ogg"
	Delete "$INSTDIR\data\music\overworld\*.txt"
	Delete "$INSTDIR\data\music\story\*.ogg"
	Delete "$INSTDIR\data\music\story\*.txt"
	RMDir "$INSTDIR\data\music\game"
	RMDir "$INSTDIR\data\music\land"
	RMDir "$INSTDIR\data\music\overworld"
	RMDir "$INSTDIR\data\music\story"
	RMDir "$INSTDIR\data\music"

	; Remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\secretmaryo_music"
	DeleteRegKey HKLM SOFTWARE\secretmaryo\music

	; Delete Music Readme
	Delete "$INSTDIR\docs\Addon - Music Readme.txt"

	; Remove uninstaller
	Delete $INSTDIR\uninstall_music.exe

SectionEnd
