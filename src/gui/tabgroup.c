/* START OF tabgroup.c ----------------------------------------------------------
 *
 *	A Tab Group widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/functions.h>
#include <gui/log.h>

#include <gui/sdl.h>

#include <gui/tabgroup.h>
#include <gui/tabgroup_private.h>

#include <gui/tabpage.h>
#include <gui/tabpage_private.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <gui/widget_shared.h>

#include <SDL.h>


/* Callbacks:
 */

static void Callback_Destroy(EG_Widget *widget_ptr)
{
        EG_TabGroup *tabgroup_ptr;
	int i;        

	/* If widget is actually just a NULL, then do nothing but log error.
	 */
        if (widget_ptr==NULL){
                EG_Log(EG_LOG_ERROR, dL"Tried to Destroy EG_TabGroup with"
		 " NULL widget pointer.", dR);
                return;
        }

	/* Child EG_TabPages become unlinked to a EG_TabGroup when the 
	 * tab group is deleted.
	 */
	EG_TABGROUP_GET_STRUCT_PTR_VOID(widget_ptr, tabgroup_ptr);

	for(i=0; i<tabgroup_ptr->count; i++){
		EG_TabPage_SetTabGroup(tabgroup_ptr->page_widget_ptr[i], NULL);
        }

	/* Free EG_Widget payload (the instance of this widget class).
	 */
        if (tabgroup_ptr != NULL) EG_Free(tabgroup_ptr);
                
        /* Free EG_Widget struct.
         */
        EG_Widget_Free(widget_ptr);
}

/* Paints the box the tab pages are drawn in.
 */
static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
        EG_TabGroup *tabgroup_ptr;
        EG_Window *window_ptr;
	SDL_Rect loc;
        SDL_Color color;
        SDL_Surface *surface_ptr;


	/* If widget falls outside area we've been asked to paint, returns
	 * success.  Otherwise, function loads all the variables we'll need to
	 * render the widget and returns false.
	 */
	if ( EG_Shared_GetRenderingDetails(widget_ptr, area,
	 (void*) &tabgroup_ptr, &window_ptr, &surface_ptr, &color, &loc) != EG_TRUE )
		return(EG_TRUE);


#ifdef EG_DEBUG
	printf("SOMEONE CALLED PAINT FOR '%s' [repaint area (window)="
	 "{%d, %d, %d, %d}:widget area (SDL_Surface)={%d, %d, %d, %d}]\n"
	, EG_Widget_GetName(widget_ptr), area.x, area.y, area.w, area.h
	, loc.x, loc.y, loc.w, loc.h);
#endif


	/* Paint the widget:
	 */

//	/* Don't do anything if not visible
//	 */
//	if (EG_Widget_GetVisibleToggle(widget_ptr) != EG_TRUE)
//		return(EG_TRUE);

	/* Subtract tab page button area from tab group area.
	 */
	loc.y += EG_TabGroup_PageButtonHeight;
	loc.h -= EG_TabGroup_PageButtonHeight;

	/* Dull down the button a bit when disabled.
	 */
	if (EG_Widget_IsEnabled(widget_ptr) != EG_TRUE){
		color.r *= 0.7; color.g *= 0.7; color.b *= 0.7;
	}

	/* Draw the tabbers box (the adjustments are for the hardwired insets.
	 */

	// [TODO] Change this, it'll leave a line around the box. Look into the border flicker thing.

	loc.x+=1; loc.y+=1; loc.w-=2; loc.h-=2;
	EG_Draw_Box(surface_ptr, &loc, &color);
	loc.x-=1; loc.y-=1; loc.w+=2; loc.h+=2;
	EG_Draw_Border(surface_ptr, &loc, &color, EG_Draw_Border_BoxHigh);

	// [TODO] Shouldn't we render our selected page group child too?

        return(EG_TRUE);
}

static EG_BOOL Callback_Visible(EG_Widget *widget_ptr, EG_BOOL is_visible)
{
	EG_TabGroup *tabgroup_ptr;
	int i;

	/* Set the state of the tab group widget (us).
	 */
	if ( EG_Widget_SetVisibleToggle(widget_ptr, is_visible) != EG_TRUE){
		EG_Log( EG_LOG_ERROR, dL"Changing widget '%s' visible state"
		 " failed (returned false).", dR
		 , EG_Widget_GetName(widget_ptr) );

		return(EG_FALSE);
	}

	/* Repaint the widget in it's new state. If not attached to a window
	 * yet, then don't bother to render.
	 */
	if ( EG_Widget_GetWindow(widget_ptr) != NULL )
		(void) EG_Widget_RepaintLot(widget_ptr);

	/* Change state of child tab pages too by passes this state change
	 * to the child widgets.
	 */
        EG_TABGROUP_GET_STRUCT_PTR(widget_ptr, tabgroup_ptr, EG_FALSE);

        for(i=0; i<tabgroup_ptr->count; i++){
		(void) EG_Widget_CallVisible(tabgroup_ptr->page_widget_ptr[i]
		 , is_visible);
        }

        return(EG_TRUE);
}

static EG_BOOL Callback_Enabled(EG_Widget *widget_ptr, EG_BOOL is_enabled)
{
	EG_TabGroup *tabgroup_ptr;
	int i;

	/* Set the state of the tab group widget (us).
	 */
	if ( EG_Widget_SetEnabledToggle(widget_ptr, is_enabled) != EG_TRUE){
		EG_Log( EG_LOG_ERROR, dL"Changing widget '%s' visible state"
		 " failed (returned false).", dR
		 , EG_Widget_GetName(widget_ptr) );

		return(EG_FALSE);
	}

        /* Repaint the widget in it's new state. If not attached to a window
         * yet, then don't bother to render.
         */
        if ( EG_Widget_GetWindow(widget_ptr) != NULL )
                (void) EG_Widget_RepaintLot(widget_ptr);

	/* Change state of child tab pages too by passes this state change
	 * to the child widgets.
	 */
	EG_TABGROUP_GET_STRUCT_PTR(widget_ptr, tabgroup_ptr, EG_FALSE);

	for(i=0; i<tabgroup_ptr->count; i++){
		(void) EG_Widget_CallEnabled(tabgroup_ptr->page_widget_ptr[i]
		 , is_enabled);
	}

        return(EG_TRUE);
}

///* EG_TabGroups cannot be stopped or started as they don't support nesting. So
// * they should never need to be stopped or started.
// */
static EG_BOOL Callback_Stopped(EG_Widget *widget_ptr, EG_BOOL is_stopped)
{
//	/* Loose compiler warning.
//	 */
//	EG_BOOL tmp_is_stopped;
//	tmp_is_stopped = is_stopped;
//
//	EG_Widget_SetStoppedToggle(widget_ptr, EG_FALSE);	
//
//	EG_Log(EG_LOG_ERROR, dL"EG_TabGroup '%s' cannot be stopped, nesting tab"
//	 " groups is not supported.", dR, EG_Widget_GetName(widget_ptr));	
//
//	return(EG_FALSE);


        EG_TabGroup *tabgroup_ptr;
        int i;

        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
        EG_TABGROUP_GET_STRUCT_PTR(widget_ptr, tabgroup_ptr, EG_FALSE);

	printf("called stop callback.........................................................................\n");

        /* Always honor state change for tab group widget itself
         */
	EG_Widget_SetStoppedToggle(widget_ptr, is_stopped);

        /* Pass state change on to child widgets.
         */

//	printf("I have %d children\n", tabgroup_ptr->count);

	printf("GOT TO LOOP\n");

        for(i=0; i<tabgroup_ptr->count; i++){
                if (is_stopped == EG_TRUE)
                        EG_Window_StopWidget(tabgroup_ptr->page_widget_ptr[i]);
                else
                        EG_Window_StartWidget(tabgroup_ptr->page_widget_ptr[i]);

//		if (page_is_shown == EG_TRUE && is_stopped == EG_FALSE)
//			EG_Widget_SetStoppedToggle(tabgroup_ptr->page_widget_ptr[i], is_stopped);
        }

        return(EG_TRUE);
}

static void Callback_Attach(EG_Widget *widget_ptr, EG_StringHash attach_to_type
 , void *attach_to_ptr, EG_BOOL attached)
{
	EG_StringHash tmp_attach_to_type;

	EG_TabGroup *tabgroup_ptr;
	EG_Window *window_ptr;
	int i;

	printf("attached = %d\n", attached);

	/* Loose compiler warning.
	 */
	tmp_attach_to_type = attach_to_type;

	/* We only need to add the pages to the window when attaching, we do
	 * nothing when detatching.
	 */
	if (attached != EG_TRUE)
		return;

	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);

	/* Get parent window.
	 */
	window_ptr = attach_to_ptr;
	CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);

	EG_TABGROUP_GET_STRUCT_PTR_VOID(widget_ptr, tabgroup_ptr);

	/* Add tab pages to window.
	 */
	for(i=0; i<tabgroup_ptr->count; i++){
		EG_Window_AddWidget(window_ptr, tabgroup_ptr->page_widget_ptr[i]);
	}

//	/* Now attach each pages child widgets.
//	 */
//
//	// [TODO] Is now in tabpage.c, leave this here for now.
//	for(i=0; i<tabgroup_ptr->count; i++){
//		EG_TabPage_AddChildWidgetsToWindow(tabgroup_ptr->page_widget_ptr[i]);
//	}

	/* If we have attached at least one tab page, make that the active one.
	 */
	if (tabgroup_ptr->count>0){

		/* Make first tab page the selected one.
		 */
		if (EG_TabGroup_SelectPageByIndex(widget_ptr,1)
		 == EG_FALSE)
			EG_Log(EG_LOG_ERROR, dL"Failed to make EG_TabPage"
			 " active.", dR);
	}
}


/* Private functions:
 */

static void InitializePayload(EG_TabGroup *tabgroup_ptr)
{
	int i;

	for(i=0; i<MAX_TABGROUP_PAGES; i++)
		tabgroup_ptr->page_widget_ptr[i] = NULL;	

	tabgroup_ptr->count = 0;
	tabgroup_ptr->selected = -1;
	tabgroup_ptr->tab_bar_width = 4;
}

static void InitializeWidget(EG_Widget *widget_ptr, SDL_Color color
 , SDL_Rect dimension)
{
	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);

	/* Initialize callbacks:
	 */
      	(void) EG_Widget_SetCallback_Destroy(widget_ptr, Callback_Destroy);
        (void) EG_Widget_SetCallback_Paint(widget_ptr, Callback_Paint);

        (void) EG_Widget_SetCallback_SDL_Event(widget_ptr
	 , EG_Callback_Generic_SDL_Event);

        (void) EG_Widget_SetCallback_Visible(widget_ptr, Callback_Visible);
        (void) EG_Widget_SetCallback_Stopped(widget_ptr, Callback_Stopped);
        (void) EG_Widget_SetCallback_Enabled(widget_ptr, Callback_Enabled);

        (void) EG_Widget_SetCallback_GotFocus(widget_ptr
	 , EG_Callback_Generic_GotFocus_NoSupport);

        (void) EG_Widget_SetCallback_LostFocus(widget_ptr
	 , EG_Callback_Generic_LostFocus_NoSupport);

        (void) EG_Widget_SetCallback_Attach(widget_ptr, Callback_Attach);

        /* Initialize state:
         */
        (void) EG_Widget_SetDimension(widget_ptr, dimension);
        (void) EG_Widget_SetBackgroundColor(widget_ptr, color);
	(void) EG_Widget_SetCanGetFocusToggle(widget_ptr, EG_FALSE);
}

/* Public functions:
 */

EG_Widget* EG_TabGroup_Create(const char *name_ptr, SDL_Color color
 , SDL_Rect dimension)
{
        EG_TabGroup *tabgroup_ptr;
        EG_Widget *widget_ptr;

	SHARED__ALLOC_PAYLOAD_STRUCT(tabgroup_ptr, EG_TabGroup
	 , "Unable to malloc EG_TabGroup struct");

	SHARED__CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr
	 , EG_Widget_Type_TabGroup, tabgroup_ptr);

	SHARED__ATTACH_PAYLOAD_TO_WIDGET(widget_ptr, tabgroup_ptr);

	InitializePayload(tabgroup_ptr);
        InitializeWidget(widget_ptr, color, dimension);

        return(widget_ptr);
}

EG_BOOL EG_TabGroup_AddPage(EG_Widget *group_widget_ptr
 , EG_Widget *page_widget_ptr)
{
	EG_TabGroup *tabgroup_ptr;
	const char *caption_ptr;
	SDL_Rect page_dimension, group_dimension;

	CHECK_EG_WIDGET_IS_VALID(group_widget_ptr, EG_FALSE);

	EG_TABGROUP_GET_STRUCT_PTR(group_widget_ptr, tabgroup_ptr, EG_FALSE);

	if (EG_Widget_GetType(page_widget_ptr) != EG_Widget_Type_TabPage){
		EG_Log(EG_LOG_ERROR, dL"Can't attach child to EG_TabGroup: "
		 "It's not an EG_TabPage.", dR);
		return(EG_FALSE);
	}

	if (tabgroup_ptr->count > MAX_TABGROUP_PAGES){
		EG_Log(EG_LOG_ERROR, dL"Can't add EG_TabPage to EG_TabGroup: "
		 "Would exceed max page count.", dR);
		return(EG_FALSE);
	}

	/* Attach EG_TabPage to this EG_TabGroup.
	 */
	tabgroup_ptr->page_widget_ptr[tabgroup_ptr->count++] = page_widget_ptr;

	/* Set this new pages dimension.
	 */
	group_dimension =EG_Widget_GetDimension(group_widget_ptr);

#define INSET 	12
#define SPACER 	2

	/* Calc. the absolute size and position of the new tab page button.
	 */
	if ( (caption_ptr=EG_TabPage_GetCaption(page_widget_ptr)) == NULL){
		/* If the tab page has no caption, then we will add the index
		 * number, so we only need two character widths.
	 	 */
		page_dimension.w = 10*2 + SPACER; //4;
	}else{
		/* Calculate the width of the tab pages selection button.
		 * (10= width of a character, 4= are the left and right insets).
		 */
		page_dimension.w = strlen(caption_ptr)*10 + INSET + SPACER; //4;
	}
	page_dimension.x = tabgroup_ptr->tab_bar_width + group_dimension.x;
	page_dimension.y = group_dimension.y;
	page_dimension.h = EG_TabGroup_PageButtonHeight;

	/* Add the width of this new tab page to the tab group button bar width.
	 * (+4 = spacer).
	 */
	tabgroup_ptr->tab_bar_width += page_dimension.w + SPACER; //4;

	/* Set the tab page dimension, log an error on failure, but continue.
	 */
	if (EG_Widget_SetDimension(page_widget_ptr, page_dimension) != EG_TRUE)
		EG_Log(EG_LOG_ERROR,dL"Could not set EG_TabPage dimension.",dR);

	/* Associate the tab page with this tab group.
	 */
	if (EG_TabPage_SetTabGroup(page_widget_ptr,group_widget_ptr) != EG_TRUE)
		EG_Log(EG_LOG_ERROR,dL"Could not associcate EG_TabPage with"
		 " EG_TabGroup.\n", dR);

	/* Set the tab page background color.
	 */
	if (EG_Widget_SetBackgroundColor(page_widget_ptr
	 , EG_Widget_GetBackgroundColor(group_widget_ptr)) != EG_TRUE)
		EG_Log(EG_LOG_ERROR,dL"Could not set EG_TabPage background"
		 " color.\n", dR);

	return(EG_TRUE);
}

void EG_TabGroup_RemovePage(EG_Widget *group_widget_ptr
 , EG_Widget *page_widget_ptr)
{
	EG_TabGroup *tabgroup_ptr;
	int i;

	CHECK_EG_WIDGET_IS_VALID_VOID(group_widget_ptr);

	EG_TABGROUP_GET_STRUCT_PTR_VOID(group_widget_ptr, tabgroup_ptr);

	/* Find page widget.
	 */
	for(i=0; i<tabgroup_ptr->count; i++)
		if (EG_Widget_GetID(page_widget_ptr) == EG_Widget_GetID(
		 tabgroup_ptr->page_widget_ptr[i]))
			break;

	/* If not found, log error and exit.
	 */	
	if (i>=tabgroup_ptr->count){
		EG_Log(EG_LOG_ERROR, dL"Tried to delete widget %s from"
		 " EG_TabGroup, when not memeber of group.", dR);
		return;
	}

	// [TODO] When we remove this we need to move all the tab page buttons too.

	// [TODO] If one of the pages widgets has focus, remove focus altogether.  
	// [TODO] Remove the page.
	// [TODO] Set tabgroups page to page after removed page, if we deleted the last page, set tabgroups page to the previous page.  If this was the only page, render only the tabgroup (will paint background).
	// [TODO] If a widget on the page had focus before we removed it, set focus to the next page.  If we removed the last page, set focus to the previous page, if we deleted the only page, ask window to movefocus.  (will set to first widget).

	// [TODO] Delete Page (but leave child widgets intact).
}

void EG_TabGroup_DeletePage(EG_Widget *group_widget_ptr
 , EG_Widget *page_widget_ptr)
{
	// [TODO] Same as remove page, but deletes widgets too.
}

EG_BOOL EG_TabGroup_SelectPage(EG_Widget *page_widget_ptr)
{
	EG_Widget *next_page_widget_ptr; 
	EG_Widget *group_widget_ptr;
	EG_TabGroup *tabgroup_ptr;
	EG_TabPage *tabpage_ptr;
	int i;

	CHECK_EG_WIDGET_IS_VALID(page_widget_ptr, EG_FALSE);
	
	/* Get tab page struct.
	 */
	EG_TABPAGE_GET_STRUCT_PTR(page_widget_ptr, tabpage_ptr, EG_FALSE);

	/* Get tab group, if NULL, then not associated with a group yet.
	 */
	if ( (group_widget_ptr=EG_TabPage_GetTabGroup(page_widget_ptr))== NULL){
		return(EG_FALSE);
	}

	EG_TABGROUP_GET_STRUCT_PTR(group_widget_ptr, tabgroup_ptr, EG_FALSE);

	/* Varify tab page is on the same window as the tab group.
	 */
	// [TODO]

	/* Get this tab pages index.
	 */
	for(i=0; i<tabgroup_ptr->count; i++){
		next_page_widget_ptr = tabgroup_ptr->page_widget_ptr[i];

		if (next_page_widget_ptr != NULL){
			if (EG_Widget_GetID(next_page_widget_ptr) ==
			 EG_Widget_GetID(page_widget_ptr)){
				return( EG_TabGroup_SelectPageByIndex(
				 EG_TabPage_GetTabGroup(page_widget_ptr),i+1) );
			}
		}
	}
	return(EG_FALSE);
}

/* Will change the tab groups page to the specified numerical page index.
 * The page arg is indexed from 1, so if you had 3 pages on our page group,
 * they would be indice: 1, 2 and 3.
 */
EG_BOOL EG_TabGroup_SelectPageByIndex(EG_Widget *widget_ptr, int page)
{
	EG_Widget *old_selected_tab_page_widget = NULL;
	EG_TabGroup *tabgroup_ptr;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);

	EG_TABGROUP_GET_STRUCT_PTR(widget_ptr, tabgroup_ptr, EG_FALSE);

	/* If page to select is already selected, do nothing and return success.
	 * (Remember 'page' is indexed from 1, but 'selected' is indexed from 0,
	 * so we +1 to 'selected').
	 */
	if (page == tabgroup_ptr->selected+1)
		return(EG_TRUE);

	/* Is the requested page to select selectable?
	 */
	if (page<1 || page>tabgroup_ptr->count){
		EG_Log(EG_LOG_ERROR, dL"Invalid EG_TabGroup page selected.",dR);
		return(EG_FALSE);
	}

	/* [TODO] Clear the windows focus otherwise repainting removes tail of button thats just lost focus..  This should be fixed..
	 */
	EG_Window_ClearFocus(EG_Widget_GetWindow(widget_ptr));

	/* Stop current page.
	 */
	if (tabgroup_ptr->selected >= 0){
		/* Remember which page has been de-selected so we can
		 * update it's tab page button.
		 */
		old_selected_tab_page_widget = 
	 	 tabgroup_ptr->page_widget_ptr[tabgroup_ptr->selected];

		if(EG_TabPage_Hide(
		 tabgroup_ptr->page_widget_ptr[tabgroup_ptr->selected])
		 != EG_TRUE){
			EG_Log(EG_LOG_ERROR, dL"Tab page failed to hide."
			 , dR);
			return(EG_FALSE);
		}
	}

	/* Repaint tab group area (thus deleting the old widgets it contains).
	 */
	EG_Widget_RepaintLot(widget_ptr);

	/* Start new page.
	 */
	if (EG_TabPage_Show(tabgroup_ptr->page_widget_ptr[page-1]) != EG_TRUE)
		EG_Log(EG_LOG_ERROR, dL"Tab page failed to show, but assuming"
		 " it did.", dR);
	else if ( EG_TabPage_RePaintChildren(
	 tabgroup_ptr->page_widget_ptr[page-1]) !=EG_TRUE)
		EG_Log(EG_LOG_ERROR, dL"Tab page failed to repaint, but"
		 " assuming it did.", dR);

	/* Remember: 'page' is indexed from 1, and 'selected' is indexed from 0.
	 */
	tabgroup_ptr->selected = page-1;

	/* Repaint newly selected tab page button, and the previous one thats
	 * been de-selected.
	 */
	EG_Widget_RepaintLot( tabgroup_ptr->page_widget_ptr
	 [tabgroup_ptr->selected] );

	if (old_selected_tab_page_widget != NULL)
		EG_Widget_RepaintLot( old_selected_tab_page_widget );

	return(EG_TRUE);
}

EG_Widget* EG_TabGroup_GetSelectedPageWidget(EG_Widget *widget_ptr)
{

        EG_TabGroup *tabgroup_ptr;

        CHECK_EG_WIDGET_IS_VALID(widget_ptr, NULL);
	EG_TABGROUP_GET_STRUCT_PTR(widget_ptr, tabgroup_ptr, NULL);

	if (tabgroup_ptr->selected<0 || tabgroup_ptr->selected >= 
	 MAX_TABGROUP_PAGES)
		return(NULL);
	else
		return(tabgroup_ptr->page_widget_ptr[tabgroup_ptr->selected]);
}



