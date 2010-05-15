#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include "log.h"
#include "windows.h"
#include "beebem_pages.h"
#include "fake_registry.h"

#include <gui.h>

/* Fake windows stuff:
 */

/* Fake windows MessageBox
 */
int MessageBox(HWND hwnd, const char *message_p, const char *title_p, int type)
{
	int ret = 1, selected, icon_type=0;
	HWND tmp_hwnd;

	tmp_hwnd = hwnd;

	// [TODO] Add support for default button, BeebEm doesn't use it anyway.
	selected = 1;


	switch (type & 0xf0){
	
	case MB_ICONHAND:
		icon_type = EG_MESSAGEBOX_STOP;
		break;
	case MB_ICONQUESTION:
		icon_type = EG_MESSAGEBOX_QUESTION;
		break;
	case MB_ICONEXCLAMATION:
		icon_type = EG_MESSAGEBOX_STOP;
		break;
	case MB_ICONASTERISK:
		icon_type = EG_MESSAGEBOX_INFORMATION;
		break;
	case MB_USERICON:
		icon_type = EG_MESSAGEBOX_STOP;
		break;
	default:
		icon_type = EG_MESSAGEBOX_INFORMATION;
		break;
	}


	switch (type & 0xf){

	case MB_OKCANCEL:
		if (EG_MessageBox(screen_ptr, icon_type, title_p, message_p, "OK", "CANCEL", NULL, NULL, selected) == 1)
			ret = IDOK;
		else
			ret = IDCANCEL;
		break;
	case MB_ABORTRETRYIGNORE:
		ret = EG_MessageBox(screen_ptr, icon_type, title_p, message_p, "Abort", "Retry", "Ignore", NULL, selected);
		switch (ret){
			case 2: ret = IDRETRY; break;
			case 3: ret = IDIGNORE; break;
			case 1: default: ret = IDABORT; break;
		}
		break;
	case MB_YESNOCANCEL:
		ret = EG_MessageBox(screen_ptr, icon_type, title_p, message_p, "Yes", "No", "Cancel", NULL, selected);
		switch (ret){
			case 2: ret = IDNO; break;
			case 3: ret = IDCANCEL; break;
			case 1: default: ret = IDYES; break;
		}
		break;
	case MB_YESNO:
		if (EG_MessageBox(screen_ptr, icon_type, title_p, message_p, "Yes", "No", NULL, NULL, selected) == 1)
			ret = IDYES;
		else
			ret = IDNO;
		break;
	case MB_RETRYCANCEL:
		if (EG_MessageBox(screen_ptr, icon_type, title_p, message_p, "Retry", "Cancel", NULL, NULL, selected) == 1)
			ret = IDRETRY;
		else
			ret = IDCANCEL;
		break;

	case MB_OK:
	default:
		(void) EG_MessageBox(screen_ptr, icon_type, title_p, message_p, "OK", NULL, NULL, NULL, 1);
		ret = IDOK;
		break;
	}

	return ret;
}

void SetWindowText(HWND hwnd, const char *title_p)
{
	SetWindowTitle( (char*) title_p);
}

void Sleep(DWORD ticks)
{
	SaferSleep( (unsigned int) ticks);
}

DWORD GetTickCount(void)
{
	return(SDL_GetTicks());
}

BOOL ModifyMenu(HMENU hMnu, UINT uPosition, UINT uFlags, PTR uIDNewItem, LPCTSTR lpNewItem)
{
	int v;

	// Only supports changing the title.
	//SetGUIOptionCaption(int windows_menu_id, const char *str)

	v = SetGUIOptionCaption( (int) uPosition, (const char*) lpNewItem);

/*
Return Value

    If the function succeeds, the return value is nonzero.

    If the function fails, the return value is zero. To get extended error information, call GetLastError.
*/

	if (v==EG_TRUE) return 1;
	else return 0;
}

UINT GetMenuState(HMENU hMenu, UINT uId, UINT uFlags)
{
	int v;

//	printf("GetMenuState\n");

	v = GetGUIOption(uId);
	
	if (v == EG_TRUE) v = MF_CHECKED;
	if (v == EG_FALSE) v = MF_UNCHECKED;

	return(v);

/*
Return Value

    If the specified item does not exist, the return value is -1.

    If the menu item opens a submenu, the low-order byte of the return value contains the menu flags associated with the item, and the high-order byte contains the number of items in the submenu opened by the item.

    Otherwise, the return value is a mask (Bitwise OR) of the menu flags. Following are the menu flags associated with the menu item.

    MF_CHECKED	A check mark is placed next to the item (for drop-down menus, submenus, and shortcut menus only).
    MF_DISABLED	The item is disabled.
    MF_GRAYED	The item is disabled and grayed.
    MF_HILITE	The item is highlighted.
    MF_MENUBARBREAK	This is the same as the MF_MENUBREAK flag, except for drop-down menus, submenus, and shortcut menus, where the new column is separated from the old column by a vertical line.
    MF_MENUBREAK	The item is placed on a new line (for menu bars) or in a new column (for drop-down menus, submenus, and shortcut menus) without separating columns.
    MF_OWNERDRAW	The item is owner-drawn.
    MF_POPUP	Menu item is a submenu.
    MF_SEPARATOR	There is a horizontal dividing line (for drop-down menus, submenus, and shortcut menus only).



Remarks

    Note  The GetMenuState function has been superseded by the GetMenuItemInfo. You can still use GetMenuState, however, if you do not need any of the extended features of GetMenuItemInfo.

    In addition, it is possible to test an item for a flag value of MF_ENABLED, MF_STRING, MF_UNCHECKED, or MF_UNHILITE. However, since these values equate to zero you must use an expression to test for them.
    Flag 	Expression to test for the flag
    MF_ENABLED	! (Flag&(MF_DISABLED | MF_GRAYED))
    MF_STRING	! (Flag&(MF_BITMAP | MF_OWNERDRAW))
    MF_UNCHECKED	! (Flag&MF_CHECKED)
    MF_UNHILITE	! (Flag&HILITE)


*/
}

DWORD CheckMenuItem(HMENU hmenu, UINT uIDCheckItem, UINT uCheck)
{
	//printf("Asked to set %d to %d\n", uIDCheckItem, uCheck);

	if (uCheck == MF_CHECKED)
		return( UpdateGUIOption(uIDCheckItem, 1) );
	else
		return( UpdateGUIOption(uIDCheckItem, 0) );
}

BOOL MoveFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, DWORD dwFlags)
{
	printf("MoveFileEx\n");
	return(FALSE);
}

BOOL EnableMenuItem(HMENU hMenu,UINT uIDEnableItem,UINT uEnable)
{
	printf("EnableMenuItem\n");
	return(TRUE);
}


/* In true cum$oft fashion this function seems to return an unsigned int32
 * but default from a signed int32 if it cannot be found...
 */
UINT GetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName,
 INT nDefault, LPCTSTR lpFileName)
{
	long val;

	if (GetFakeRegistryItem_Long(lpKeyName, &val) == TRUE)
		return (UINT) val;
	else
		return (UINT) nDefault;
}



//GetPrivateProfileString(CFG_VIEW_SECTION, CFG_VIEW_SHOW_FPS, "1",
//--                    CfgValue, sizeof(CfgValue), CFG_FILE_NAME);


DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault,
 LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	// lpKeyName = key
	// lpReturnedString = return value
	// lpDefault = default value
/*
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/getprivateprofilestring.asp

lpDefault
    [in] A default string. If the lpKeyName key cannot be found in the initialization file, GetPrivateProfileString copies the default string to the lpReturnedString buffer. If this parameter is NULL, the default is an empty string, "".

    Avoid specifying a default string with trailing blank characters. The function inserts a null character in the lpReturnedString buffer to strip any trailing blanks.

        Windows Me/98/95:  Although lpDefault is declared as a constant parameter, the system strips any trailing blanks by inserting a null character into the lpDefault string before copying it to the lpReturnedString buffer.
*/

	char *fake_reg_string_ptr;
	char *callers_string_ptr = (char*) lpReturnedString;
	BOOL ret;

	ret = GetFakeRegistryItem_String( (char*) lpKeyName, (char**) &fake_reg_string_ptr);
	if (ret == TRUE){
		strcpy(callers_string_ptr, fake_reg_string_ptr);
	}else{
		if (lpDefault == NULL){
			*callers_string_ptr='\0';
		}else{
			strcpy(callers_string_ptr, lpDefault);
		}
	}

	//printf("[%s]\n", callers_string_ptr);

	return(strlen(callers_string_ptr));
}
