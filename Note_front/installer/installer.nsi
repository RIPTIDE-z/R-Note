Unicode true
!include "MUI2.nsh"

!define APP_NAME    "R-Note"
!define APP_VERSION "1.0.0"
!define SOURCE_DIR  "..\bin\Release"     

Name "${APP_NAME}"
OutFile "${APP_NAME}_Setup.exe"

InstallDir "$PROGRAMFILES\${APP_NAME}"

!define MUI_ICON    "note.ico"
!define MUI_UNICON  "note.ico"

!define MUI_BGCOLOR  "FFFFFF"
!define MUI_TEXTCOLOR "000000"

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME         
!insertmacro MUI_PAGE_COMPONENTS       
!insertmacro MUI_PAGE_DIRECTORY        
!insertmacro MUI_PAGE_INSTFILES        
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "SimpChinese"

Section "主程序" SEC_MAIN
  SectionIn RO                
  SetOutPath "$INSTDIR"
  File /r "${SOURCE_DIR}\*.*"
SectionEnd

Section "创建桌面快捷方式" SEC_DESKTOP
  CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\R-Note.exe"
SectionEnd
