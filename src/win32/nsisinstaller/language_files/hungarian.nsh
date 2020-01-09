;;  Hungarian language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252

; Startup Checks
!define INSTALLER_IS_RUNNING			"A telep�t�program m�r fut."
!define EKIGA_IS_RUNNING				"A Ekiga jelenleg fut. L�pjen ki a Ekigab�l, �s pr�b�lja �jra."
!define GTK_INSTALLER_NEEDED			"A GTK+ futtat�si k�rnyezet vagy hi�nyzik, vagy �jabb verzi�j�ra van sz�ks�g.$\rTelep�tse a GTK+ futtat�si k�rnyezet v${GTK_VERSION} vagy �jabb v�ltozat�t"

; License Page
!define EKIGA_LICENSE_BUTTON			"K�vetkez� >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"Az $(^Name) a GNU General Public License (GPL) alatt ker�l forgalomba hozatalra. Itt a licenc kiz�r�lag inform�ci�s c�lokat szolg�l. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga vide�telefon (sz�ks�ges)"
!define GTK_SECTION_TITLE			"GTK+ futtat�si k�rnyezet (sz�ks�ges)"
!define GTK_THEMES_SECTION_TITLE			"GTK+ t�m�k"
!define GTK_NOTHEME_SECTION_TITLE		"Nincs t�ma"
!define GTK_WIMP_SECTION_TITLE			"Wimp t�ma"
!define GTK_BLUECURVE_SECTION_TITLE		"Bluecurve t�ma"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE		"Light House Blue t�ma"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Parancsikonok"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Asztal"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Start Men�"
!define EKIGA_SECTION_DESCRIPTION			"Alapvet� Ekiga f�jlok �s dll f�jlok"
!define GTK_SECTION_DESCRIPTION			"Az Ekiga �ltal haszn�lt t�bbplatformos GUI eszk�zk�szlet"
!define GTK_THEMES_SECTION_DESCRIPTION		"A GTK+ t�m�k megv�ltoztatj�k a GTK+ alkalmaz�sok megjelen�s�t."
!define GTK_NO_THEME_DESC			"Ne telep�tsen GTK+ t�m�t"
!define GTK_WIMP_THEME_DESC			"A GTK-Wimp (Windows megszem�lyes�t�) olyan  GTK t�ma, amely j�l illeszkedik a Windows asztali k�rnyezet�be."
!define GTK_BLUECURVE_THEME_DESC			"The Bluecurve t�ma."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"A Lighthouseblue t�ma."
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Az Ekiga ind�t�sa a Windows ind�t�sakor"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Parancsikonok a Ekiga ind�t�s�hoz"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Parancsikon l�trehoz�sa az asztalon az Ekiga sz�m�ra"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Start Men� bejegyz�s l�trehoz�sa az Ekiga sz�m�ra"

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"A rendszer egy r�gebbi GTK+ futtat�si k�rnyezetet tal�lt. K�v�nja friss�teni?$\rMegjegyz�s: Amennyiben nem v�gzi el a friss�t�st, el�fordulhat hogy az Ekiga  nem fog m�k�dni."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"L�togassa meg a windowsos Ekiga weboldal�t"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (csak elt�vol�t�s)"
!define EKIGA_RUN_AT_STARTUP			"A Ekiga futtat�sa a Windows ind�t�sakor"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"A rendszer nem k�pes az Ekiga jelenleg telep�tett verzi�j�nak elt�vol�t�s�ra. Az �j verzi� a jelenleg telep�tett v�ltozat elt�vol�t�sa n�lk�l ker�l telep�t�sre."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Hiba a GTK+ futtat�si k�rnyezet telep�t�se k�zben."
!define GTK_BAD_INSTALL_PATH			"A megadott el�r�si �t nem �rhet� el, vagy nem hozhat� l�tre."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"Nem jogosult a GTK+ t�ma telep�t�s�hez."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Az elt�vol�t� nem tal�lt Ekiga be�ll�t�sjegyz�k-bejegyz�seket.$\rAz alkalmaz�st val�sz�n�leg m�sik felhaszn�l� telep�tette."
!define un.EKIGA_UNINSTALL_ERROR_2		"Nem jogosult az alkalmaz�s elt�vol�t�s�ra."
