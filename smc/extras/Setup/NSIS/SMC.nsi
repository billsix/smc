; ##########################################
; # Secret Maryo Chronicles NSIS Installer #
; # http://nsis.sourceforge.net            #
; ##########################################

!define PRODUCT_NAME "Secret Maryo Chronicles"
!define PRODUCT_VERSION "1.4"
!define PRODUCT_PUBLISHER "Florian Richter"
!define PRODUCT_WEB_SITE "http://www.secretmaryo.org"
!define PRODUCT_DIR_REGKEY "Software\secretmaryo"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\secretmaryo"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

;----------- Configuration ---------------------

; Setup Filename
OutFile "SMC_${PRODUCT_VERSION}_win32.exe"
; Installer Name
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"

; Set Compression
SetCompressor lzma
; Set Icon
Icon "icon.ico"

; default installation directory
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"

; Registry key to check for directory ( if you install again, it overwrites the old one automatically )
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" "install_dir"

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

;------------- Reserve Files -------------------

;These files should be inserted before other files in the data block
;Keep these lines before any File command
;Only for solid compression (by default, solid compression is enabled for BZIP2 and LZMA)

ReserveFile "custom_page_end.ini"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;-------------- Variables ------------------

Var INI_VALUE_HOMEPAGE
Var INI_VALUE_CONTROLS
Var INI_VALUE_DONATE

;---------- Pages ----------------------

!insertmacro MUI_PAGE_LICENSE "..\..\..\docs\license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
Page custom CustomPage_end

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
   MessageBox MB_OK|MB_ICONEXCLAMATION "${PRODUCT_NAME} installer is already running."
   Abort

 ReadRegStr $R0 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
 ;Run the uninstaller
 StrCmp $R0 "" skip_uninstall
   MessageBox MB_YESNO|MB_ICONEXCLAMATION "${PRODUCT_NAME} is already installed. $\nYou should remove the previous version before installing $(^Name) ?" IDNO skip_uninstall
   ClearErrors
   ;Don't copy the uninstaller to a temp file
   ExecWait '$R0 _?=$INSTDIR'
 skip_uninstall:

 ; custom end page
 !insertmacro MUI_INSTALLOPTIONS_EXTRACT "custom_page_end.ini"

FunctionEnd

; custom end page
Function CustomPage_end

  !insertmacro MUI_HEADER_TEXT "${PRODUCT_NAME} successfully installed" "Choose which options to enable"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "custom_page_end.ini"

FunctionEnd

; on successful end
Function .onInstSuccess

  ;Read  the value from the custom page INI file
  !insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE_HOMEPAGE "custom_page_end.ini" "Field 2" "State"
  !insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE_CONTROLS "custom_page_end.ini" "Field 3" "State"
  !insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE_DONATE "custom_page_end.ini" "Field 5" "State"

  ; Visit Homepage if checked
  StrCmp $INI_VALUE_HOMEPAGE "1" "" +2
    ExecShell "open" "${PRODUCT_WEB_SITE}/"

  ; Open Controls if checked
  StrCmp $INI_VALUE_CONTROLS "1" "" +2
    ExecShell "open" "docs\controls.html"

  ; Visit Donate Homepage if checked
  StrCmp $INI_VALUE_DONATE "1" "" +2
    ExecShell "open" "${PRODUCT_WEB_SITE}/?page=donate"

FunctionEnd

;---------- install types ----------------------

InstType "Default"

;---------- The stuff to install ----------------------

; Main Game
Section "!${PRODUCT_NAME}" Sec_SMC

  ; required
  SectionIn 1 RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Create Directories

  ; Installation Directories
  FIle /r "..\..\..\data"
  FIle /r "..\..\..\Microsoft.VC80.CRT"
  FIle /r "..\..\..\docs"
  ; Installation Files
  FIle "..\..\..\Secret Maryo Chronicles.exe"
  FIle /r "..\..\..\*.dll"
  FIle /r "..\..\..\*.manifest"
  FIle /r "..\..\..\*.xsd"

  ; Write the installation path into the registry
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "install_dir" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "version" "${PRODUCT_VERSION}"
  
  ; Write the uninstall keys for Windows
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Secret Maryo Chronicles.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Source Files
Section "Source Code" Sec_Source

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Installation Directories
  FIle /r "..\..\..\makefiles"
  FIle /r "..\..\..\src"
  
SectionEnd

; Optional Start Menu Shortcuts
Section "Start Menu Shortcuts" Sec_Menu_Shortcut

  ; default is enabled
  SectionIn 1

  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0 SW_SHOWNORMAL "" "Uninstall ${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\Secret Maryo Chronicles.exe" "" "$INSTDIR\Secret Maryo Chronicles.exe" 0 SW_SHOWNORMAL "" "Start ${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Controls.lnk" "$INSTDIR\docs\controls.html" "" "$INSTDIR\docs\controls.html" 0 SW_SHOWNORMAL "" "Show the Controls manual"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Homepage.lnk" "${PRODUCT_WEB_SITE}/" "" "${PRODUCT_WEB_SITE}/" 0 SW_SHOWNORMAL "" "Visit the ${PRODUCT_NAME} Homepage"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Donate.lnk" "${PRODUCT_WEB_SITE}/?page=donate" "" "${PRODUCT_WEB_SITE}/?page=donate" 0 SW_SHOWNORMAL "" "Donate for ${PRODUCT_NAME}"

SectionEnd

; Optional Desktop Shortcut
Section "Desktop Shortcut" Sec_Desktop_Shortcut

  ; default is enabled
  SectionIn 1

  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\Secret Maryo Chronicles.exe" "" "$INSTDIR\Secret Maryo Chronicles.exe" 0 SW_SHOWNORMAL "" "Start ${PRODUCT_NAME}"
  
SectionEnd

;------------- Descriptions -------------------

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${Sec_SMC} "${PRODUCT_NAME} Game files (required)"
    !insertmacro MUI_DESCRIPTION_TEXT ${Sec_Source} "Game Source Code only intended for Developers"
    !insertmacro MUI_DESCRIPTION_TEXT ${Sec_Menu_Shortcut} "Startmenu Shortcuts"
    !insertmacro MUI_DESCRIPTION_TEXT ${Sec_Desktop_Shortcut} "Desktop Shortcut"
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;------------ Uninstaller --------------------


Section "Uninstall"
  
  MessageBox MB_YESNO "Do you want to save your Levels, Worlds, Savegames and Screenshots ?" IDYES true IDNO false
  true:
    ;nothing
  Goto next
  false:
    ; user directory
    Delete "$APPDATA\smc\config.xml"
    RMDir /r "$APPDATA\smc\savegames"
    RMDir /r "$APPDATA\smc\screenshots"
    RMDir /r "$APPDATA\smc\levels"
    RMDir /r "$APPDATA\smc\worlds"
    RMDir "$APPDATA\smc"
    ; game directory
    Delete "$INSTDIR\data\levels\*.smclvl"
    Delete "$INSTDIR\data\Makefile.am"
    Delete "$INSTDIR\configure"
    Delete "$INSTDIR\install-sh"
    Delete "$INSTDIR\SConstruct"
    RMDir /r "$INSTDIR\data\world"
    RMDir /r "$INSTDIR\src"
    RMDir /r "$INSTDIR\makefiles"
    ; old stuff
    Delete "$INSTDIR\config.xml"
    Delete "$INSTDIR\savegames\*.save"
    Delete "$INSTDIR\screenshots\*.bmp"
    Delete "$INSTDIR\data\levels\*.txt"
    Delete "$INSTDIR\data\levels\*.lvl"
  next:

  ; Remove registry keys
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegValue HKLM "${PRODUCT_DIR_REGKEY}" "version"

  ; Remove main directoy files
  Delete "$INSTDIR\Secret Maryo Chronicles.exe"
  Delete "$INSTDIR\*.ac"
  Delete "$INSTDIR\*.am"
  Delete "$INSTDIR\*.sh"
  Delete "$INSTDIR\*.log"
  Delete "$INSTDIR\stdout.txt"
  Delete "$INSTDIR\stderr.txt"
  Delete "$INSTDIR\*.manifest"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.xsd"

  ; Remove data files
  RMDir /r "$INSTDIR\data\editor"
  RMDir /r "$INSTDIR\data\gui"
  RMDir /r "$INSTDIR\data\icon"
  RMDir /r "$INSTDIR\data\music"
  RMDir /r "$INSTDIR\data\pixmaps"
  RMDir /r "$INSTDIR\data\schema"
  RMDir /r "$INSTDIR\data\sounds"

  ; Remove directories if empty
  RMDir "$INSTDIR\data\world"
  RMDir "$INSTDIR\data\levels"
  RMDir "$INSTDIR\data"
  RMDir "$INSTDIR\savegames"
  RMDir "$INSTDIR\screenshots"
  RMDir /r "$INSTDIR\docs"
  RMDir /r "$INSTDIR\Microsoft.VC80.CRT"
  RMDir "$INSTDIR\src"
  RMDir "$INSTDIR\makefiles"

  ; Remove uninstaller
  Delete $INSTDIR\uninstall.exe

  ; Remove installation directories if empty
  RMDir "$INSTDIR"
  ; Remove startmenu directory
  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
  ; Remove desktop shortcut
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  SetAutoClose true

SectionEnd
