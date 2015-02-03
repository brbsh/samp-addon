Name "SAMP-Addon Client"
OutFile "addon.exe"

InstallDir $EXEDIR

Section
	MessageBox MB_OK "SAMP-Addon will be installed to your PC"
	SetOutPath "$TEMP\SAMP-Addon\"
	File updater.exe
	nsExec::ExecToLog /OEM '"$TEMP\SAMP-Addon\updater.exe"'
	Pop $0
	Pop $1
	DetailPrint "$1"
	DetailPrint "Installer exited with return code $0 (0 = OK, Non-0 = FAIL)"
	WriteUninstaller "$TEMP\SAMP-Addon\uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SAMP-Addon" \
                 "DisplayName" "SAMP-Addon"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SAMP-Addon" \
                 "UninstallString" "$\"$TEMP\SAMP-Addon\uninstall.exe$\""
	CreateShortCut "$SMPROGRAMS\SAMP-Addon\Update (Manual).lnk" "$TEMP\SAMP-Addon\updater.exe"
	CreateShortCut "$SMPROGRAMS\SAMP-Addon\Uninstall.lnk" "$TEMP\SAMP-Addon\uninstall.exe"
SectionEnd

Section "uninstall"
	nsExec::ExecToLog /OEM '"$TEMP\SAMP-Addon\updater.exe" /uninstall'
	Pop $0
	Pop $1
	DetailPrint "$1"
	DetailPrint "Uninstaller exited with return code $0 (0 = OK, Non-0 = FAIL)"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SAMP-Addon"
	Delete "$SMPROGRAMS\SAMP-Addon\Update (Manual).lnk"
	Delete "$SMPROGRAMS\SAMP-Addon\Uninstall.lnk"
	RMDir "$SMPROGRAMS\SAMP-Addon"
	Delete "$TEMP\SAMP-Addon\updater.exe"
	Delete "$TEMP\SAMP-Addon\uninstall.exe"
	RMDir "$TEMP\SAMP-Addon"
	MessageBox MB_OK "SAMP-Addon was uninstalled"
SectionEnd