/* START OF radiogroup.c -------------------------------------------------------
 *
 *	A Radio Button Group widget.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */


#if HAVE_CONFIG_H
#       include <config.h>
#endif


#include <gui/log.h>

#include <gui/functions.h>

#include <gui/radiogroup.h>
#include <gui/radiogroup_private.h>

#include <gui/radiobutton.h>
#include <gui/radiobutton_private.h>

#include <gui/togglebutton.h>
#include <gui/togglebutton_private.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <gui/widget_shared.h>

#include <gui/sdl.h>

#include <SDL.h>


/* Callbacks:
 */

/* Will unlink all child Radio Buttons from this Radio Group on deletion.  Does
 * not delete the children.
 */
static void Callback_Destroy(EG_Widget *widget_ptr)
{
	EG_RadioGroup *radiogroup_ptr;
	EG_Widget *tmp_widget_ptr;
	int i;

	/* If widget is actually just a NULL, then do nothing but log error.
	 */
	if (widget_ptr==NULL){
		EG_Log(EG_LOG_ERROR, dL"Tried to Destroy Radio Group with"
		 " widget_ptr=NULL", dR);
              return;
	}

	/* If widget is not a RadioGroup, then don't delete children, but log
	 * as error. (A serious one too!)
	 */
	if (EG_Widget_GetType(widget_ptr) != EG_Widget_Type_RadioGroup){
		EG_Log(EG_LOG_ERROR, dL"EG_RadioGroup Destroy callback called"
		 " for none EG_RadioGroup widget.", dR);
	}else{
		/* Unlink children from this group (they become unassociated
		 * Radio Buttons again).
		 */
		if ( (radiogroup_ptr= (EG_RadioGroup*) EG_Widget_GetPayload(widget_ptr)) != NULL){
			for(i=0; i<radiogroup_ptr->count; i++){
				tmp_widget_ptr =
				 radiogroup_ptr->button_widget_ptr[i];

				switch ( EG_Widget_GetType(tmp_widget_ptr) ){
				case EG_Widget_Type_RadioButton:
					EG_RadioButton_SetGroupParent(tmp_widget_ptr, NULL);
					break;

				case EG_Widget_Type_ToggleButton:
					EG_ToggleButton_SetGroupParent(tmp_widget_ptr, NULL);
					break;

				default:
					EG_Log( EG_LOG_ERROR, dL"Widget %s is of incorrect type."
				 	 , dR, EG_Widget_GetName(tmp_widget_ptr) );
					break;
				}

//				if (EG_Widget_GetType(tmp_widget_ptr) == EG_Widget_Type_RadioButton)
//					EG_RadioButton_SetGroupParent(tmp_widget_ptr, NULL);
//
//				if (EG_Widget_GetType(tmp_widget_ptr) == EG_Widget_Type_ToggleButton)
//					EG_ToggleButton_SetGroupParent(tmp_widget_ptr, NULL);
                	}
        	}
		
		/* Free the EG_RadioGroup struct.
		 */
		EG_Free(radiogroup_ptr);
	}
  
	/* Free EG_Widget struct.
	 */
	EG_Widget_Free(widget_ptr);
}

/* Simply repaints all the children. (Maybe this is a bad idea.)
 */
static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
        EG_RadioGroup *radiogroup_ptr;
	SDL_Rect *tmp_area;
	int i;

	/* Loose compiler warning.
	 */
	tmp_area = &area;

	/* If asked to paint, paint the children.
	 */
        EG_RADIOGROUP_GET_STRUCT_PTR(widget_ptr, radiogroup_ptr, EG_FALSE);

	/* Repaint children. Repaint the whole area of the widget as the
	 * Radio Group knows  nothing about the size or location of it's
	 * children.
	 */
	for(i=0; i<radiogroup_ptr->count; i++)
		(void) EG_Widget_RepaintLot(
		 radiogroup_ptr->button_widget_ptr[i]);

	/* Return success a painting.
	 */
	return(EG_TRUE);
}

/* If a Radio Groups visibility state chages, then reflect that change to
 * all it's child widgets.
 *
 * Warning: Making a Radio Group visible or invisible will loose any visiblity
 * setting a child may already have.  Consider changing the visibility state
 * via the Radio Group as a global over-ride for all child widgets.
 */
static EG_BOOL Callback_Visible(EG_Widget *widget_ptr, EG_BOOL is_visible)
{
	int i;
	EG_RadioGroup *radiogroup_ptr;
	EG_Widget *tmp_widget_ptr;

	// [TODO] THIS ONE SHOULD BE OK FOR THE CHANGE, EG_Shared_UpdateVisibleState DOES ALL THE REPAINTING/FOCUS WORK.

        CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	EG_RADIOGROUP_GET_STRUCT_PTR(widget_ptr, radiogroup_ptr, EG_FALSE);

        /* Change state of Radio Group widget.
         */
        (void) EG_Shared_UpdateVisibleState(widget_ptr, is_visible);

        /* Reflect change with children too.
         */
        for(i=0; i<radiogroup_ptr->count; i++){
                tmp_widget_ptr = radiogroup_ptr->button_widget_ptr[i];

                (void) EG_Shared_UpdateVisibleState(tmp_widget_ptr, is_visible);
        }

	return(EG_TRUE);
}


/* If a Radio Groups enabled state chages, then reflect that change to
 * all it's child widgets.
 *
 * Warning: Making a Radio Group enabled or disabled will loose any enabled
 * setting a child may already have.  Consider changing the enabled state
 * via the Radio Group as a global over-ride for all child widgets.
 */
static EG_BOOL Callback_Enabled(EG_Widget *widget_ptr, EG_BOOL is_enabled)
{
	EG_RadioGroup *radiogroup_ptr;
	EG_Widget *tmp_widget_ptr;
	int i;

	CHECK_EG_WIDGET_IS_VALID(widget_ptr, EG_FALSE);
	EG_RADIOGROUP_GET_STRUCT_PTR(widget_ptr, radiogroup_ptr, EG_FALSE);

	/* Change state of Radio Group widget.
	 */
	(void) EG_Shared_UpdateEnabledState(widget_ptr, is_enabled);

	/* Reflect change with children too.
	 */
	for(i=0; i<radiogroup_ptr->count; i++){
		tmp_widget_ptr = radiogroup_ptr->button_widget_ptr[i];

		(void) EG_Shared_UpdateEnabledState(tmp_widget_ptr, is_enabled);
	}

	return(EG_TRUE);
}       

/* Remember we don't need to render the state change when a widget is stopped or
 * started!  We do however need to reflect that change to our children.
 */
static EG_BOOL Callback_Stopped(EG_Widget *widget_ptr, EG_BOOL is_stopped)
{
	int i;
	EG_RadioGroup *radiogroup_ptr;

	/* Stop radio group.
	 */
	EG_Widget_SetStoppedToggle(widget_ptr, is_stopped);
 	EG_RADIOGROUP_GET_STRUCT_PTR(widget_ptr, radiogroup_ptr, EG_FALSE);

	/* Stop child Radio Buttons.
	 */
	for(i=0; i<radiogroup_ptr->count; i++){
		EG_Widget_SetStoppedToggle(radiogroup_ptr->button_widget_ptr[i]
		 , is_stopped);
	}

	return(EG_TRUE);
}

/* When Radio Group is attached to a window, make sure all children are also
 * attached.
 */
static void Callback_Attach(EG_Widget *widget_ptr, EG_StringHash attach_to_type
 , void *attach_to_ptr, EG_BOOL attached)
{
	EG_RadioGroup *radiogroup_ptr;
	EG_Window *window_ptr;
	EG_StringHash tmp_attach_to_type;
	int i;

	/* Loose compiler warning.
	 */
	tmp_attach_to_type = attach_to_type;

	/* We only need to add the buttons to the window when attaching, we do
	 * nothing when detatching.
	 */
	if (attached != EG_TRUE)
		return;

	CHECK_EG_WIDGET_IS_VALID_VOID(widget_ptr);
	EG_RADIOGORUP_GET_STRUCT_PTR_VOID(widget_ptr, radiogroup_ptr);

	/* Get parent window.
	 */
	window_ptr = (EG_Window*) attach_to_ptr;
	CHECK_EG_WINDOW_IS_VALID_VOID(window_ptr);

	/* Add the radio buttons to window.
	 */
	for(i=0; i<radiogroup_ptr->count; i++){
		EG_Window_AddWidget(window_ptr
		 , radiogroup_ptr->button_widget_ptr[i]);
	}
}


/* Private functions:
 */

static void InitializePayload(EG_RadioGroup *radiogroup_ptr)
{
	int i;

	for(i=0; i<MAX_RADIOGROUP_BUTTONS; i++)
		radiogroup_ptr->button_widget_ptr[i] = NULL;	

	radiogroup_ptr->count = 0;
	radiogroup_ptr->selected = -1;
}

static void InitializeWidget(EG_Widget *widget_ptr)
{
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
	 , EG_Callback_Generic_LostFocus);

        (void) EG_Widget_SetCallback_Attach(widget_ptr, Callback_Attach);

        /* Initialize state:
         */
	(void) EG_Widget_SetCanGetFocusToggle(widget_ptr, EG_FALSE);
}


/* Public functions:
 */

EG_Widget* EG_RadioGroup_Create(const char *name_ptr)
{
        EG_RadioGroup *radiogroup_ptr;
        EG_Widget *widget_ptr;
	void *ptr;

        SHARED__ALLOC_PAYLOAD_STRUCT(ptr, EG_RadioGroup
         , "Unable to malloc EG_RadioGroup struct");
	radiogroup_ptr = (EG_RadioGroup*) ptr;

        SHARED__CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr
	 , EG_Widget_Type_RadioGroup, radiogroup_ptr);

        SHARED__ATTACH_PAYLOAD_TO_WIDGET(widget_ptr, radiogroup_ptr);

	InitializePayload(radiogroup_ptr);
	InitializeWidget(widget_ptr);

        return(widget_ptr);
}

EG_BOOL EG_RadioGroup_AddButton(EG_Widget *group_widget_ptr
 , EG_Widget *button_widget_ptr)
{
	EG_RadioGroup *radiogroup_ptr;

	CHECK_EG_WIDGET_IS_VALID(group_widget_ptr, EG_FALSE);
	EG_RADIOGROUP_GET_STRUCT_PTR(group_widget_ptr, radiogroup_ptr,EG_FALSE);

	/* Check we can add button.
	 */
	if (radiogroup_ptr->count >= MAX_RADIOGROUP_BUTTONS){
		EG_Log(EG_LOG_ERROR, dL"Can't add button to"
		 " EG_RadioGroup: Would exceed max button count.", dR);
		return(EG_FALSE);
	}

	/* Add the button
	 */
	switch ( EG_Widget_GetType(button_widget_ptr) ){

	case EG_Widget_Type_RadioButton:
		/* Attach the button
		 */
		radiogroup_ptr->button_widget_ptr[radiogroup_ptr->count++] = button_widget_ptr;

		/* Associate the radio button with this radio group.
		 */
		if (EG_RadioButton_SetGroupParent(button_widget_ptr, group_widget_ptr) != EG_TRUE)
			EG_Log(EG_LOG_ERROR,dL"Could not associate EG_RadioButton with EG_RadioGroup.\n", dR);

		/* If this is the first button, select it.  User will select another
		 * button later if required.
		 */
		if (radiogroup_ptr->selected == -1 && radiogroup_ptr->count == 1){
			radiogroup_ptr->selected = 1;
			if( EG_RadioButton_Tick(button_widget_ptr) != EG_TRUE)
				EG_Log(EG_LOG_WARNING, dL"EG_RadioButton could not set.", dR);
		}
		break;


	case EG_Widget_Type_ToggleButton:
		/* Attach the button
		 */
		radiogroup_ptr->button_widget_ptr[radiogroup_ptr->count++] = button_widget_ptr;

		/* Associate the toggle button with this radio group.
		 */
		if (EG_ToggleButton_SetGroupParent(button_widget_ptr, group_widget_ptr) != EG_TRUE)
			EG_Log(EG_LOG_ERROR,dL"Could not associate EG_ToggleButton with EG_RadioGroup.\n",dR);

		/* If this is the first button, select it.  User will select another
		 * button later if required.
		 */
		if (radiogroup_ptr->selected == -1 && radiogroup_ptr->count == 1){
			radiogroup_ptr->selected = 1;
			if( EG_ToggleButton_SetSelected(button_widget_ptr) != EG_TRUE)
				EG_Log(EG_LOG_WARNING, dL"EG_ToggleButton could not select.", dR);
		}
		break;

	default:
		EG_Log(EG_LOG_ERROR, dL"Can't attach child to EG_RadioGroup: "
		 "It's not of correct type.", dR);
		return EG_FALSE;
		break;
	}

	return(EG_TRUE);
}

void EG_RadioGroup_RemoveButton(EG_Widget *group_widget_ptr
 , EG_Widget *button_widget_ptr)
{
	EG_RadioGroup *radiogroup_ptr;
	int i;

	CHECK_EG_WIDGET_IS_VALID_VOID(group_widget_ptr);
	EG_RADIOGORUP_GET_STRUCT_PTR_VOID(group_widget_ptr, radiogroup_ptr);

	/* Find page widget.
	 */
	for(i=0; i<radiogroup_ptr->count; i++)
		if (EG_Widget_GetID(button_widget_ptr) == EG_Widget_GetID(
		 radiogroup_ptr->button_widget_ptr[i]))
			break;

	/* If not found, log error and exit.
	 */	
	if (i>=radiogroup_ptr->count){
		EG_Log(EG_LOG_ERROR, dL"Tried to delete widget %s from"
		 " EG_RadioGroup, when not memeber of group.", dR);
		return;
	}

	// [TODO] If the removed widget had focus, move focus, if still have focus, clear focus.
	// [TODO] If this button was selected make next button selected.
	// [TODO] Restructure the array (with the removed one missing).
	// [TODO] Tell window to repaint removed widgets area.
}

void EG_RadioGroup_DeleteButton(EG_Widget *group_widget_ptr
 , EG_Widget *button_widget_ptr)
{
	// [TODO] Same as above, but deletes widgets too.
}

EG_BOOL EG_RadioGroup_Select(EG_Widget *button_widget_ptr)
{
	EG_Window *window_ptr;
	EG_Widget *group_widget_ptr;
	EG_Widget *next_button_widget_ptr;

	EG_RadioButton *radio_button_ptr;
	EG_ToggleButton *toggle_button_ptr;

	EG_RadioGroup *radiogroup_ptr;
	int i;

	CHECK_EG_WIDGET_IS_VALID(button_widget_ptr, EG_FALSE);

	/* Get widget payload.
	 */
	switch ( EG_Widget_GetType(button_widget_ptr) ){
	case EG_Widget_Type_RadioButton:
		EG_RADIOBUTTON_GET_STRUCT_PTR(button_widget_ptr, radio_button_ptr, EG_FALSE);
		break;
	case EG_Widget_Type_ToggleButton:
		EG_TOGGLEBUTTON_GET_STRUCT_PTR(button_widget_ptr, toggle_button_ptr, EG_FALSE);
		break;
	default:
		EG_Log(EG_LOG_ERROR, dL"Button %s is not of valid type.", dR                                                   , EG_Widget_GetName(button_widget_ptr) );
		return EG_FALSE;
		break;
	}

//	if (EG_Widget_GetType(button_widget_ptr) == EG_Widget_Type_RadioButton)
//		EG_RADIOBUTTON_GET_STRUCT_PTR(button_widget_ptr, radio_button_ptr, EG_FALSE);
//	else if (EG_Widget_GetType(button_widget_ptr) == EG_Widget_Type_ToggleButton)
//		EG_TOGGLEBUTTON_GET_STRUCT_PTR(button_widget_ptr, toggle_button_ptr, EG_FALSE);
//	else{
//		EG_Log(EG_LOG_ERROR, dL"Button %s is not of valid type.", dR
//		 , EG_Widget_GetName(button_widget_ptr) );
//		return EG_FALSE;
//	}

	/* Get EG_RadioGroup, if NULL, then not associated with a group yet.
	 */
	switch ( EG_Widget_GetType(button_widget_ptr) ){
	case EG_Widget_Type_RadioButton:
		if ( (group_widget_ptr=EG_RadioButton_GetGroupParent(button_widget_ptr)) == NULL){
			EG_Log(EG_LOG_ERROR, dL"EG_RadioButton is not associated with"
			 " an EG_RadioGroup.", dR);
			return EG_FALSE;
		}
		break;
	case EG_Widget_Type_ToggleButton:
		if ( (group_widget_ptr=EG_ToggleButton_GetGroupParent(button_widget_ptr)) == NULL){
			EG_Log(EG_LOG_ERROR, dL"EG_ToggleButton is not associated with"
			 " an EG_RadioGroup.", dR);
			return EG_FALSE;
		}
		break;
	default:
		/* To keep gcc happy, the switch above will make sure this is never called anyway.
		 */
		group_widget_ptr = NULL; break;
	}
	radiogroup_ptr = (EG_RadioGroup*) EG_Widget_GetPayload(group_widget_ptr);


	/* Varify radio button is on the same window as the radio group.
	 */
	// [TODO]


	/* Find button in the radio group and select it.
	 */
	for(i=0; i<radiogroup_ptr->count; i++){
		next_button_widget_ptr = radiogroup_ptr->button_widget_ptr[i];
		if (next_button_widget_ptr != NULL){
			if ( EG_Widget_GetID(next_button_widget_ptr) ==
			 EG_Widget_GetID(button_widget_ptr) )
				break;
		}
	}
	if (i>=radiogroup_ptr->count){
		EG_Log(EG_LOG_ERROR, dL"Button not on EG_RadioGroup"
		 " list.", dR);
		return(EG_FALSE);
	}

	/* Get window.
	 */
	window_ptr= (EG_Window*) EG_Widget_GetWindow(button_widget_ptr);

	/* Loose focus (if have it, or are associated with a window yet).
	 */
	if (window_ptr != NULL)
		EG_Window_ClearFocus(window_ptr);

	/* Clear all buttons.
	 */
	for(i=0; i<radiogroup_ptr->count; i++){
		next_button_widget_ptr = radiogroup_ptr->button_widget_ptr[i];
		if ( EG_Widget_GetID(next_button_widget_ptr)
		 != EG_Widget_GetID(button_widget_ptr) ){
//			if (EG_RadioButton_Untick(next_button_widget_ptr)
//			 != EG_TRUE)
//				EG_Log(EG_LOG_ERROR, dL"EG_RadioButton could"
//				 " not unset.", dR);
//			else
//				(void) EG_Widget_RepaintLot(
//				 next_button_widget_ptr);
			
			switch ( EG_Widget_GetType(next_button_widget_ptr) ){
			case EG_Widget_Type_RadioButton:
				if (EG_RadioButton_Untick(next_button_widget_ptr) != EG_TRUE)
					EG_Log(EG_LOG_ERROR, dL"EG_RadioButton could not unset.", dR);
				break;
			case EG_Widget_Type_ToggleButton:
				if (EG_ToggleButton_SetUnselected(next_button_widget_ptr) != EG_TRUE)
					EG_Log(EG_LOG_ERROR, dL"EG_ToggleButton could not be unselected."
					 , dR);
				break;
			}
			(void) EG_Widget_RepaintLot(next_button_widget_ptr);
		}
	}



	/* Set focus to new button and set it.
	 */
//	if( EG_RadioButton_Tick(button_widget_ptr) != EG_TRUE)
//		EG_Log(EG_LOG_WARNING, dL"EG_RadioButton could not set.", dR);
//	else{
//		if (window_ptr != NULL){
//			if (EG_Window_SetFocusToThisWidget(button_widget_ptr) != EG_TRUE){
//				EG_Window_ClearFocus(window_ptr);
//				EG_Log(EG_LOG_WARNING, dL"Could not move focus to pressed"
//				 " button.", dR);
//			}
//		}
//	}

	switch ( EG_Widget_GetType(button_widget_ptr) ){
	case EG_Widget_Type_RadioButton:
		if( EG_RadioButton_Tick(button_widget_ptr) != EG_TRUE)
			EG_Log(EG_LOG_WARNING, dL"EG_RadioButton could not set.", dR);
		else{
			if (window_ptr != NULL){
				if (EG_Window_SetFocusToThisWidget(button_widget_ptr) != EG_TRUE){
					EG_Window_ClearFocus(window_ptr);
					EG_Log(EG_LOG_WARNING, dL"Could not move focus to pressed"
					 " button.", dR);
				}
			}
		}
		break;
	case EG_Widget_Type_ToggleButton:
		if( EG_ToggleButton_SetSelected(button_widget_ptr) != EG_TRUE)
			EG_Log(EG_LOG_WARNING, dL"EG_ToggleButton could not be selected.", dR);
		else{
			if (window_ptr != NULL){
				if (EG_Window_SetFocusToThisWidget(button_widget_ptr) != EG_TRUE){
					EG_Window_ClearFocus(window_ptr);
					EG_Log(EG_LOG_WARNING, dL"Could not move focus to pressed"
					 " button.", dR);
				}
			}
		}
		break;
	}

	return(EG_TRUE);
}
