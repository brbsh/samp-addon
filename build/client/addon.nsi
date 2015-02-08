Name "SAMP-Addon Client"
OutFile "addon.exe"

#RequestExecutionLevel user

InstallDir $EXEDIR

Section
	MessageBox MB_OK "SAMP-Addon will be installed to your PC"
	SetOutPath "$APPDATA\SAMP-Addon\"
	File updater.exe
	nsExec::ExecToLog /OEM '"$APPDATA\SAMP-Addon\updater.exe"'
	Pop $0
	Pop $1
	DetailPrint "$1"
	DetailPrint "Installer exited with return code $0 (0 = OK, Non-0 = FAIL)"
	WriteUninstaller "$APPDATA\SAMP-Addon\uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SAMP-Addon" \
                 "DisplayName" "SAMP-Addon"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SAMP-Addon" \
                 "UninstallString" "$\"$APPDATA\SAMP-Addon\uninstall.exe$\""
	#CreateShortCut "$SMPROGRAMS\SAMP-Addon\Uninstall.lnk" "$APPDATA\SAMP-Addon\uninstall.exe"
	#CreateShortCut "$SMPROGRAMS\SAMP-Addon\Manual Updater.lnk" "$APPDATA\SAMP-Addon\updater.exe"
SectionEnd

Section "uninstall"
	nsExec::ExecToLog /OEM '"$APPDATA\SAMP-Addon\updater.exe" /uninstall'
	Pop $0
	Pop $1
	DetailPrint "$1"
	DetailPrint "Uninstaller exited with return code $0 (0 = OK, Non-0 = FAIL)"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SAMP-Addon"
	#Delete "$SMPROGRAMS\SAMP-Addon\Manual Updater.lnk"
	#Delete "$SMPROGRAMS\SAMP-Addon\Uninstall.lnk"
	#RMDir "$SMPROGRAMS\SAMP-Addon"
	Delete "$APPDATA\SAMP-Addon\updater.exe"
	Delete "$APPDATA\SAMP-Addon\uninstall.exe"
	RMDir "$APPDATA\SAMP-Addon"
	MessageBox MB_OK "SAMP-Addon was uninstalled"
SectionEnd