;;  Russian language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1251
;;  Author: Alexey Loukianov a.k.a. LeXa2
;;
;;  Note: To translate this file:
;;  - download this file on your computer
;;  - translate all the strings into your language
;;  - put the appropriate Windows Code Page (the one you use) above
;;  - add yourself as Author above
;;  - send us the file and remind us:
;;    - to add the entry for your file in ekiga.nsi
;;      (MUI_LANGUAGE and EKIGA_MACRO_INCLUDE_LANGFILE)
;;    - to replace everywhere in your file
;;      "!insertmacro EKIGA_MACRO_DEFAULT_STRING" with "!define"

; Startup Checks
!define INSTALLER_IS_RUNNING			"��������� ��������� ��� ��������."
!define EKIGA_IS_RUNNING				"������������ Ekiga ��� �������. ���������� ����� �� ���� � ��������� ��������� ��������� ��� ���."
!define GTK_INSTALLER_NEEDED			"�� ������ ������� ����������� ���� ������� ���������� ���������� GTK+ runtime.$\r����������, ���������� GTK+ runtime ������ v${GTK_VERSION} ��� �����."

; License Page
!define EKIGA_LICENSE_BUTTON			"����� >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) ����������� �� ����� �������������� GNU General Public License (GPL). �������������� ��� ����� �������� ���������� ������ ��� �������. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"������������ Ekiga (�����������)"
!define GTK_SECTION_TITLE			"���������� GTK+ Runtime (�����������)"
!define GTK_THEMES_SECTION_TITLE			"���� GTK+"
!define GTK_NOTHEME_SECTION_TITLE		"��� ���"
!define GTK_WIMP_SECTION_TITLE			"���� Wimp"
!define GTK_BLUECURVE_SECTION_TITLE		"���� Bluecurve"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE		"���� Light House Blue"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"������"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"������� ����"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"���� ����"
!define EKIGA_SECTION_DESCRIPTION			"�������� ���������� � ����� Ekiga"
!define GTK_SECTION_DESCRIPTION			"�����-������������� ���������� ����������������� ����������, ������������ Ekiga"
!define GTK_THEMES_SECTION_DESCRIPTION		"���� GTK+ ����������� ��� ��������� �������� ���� ����������, ������������ ���������� GTK+."
!define GTK_NO_THEME_DESC			"�� ������������� ���� GTK+"
!define GTK_WIMP_THEME_DESC			"GTK-Wimp (Windows impersonator) ��� ���� GTK, ������� ��������� ��������� ������� ��� ������� ���������� Windows."
!define GTK_BLUECURVE_THEME_DESC			"���� Bluecurve."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"���� Lighthouseblue."
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"���������� Ekiga ��� ������ Windows"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"������ ��� ������� Ekiga"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"������� ����� ��� Ekiga �� ������� �����"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"������� ��� Ekiga ����� � ���� ����"

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"���������� ���������� ������ ��������� GTK+. ���������� ����������?$\r��������: ���������� ����� ���� ���������� ��� ������� Ekiga."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"�������� ���-���� Ekiga ��� Windows"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (������ ��������)"
!define EKIGA_RUN_AT_STARTUP			"��������� Ekiga ��� ������ Windows"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"������ �������� ��� ������������� ������ Ekiga. ��������� ����� ������ ����� ���������� ��� �������� ������������ ������."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"������ ��������� ��������� GTK+. ������������ � ���������� ���������?"
!define GTK_BAD_INSTALL_PATH			"�� ������ �������� ������ ��� ������� ����� �� ���������� ���� ����."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"� ��� ����������� ����������� ���������� ��� ��������� ��� GTK+."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"��������� �������� �� ������ ����� ������ ������� ��� Ekiga.$\r��������� ����� ������������ Ekiga ��� ���������� �� ��� ������� ������ ������� ������������."
!define un.EKIGA_UNINSTALL_ERROR_2		"� ��� ����������� ����������� ���������� ��� �������� ����� ����������."
