//#include "include/gui.h"

#include <SDL.h>

#ifdef __cplusplus
extern "C"{
#endif
	void		Show_Main(void);
	EG_BOOL 	InitializeBeebEmGUI(SDL_Surface *screen_ptr);
	void	 	DestroyBeebEmGUI();

	int UpdateGUIOption(int windows_menu_id, int is_selected);
	int GetGUIOption(int windows_menu_id);
	int SetGUIOptionCaption(int windows_menu_id, const char *str);

	int Open_GTK_File_Selector(char *filename_ptr);
	int Save_GTK_File_Selector(char *filename_ptr);

	void Update_FDC_Buttons(void);
	void Update_Resolution_Buttons(void);

	void SetNameForDisc(int drive, char *name_ptr);
	void SetFullScreenTickbox(int state);

	void ClearWindowsBackgroundCacheAndResetSurface(void);
#ifdef __cplusplus
}
#endif
