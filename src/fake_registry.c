/* Like a Windows registry, but worse (if that's even possible).
 *
 * What was I thinking when I made this?  This sucks big-time.
 */

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "fake_registry.h"

#include <functions.h>
#include <gui.h>

#include "user_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Malloc and Free (just use EG's versions):
 */
#define FR_MALLOC EG_Malloc
#define FR_FREE EG_Free


/* Extra UNIX specific includes:
 */
#ifdef WITH_UNIX_EXTRAS
#	include <sys/types.h>
#	include <unistd.h>
#	include <pwd.h>
#	include <errno.h>
#	include <sys/stat.h>
#	include <dirent.h>
#endif


/* Fake registry definitions:
 */
#define FR_FILENAME_LEN		512


/* Fake registry variables:
 */
typedef struct me{
	char 		*key_ptr;
	char 		*value_ptr;

	struct me 	*next_ptr;
}FakeRegistry;

static char reg_file[FR_FILENAME_LEN];

static FakeRegistry *reg_head_ptr;
static FakeRegistry *reg_tail_ptr;

static BOOL SetFakeRegistryItem(const char *key_ptr, const char *value_ptr);
static BOOL GetFakeRegistryItem(const char *key_ptr, char **value_ptr);


/* Public API:
 */

/* Initialize the fake registry.
 */
void InitializeFakeRegistry(void)
{
	/* There must be a better way of doing this:
	 */
	char row[10*1024], key[10*1024], value[10*1024];
	FILE *f_ptr;
	int i;

	/* Initialize fake registry linked list.
	 */
	reg_head_ptr = reg_tail_ptr = NULL;

	/* Load a previously saved fake registry (if this fails we assume one
	 * does not yet exist and leave creating one up to the save function).
	 */
	if ( GetUserConfigPath(reg_file, FR_FILENAME_LEN) == NULL)
		return;

	/* Append filename of registry to path.
	 */
	strcat(reg_file, REG_FILENAME);

	/* Test file, quit if does not exist (first time beebem ran so
	 * no entries).
	 */
	if ( (f_ptr=fopen(reg_file, "r")) == NULL)
		return;

	/* Load existing registry
	 */
	while (fgets( row, 256, f_ptr ) != NULL){
		i = strlen(row) -1;
		while(i>=0){
 			if (row[i]<32) row[i] = '\0';
				i--;
		}

		if (strlen(row)<=0)
			continue;

		if (row[0] == '#')
			continue;

		for(i=0; i< (int) strlen(row); i++)
			if (row[i] == '#'){
				row[i]='\0';
				break;
			}

		i = strlen(row) -1;
		while(i>=0){
			if (row[i] == '=') break;
			i--;
		}
		if (i<=0)
			continue;

		memcpy(key, row, i);
		key[i]='\0';

		value[0] = '\0';
		if(strlen(row)-i-1>0){
			memcpy(value, row+i+1, strlen(row)-i-1);
			value[strlen(row)-i-1] = '\0';
		}

		i = strlen(value) -1;
		while(value[i]<=32 && i>=0){
			value[i] = '\0';
			i--;
		}

		if (SetFakeRegistryItem(key, value) != TRUE)
                	pERROR(dL"Failed to add '%s' to the registry!\n", dR
			 , key);
	}
	fclose(f_ptr);
}

void RemoveFakeRegistryItem(const char *key_ptr)
{
	FakeRegistry *ptr_previous = NULL;
	FakeRegistry *ptr = reg_head_ptr;

	while( ptr != NULL){
		if (strcasecmp(key_ptr, ptr->key_ptr) == 0){
			
			if (ptr_previous != NULL)
				ptr_previous->next_ptr = ptr->next_ptr;

			if (reg_head_ptr == ptr) reg_head_ptr = NULL;
			if (reg_tail_ptr == ptr) reg_tail_ptr = ptr_previous;

			FR_FREE(ptr->key_ptr);
			FR_FREE(ptr->value_ptr);
			FR_FREE(ptr);
			return;
		}
	
		ptr_previous = ptr;
		ptr = ptr->next_ptr;
	}
}

void DumpFakeRegistry(void)
{
	FakeRegistry *ptr = reg_head_ptr;

	while( ptr != NULL){
		printf("[%8lX][%8lX][%8lX] [%s]=\"%s\"\n"
		, (unsigned long) ptr, (unsigned long) ptr->key_ptr
		, (unsigned long) ptr->value_ptr 
		, ptr->key_ptr, ptr->value_ptr);

		ptr = ptr->next_ptr;
	}
}

BOOL SaveFakeRegistry(void)
{
	FILE *f_ptr;
	FakeRegistry *registry_ptr = reg_head_ptr;

	/* Set file path, if fail cannot continue.
	 */ 
        if ( GetUserConfigPath(reg_file, FR_FILENAME_LEN) == NULL)
                return FALSE;

        /* Append filename of registry to path.
         */
        strcat(reg_file, REG_FILENAME);

	/* Save the fake registry to disc.
	 */
	if ( (f_ptr=fopen(reg_file, "w")) != NULL){

		fprintf(f_ptr, "# Beebem configuration file.\n");
		fprintf(f_ptr, "#\n");
		fprintf(f_ptr, "# This is a fake windows registry."
		 " Edit this at your own risk!\n");
		fprintf(f_ptr, "# (TIP: Deleted rows will be recreated next"
		 " time you save config.)\n");

		while( registry_ptr != NULL){
			fprintf(f_ptr, "%s=%s\n", registry_ptr->key_ptr
			 , registry_ptr->value_ptr);
                	registry_ptr = registry_ptr->next_ptr;
        	}

		fclose(f_ptr);
	}else{
		fprintf(stderr, "Could not save configuration [%s]!\n"
		 , reg_file);
	}

	return TRUE;
}

void DestroyFakeRegistry(void)
{
	FakeRegistry *ptr = reg_head_ptr;
	FakeRegistry *next_ptr;

	while( ptr != NULL){
		FR_FREE(ptr->key_ptr);
		FR_FREE(ptr->value_ptr);

		next_ptr = ptr->next_ptr;
		FR_FREE(ptr);

		ptr = next_ptr;
	}

	reg_head_ptr = NULL;
	reg_tail_ptr = NULL;
}

/* Private functions:
 */

static BOOL GetFakeRegistryItem(const char *key_ptr, char **value_ptr)
{
	FakeRegistry *ptr = reg_head_ptr;

	while( ptr != NULL){
		if (strcasecmp(key_ptr, ptr->key_ptr) == 0 ){
			*value_ptr = ptr->value_ptr;
			return(TRUE);
		}

		ptr = ptr->next_ptr;
	}

	*value_ptr = NULL;
	return(FALSE);
}

static BOOL SetFakeRegistryItem(const char *key_ptr, const char *value_ptr)
{
	FakeRegistry *ptr = reg_head_ptr;
	char *new_value_ptr;
	char *new_key_ptr;

	if ( (new_value_ptr=(char*) FR_MALLOC(strlen(value_ptr)+1)) != NULL)
		strcpy(new_value_ptr, value_ptr);
	else return(FALSE);

	/* If already in registry, just change the value:
	 */
	while( ptr != NULL){
		if (strcasecmp(key_ptr, ptr->key_ptr) == 0 ){
			FR_FREE(ptr->value_ptr);
			ptr->value_ptr = new_value_ptr;

			return(TRUE);
		}
		ptr = ptr->next_ptr;
	}

	/* If don't already have, create a new registry item and tag it onto
	 * the end:
	 */
	if ( (new_key_ptr=(char*) FR_MALLOC(strlen(key_ptr)+1)) != NULL){
		strcpy(new_key_ptr, key_ptr);
	}else{ FR_FREE(new_value_ptr); return(FALSE); }

	if ( (ptr= (FakeRegistry*) FR_MALLOC(sizeof(FakeRegistry))) != NULL ){
		ptr->key_ptr = new_key_ptr;
		ptr->value_ptr = new_value_ptr;
		ptr->next_ptr = NULL;
	}else{
		FR_FREE(new_value_ptr);
		FR_FREE(new_key_ptr);
		return(FALSE);
	}

	if (reg_head_ptr == NULL){
		reg_head_ptr = ptr;
	}else{ if (reg_tail_ptr != NULL) reg_tail_ptr->next_ptr = ptr; }
	reg_tail_ptr = ptr;

	return(TRUE);
}


/* The following functions wrap the static linked list functions above:
 */

BOOL GetFakeRegistryItem_String(const char *key_ptr, char **value_ptr)
{
	return(GetFakeRegistryItem(key_ptr, value_ptr));
}

BOOL SetFakeRegistryItem_String(const char *key_ptr, const char *value_ptr)
{
	return(SetFakeRegistryItem(key_ptr, value_ptr));
}

BOOL GetFakeRegistryItem_Double(const char *key_ptr, double *double_ptr)
{
	char *c_ptr;

	if (! GetFakeRegistryItem(key_ptr, &c_ptr))
		return(FALSE);

	*double_ptr = atof(c_ptr);
	return(TRUE);
}

BOOL SetFakeRegistryItem_Double(const char *key_ptr, const double double_val)
{
	char buffer[100+1]; // It should never be this big!

	sprintf(buffer, "%lf", double_val);
	return(SetFakeRegistryItem(key_ptr, buffer));
}

BOOL GetFakeRegistryItem_Long(const char *key_ptr, long *long_val)
{
	char *val_ptr;

	if ( GetFakeRegistryItem(key_ptr, &val_ptr) ){
		*long_val = atol(val_ptr);
		return(TRUE);
	}else{
		return(FALSE);
	}
}

BOOL SetFakeRegistryItem_Long(const char *key_ptr, const long long_val)
{
	char *buffer[100+1]; // It should never be this big!

	sprintf( (char*) buffer, "%ld", long_val);
	return(SetFakeRegistryItem(key_ptr, (char*) buffer));
}

