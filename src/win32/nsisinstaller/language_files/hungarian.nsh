;;
;;  hungarian.nsh
;;
;;  Default language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252
;;
;;  Version 3
;;  Note: If translating this file, replace "!define"
;;  with "!define".

; Make sure to update the EKIGA_MACRO_LANGUAGEFILE_END macro in
; langmacros.nsh when updating this file

; Startup Checks
!define INSTALLER_IS_RUNNING			"A telep�t�program m�r fut."
!define EKIGA_IS_RUNNING				"A Ekiga jelenleg fut. L�pjen ki a Ekiga-b�l �s pr�b�lja ism�t."
!define GTK_INSTALLER_NEEDED			"A GTK+ futtat�si k�rnyezet vagy hi�nyzik, vagy �jabb verzi�j�ra van sz�ks�g $\r K�rj�k, telep�tse a v${GTK_VERSION} vagy a GTK+ futtat�si k�rnyezet frissebb v�ltozat�t"

; License Page
!define EKIGA_LICENSE_BUTTON			"K�vetkez� >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) A GNU General Public License (GPL) neve alatt ker�l forgalomba hozatalra. Itt a licenc kiz�r�lag inform�ci�s c�lokra szolg�l. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga vide�telefon (sz�ks�ges)"
!define GTK_SECTION_TITLE			"GTK+ futtat�si k�rnyezet (sz�ks�ges)"
!define GTK_THEMES_SECTION_TITLE			"GTK+ t�m�k"
!define GTK_NOTHEME_SECTION_TITLE		"Nincs t�ma"
!define GTK_WIMP_SECTION_TITLE			"Wimp t�ma"
!define GTK_BLUECURVE_SECTION_TITLE		"Bluecurve t�ma"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE		"Light House Blue t�ma"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Parancsikonok"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Munkaasztal"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Start Men�"
!define EKIGA_SECTION_DESCRIPTION			"Alapvet� Ekiga f�jlok �s dll f�jlok"
!define GTK_SECTION_DESCRIPTION			"A NeophoeX �ltal haszn�lt multi-platform GUI eszk�zt�r"
!define GTK_THEMES_SECTION_DESCRIPTION		"A GTK+ t�m�k megv�ltoztatj�k a GTK+ alkalmaz�sok megjelen�s�t �s �rz�svil�g�t."
!define GTK_NO_THEME_DESC			"Ne telep�tsen GTK+ t�m�t"
!define GTK_WIMP_THEME_DESC			"A GTK-Wimp (Windows megszem�lyes�t�) olyan  GTK t�ma, amely j�l illeszkedik a Windows munkaasztal k�rnyezet�be."
!define GTK_BLUECURVE_THEME_DESC			"The Bluecurve t�ma."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"The Lighthouseblue t�ma."
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Parancsikonok a Ekiga ind�t�s�hoz"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Parancsikont hoz l�tre a munkaasztalon, amely seg�ts�g�vel a Ekiga el�rhet�"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Start Men� bejegyz�st hoz l�tre a Ekiga sz�m�ra"

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"A rendszer egy r�gebbi GTK+ futtat�s k�rnyezetet tal�lt. K�v�nja friss�teni? $\r Megjegyz�s: Amennyiben nem  v�gzi el a friss�t�st, fenn�ll annak vesz�lye, hogy a Ekiga  nem fog m�k�dni."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"L�togassas meg a a Windows Ekiga weboldalt"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (csak elt�vol�t�s)"
!define EKIGA_RUN_AT_STARTUP			"A Ekiga futtat�sa a Windows ind�t�sakor"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"A rendszer nem k�pes a Ekiga jelenleg telep�tett verzi�j�nak elt�vol�t�s�ra. Az �j verzi� a jelenleg telep�tett v�ltozat elt�vol�t�sa n�lk�l ker�l telep�t�sre."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Hiba a GTK+ futtat�s k�rnyezet telep�t�se k�zben."
!define GTK_BAD_INSTALL_PATH			"A megadott el�r�si �t nem �rhet� el, vagy nem hozhat� l�tre."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"Nincs enged�lye a GTK+ t�ma telep�t�s�hez."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Az elt�vol�t� nem tal�lt Ekiga k�nyvt�ri bejegyz�seket.$\rIt az alkalmaz�st val�sz�n�leg m�sik felhaszn�l� telep�tette."
!define un.EKIGA_UNINSTALL_ERROR_2		"Nem jogosult az alkalmaz�s elt�vol�t�s�ra."

; Spellcheck Section Prompts
!define EKIGA_SPELLCHECK_SECTION_TITLE		"Helyes�r�sellen�rz�si t�mogat�s"
!define EKIGA_SPELLCHECK_ERROR			"Hiba a helyes�r�sellen�rz� telep�t�se k�zben"
!define EKIGA_SPELLCHECK_DICT_ERROR		"Hiba a helyes�r�sellen�rz� sz�t�r telep�t�se k�zben"
!define EKIGA_SPELLCHECK_SECTION_DESCRIPTION	"T�mogat�s a helyes�r�sellen�rz�shez .  (A telep�t�shez Internet kapcsolatra van sz�ks�g)"
!define ASPELL_INSTALL_FAILED			"A telep�t�s sikertelen"
!define EKIGA_SPELLCHECK_BRETON			"Breton"
!define EKIGA_SPELLCHECK_CATALAN			"Katal�n"
!define EKIGA_SPELLCHECK_CZECH			"Cseh"
!define EKIGA_SPELLCHECK_WELSH			"Welsh"
!define EKIGA_SPELLCHECK_DANISH			"D�n"
!define EKIGA_SPELLCHECK_GERMAN			"N�met"
!define EKIGA_SPELLCHECK_GREEK			"G�r�g"
!define EKIGA_SPELLCHECK_ENGLISH			"Angol"
!define EKIGA_SPELLCHECK_ESPERANTO		"Eszperant�"
!define EKIGA_SPELLCHECK_SPANISH			"Spanyol"
!define EKIGA_SPELLCHECK_FAROESE			"Fer�er szigeteki"
!define EKIGA_SPELLCHECK_FRENCH			"Francia"
!define EKIGA_SPELLCHECK_ITALIAN			"Olasz"
!define EKIGA_SPELLCHECK_DUTCH			"Holland"
!define EKIGA_SPELLCHECK_NORWEGIAN		"Norv�g"
!define EKIGA_SPELLCHECK_POLISH			"Lengyel"
!define EKIGA_SPELLCHECK_PORTUGUESE		"Portug�l"
!define EKIGA_SPELLCHECK_ROMANIAN			"Rom�n"
!define EKIGA_SPELLCHECK_RUSSIAN			"Orosz"
!define EKIGA_SPELLCHECK_SLOVAK			"Szlov�k"
!define EKIGA_SPELLCHECK_SWEDISH			"Sv�d"
!define EKIGA_SPELLCHECK_UKRAINIAN		"Ukr�n"

