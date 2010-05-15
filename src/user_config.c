/* START OF user_config.c ------------------------------------------------------
 *
 * 	Locate/create user configuration files.
 *
 * 	There are two versions of the below, a UNIX version (WITH_UNIX_EXTRAS)
 *	and an ANSI C version.  The ANSI C version will fail (#error) where you
 *	need to add OS specific code.
 *
 *	(It'll only support UNIX and <other>, not multiple systems, rethink it.)
 *
 *	(This again is not fantastic code, but if I'm going to get anywhere with
 *	this I'm going to have to speedup development a bit.)
 *
 *      ---
 *      Written by David Eggleston (2006) <deggleston@users.sourceforge.net>
 *      for the 'BeebEm' Acorn BBC Model B, Integra-B, Model B Plus and
 *      Master 128 emulator.
 *
 *      This file is part of BeebEm and may be copied only under the terms of
 *	either the GNU General Public License (GPL) or Dr. David Alan Gilbert's
 *	BeebEm license.
 *
 *      For more details please visit:
 *
 *      http://www.gnu.org/copyleft/gpl.html
 *      ---
 */

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "user_config.h"
#include <gui.h>
#include <sdl.h>
#include "beebem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* Sub directory (from DATA_DIR) for user configuration directory when not UNIX.
 */
#define NONE_UNIX_CONFIG_DIR "config"

/* Assumes appending NULL to a string is OK, will return failure when appending
 * a string to NULL.
 *
 * The path and filename functions use this to make sure we don't manipulate
 * paths that are too long.
 *
 * len should include the string terminater, so 10 would be 9 valid characters
 * and one '\0' terminator.
 *
 * (This function probably shouldn't be here, but what the heck).
 */
static BOOL ConcatenateStrings(char *destination, char *append, size_t len)
{
	if (destination == NULL){
		qERROR("Tried to append string to a NULL pointer.");
		return FALSE;
	}
	if (append == NULL || strlen(append)==0) return TRUE;

	/* If the actual length of destination is greater than len suggests it
	 * should be, bail out with error. (>= as len includes '\0' and strlen
	 * does not).
	 */
	if (strlen(destination)>=len){
		qERROR("The actual length of destination string is greater than"
	 	 " it's length flag suggests..");
		return FALSE;
	}

	/* Concatenate as many characters as we can to destination.
	 * strncat will add the terminator '\0' to the end (unlike strncpy) so
	 * we need to subtract one from len.
	 */
	strncat( destination, append, len - strlen(destination) - 1 );

	/* If we could not concatenate all characters from source to destination
	 * then return failure.
         */
	if (strlen(destination) + strlen(append) >= len)
		return FALSE;
	
	/* Otherwise, success.
	 */
	return TRUE;
}

/* (Same as concatenate function above, wraps strcpy with a few extra checks.)
 */
static BOOL CopyString(char *destination, char *source, size_t len)
{
	/* Make sure string is valid.
	 */
	if (destination == NULL){
                qERROR("Tried to copy string to a NULL pointer.");
                return FALSE;
        }
	if (len<1){
		qERROR("Tried to copy string to string with length zero.");
		return FALSE;
	}

	/* If the actual length of destination is greater than len suggests it
	 * should be, bail out with error.
	 */
	if (strlen(destination)>=len){
		qERROR("The actual length of destination string is greater than"
	 	 " it's length flag suggests..");
		return FALSE;
	}

	/* Clear destination string. Make sure this always happens before
	 * validating the source string. If we copy a NULL to a string the
	 * string should be cleared.
	 */
	destination[0]='\0';

	/* Don't do anything if source is invalid.
	 */
	if (source == NULL || strlen(source)==0) return TRUE;


        /* If destination string is long enough to hold the source strings
	 * contents copy as many characters as we can.
         */
	strncpy(destination, source, len);

	/* strncpy will not terminate the destination string if the length of
	 * the source string is greater or equal to the size of the destination
	 * string, so we must do it ourselves.
	 *
	 * We should also return FALSE if only part of the string copied.
	 */
	if (strlen(destination) >= len){
		destination[len-1]='\0'; return FALSE;
	}

        return TRUE;
}

/* Convert path into a C path.  Assumes pointer passed is a valid terminated C 
 * string.
 *
 * Depending on your OS you may need to change this. Currently just checks if
 * it's a windows path and converts the "\"s into "/"s.
 *
 * len includes the string terminator '\0'.
 */
static void ConvertPathToC(char *path, size_t len)
{
	int i;

        if (path != NULL)
		for (i=0; i<=strlen(path) && i<len; i++)
			if (path[i]=='\\')
				path[i]='/';
}

/* Will convert string to a valid C path.
 *
 * If a string is not empty, the none empty part is considered a path and a
 * forward slash is appended.  If the string already has a trailing forward
 * slash, no changes are made to the string.
 *
 * len includes the string terminator '\0'.
 */
static BOOL TurnStringIntoAPath(char *path, size_t len)
{
	/* Don't bother if path NULL.
	 */
	if (path == NULL){
		qERROR("Path is a NULL string.");
		return FALSE;
	}

        /* Convert the string into a C friendly path.
         */
        ConvertPathToC(path, len);

	/* Don't append forward slash if string empty.
	 */
	if (strlen(path)==0) return TRUE;

	/* Do not append a forward slash if we already have one.
	 */
	if (path[strlen(path)-1] == '/')
		return TRUE;

	/* Try to append a forward slash to the string and return the cat
	 * functions result.
	 */
	return ConcatenateStrings(path, "/", len);
}

/* Will add a directory string to a path string.  Will validate the
 * concationation and convert the path string part to something C can use if
 * required. Does not convert the directory string part, just appends it.
 *
 * Will add a prefixed forward slash to the resulting path.
 */
static BOOL AppendDirToPath(char *path, char *dir, size_t len)
{
	/* Make sure path really is a path. If conversion fails (the string is
	 * a NULL pointer), bail out with false.
	 */
	if (! TurnStringIntoAPath(path, len) ) return FALSE;

	/* Do nothing if appending an empty string or NULL (make sure this is
	 * after TurnStringIntoAPath as we say it will convert the path argument
	 * regardless of the directory strings contents in the description).
	 */
	if (dir == NULL || strlen(dir) == 0) return TRUE;

	/* If dir to add is prefixed with a forward slash and the existing
	 * path it's to be appended to is not empty, then remove the trailing
	 * forward slash from the path, otherwise we'll end up with two.
	 *
 	 * (The '/' check on the path is not needed as TurnStringIntoAPath will
	 * add one if its source string is not empty, but I'll add it anyway.)
	 */
	if (dir[0]=='/' && strlen(path)>0 && path[strlen(path)-1] == '/')
		path[strlen(path)-1] = 0;

	/* Append the dir to the path
	 */
	if (! ConcatenateStrings(path, dir, len) )
		return FALSE;

	/* Revalidate this new path (if dir didn't end in a trailing forward
	 * slash it will get one here).
	 */
	return TurnStringIntoAPath(path, len);
}

/* Will append a filename to a path.
 */
static BOOL AppendFilenameToPath(char *path, char *filename, size_t len)
{
        /* Make sure path really is a path. If conversion fails, bail out.
         */
        if (! TurnStringIntoAPath(path, len) ) return FALSE;

	/* If the filename is not valid, fail
	 */
	if (filename == NULL || strlen(filename)==0) return FALSE;

	/* Append the filename to the Path and return result.
	 */
	return ConcatenateStrings(path, filename, len);
}

/* Test whether a regualr file exists.
 */
static BOOL TestFile(char *file)
{
	FILE *f;

	/* If the file to test is invalid, return false.
	 */
	if (file == NULL || strlen(file)==0) return FALSE;
	
	/* Suck it and see!
	 */
	if ( (f=fopen(file, "r")) == NULL) return FALSE;
	fclose(f);

	return TRUE;
}

/* Test whether a directory entry exists.
 */
static BOOL TestDirectory(char *file)
{
	/* Bail if the file to test is invalid, return false.
	 */
	if (file == NULL || strlen(file)==0) return FALSE;

#ifdef WITH_UNIX_EXTRAS
	DIR *dir;

	if ( (dir=opendir(file)) == NULL) return FALSE;
	closedir(dir);

	return TRUE;
#else
#	error You need to add TestDirectory support for your OS here.
#endif
}

/* Make a copy of a file.
 *
 * Both paths must be fully quantified (leading "/") for security reasons.
 */
static BOOL CopyFile(char *source_file, char *dest_file)
{
	/* Terminate program if file paths are not fully quantified.
	 *
	 * (They always should be for BeebEm, and if they're not, then it could
	 * be a security risk).
	 */
	if ( source_file == NULL || dest_file == NULL
	 || strlen(source_file) == 0 || strlen(dest_file) == 0
	 || source_file[0] != '/' || dest_file[0] != '/') {
		qFATAL("File copy arguments are either invalid or not fully"
		 " quantified - bailing out (call security).");
		exit(1);
	}

	/* Use 'cp' command instead of copying the files in C (cp must be
	 * in your PATH).
	 */
#ifdef WITH_SYSTEM_CP
	char command[5*1024];

	sprintf(command, "cp %s %s", source_file, dest_file);
	if (system(command) != 0){
		qERROR("cp command failed.");
		return FALSE;
	}else
		return TRUE;
#else
	/* Use C to copy the file instead of relying on 'cp'.
	 */
	FILE *src_f, *dst_f;
	int c, failed=0;

	if (source_file == NULL){
		qERROR("Source file is null string.");
		return FALSE;
	}
	if (dest_file == NULL){
		qERROR("Destination file is null string.");
		return FALSE;
	}

	/* Open the source file for reading, quit on fail.
	 */
	if ( (src_f=fopen(source_file, "r")) == NULL){
		pERROR(dL"Unable to open source file [%s]."
		 , dR, source_file);
		return FALSE;
	}

	/* Open and truncate the dest file for writing,
	 * close source file and quit on fail.
	 */
	if ( (dst_f=fopen(dest_file, "w")) == NULL){
		pERROR(dL"Unable to write to dest file [%s]."
		 , dR, dest_file);
		fclose(src_f);
		return FALSE;
	}

	/* Copy data from the source file to the dest file.
	 * (this is not fast!)
	 */
	while ( (c=fgetc(src_f)) != EOF){
		if (fputc(c, dst_f)==EOF){
			failed=1;
			break;
		}
	}

	/* Close files
	 */
	fclose(dst_f);
	fclose(src_f);

	if (failed==1){
		qERROR("File copy failed during copy.");
		return FALSE;
	}else
		return TRUE;
#endif
}


/* Create a new directory (succeeds if dir already exists).
 */
static BOOL CreateDirectory(char *file)
{
	/* Terminate program if the new directories path is not fully
	 * quantified.
	 *
	 * (They always should be for BeebEm, and if they're not, then it could
	 * be a security risk).
	 */
	if ( file == NULL || strlen(file) == 0 || file[0] != '/') {
		qFATAL("Directory creation argument is either invalid or not"
		 " fully quantified - bailing out (call security).");
		exit(1);
	}

#ifdef WITH_UNIX_EXTRAS
	if (TestDirectory(file)) return TRUE;

	errno = 0;
	if (mkdir(file, (mode_t) S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP
	 | S_IXGRP | S_IROTH | S_IXOTH ) == 0)
		return TRUE;

	pFATAL(dL"Could not create directory: %s", dR, strerror(errno));
	return FALSE;
#else
#	error You need to add CreateDirectory support for your OS here.
#endif
}

/* Get users config directory ("${DATA_DIR}/config/" or "$HOME/.beebem/),
 * does add a trailing forward slash.
 *
 * On UNIX for a user called dave, would return a pointer to something like:
 * "/home/dave/.beebem/".  If this directory does not exist, it's created.
 *
 * On None UNIX systems, the installation path of the application + a config
 * directory name are used instead.  So something like:
 * "/applications/beebem/config/" will be returned instead. If this directory
 * does not exist, it's created.
 */
char* GetUserConfigPath(char *path, size_t len)
{
	/* Return with error if string illegal length of 2 is valid.
 	 * (Remember BeebEm paths must be fully quanitified so the minimum would
	 * be "/\0", two chars.
	 */
	if (len<2) return NULL;

	/* Clear provided string
	 */
	path[0] = 0;

#ifdef WITH_UNIX_EXTRAS
	struct passwd *pw_ptr;

	/* Find user in password database.
	 */
	errno = 0; 
	if ( ! (pw_ptr=getpwuid(getuid())) ){
		/* Not found, clear config filename.
		 */
		pFATAL(dL"Unable to determine user home dir: %s", dR, strerror(errno));
		exit(1);
	}else{

		if (! AppendDirToPath(path, pw_ptr->pw_dir, len) ){
			qFATAL("Unable to accept user accounts dir.");
			exit(1);
		}

		if (! AppendDirToPath(path, "." PACKAGE, len) ){
			qFATAL("Unable to append ." PACKAGE " to user accounts dir.");
			exit(1);
		}
	}
#else
	/* Root of application data dir
	 */
	
	if (! AppendDirToPath(path, DATA_DIR, len) ){
		qFATAL("Unable to determine default user config dir.");
		exit(1);
	}

	/* Add config dir
	 */
	if (! AppendDirToPath(path,NONE_UNIX_CONFIG_DIR, len) ){
		qFATAL("Unable to accept default user config dir.");
		exit(1);
	}
#endif

	/* If configuration directory does not exist, create it.
	 */
	if (! TestDirectory(path) ){
		if (! CreateDirectory(path)){
			qFATAL("Unable to create user config dir.");
			exit(1);
		}

#ifndef WITHOUT_WELCOME_MESSAGE
//#	ifdef WITH_UNIX_EXTRAS
//
//		/* If you don't like this then take the source code and
//		 * maintain your own version of BeebEm.
//		 */
//		if (TestDirectory("/usr/share/doc/release-notes/SUSE_Linux") ) {
//			EG_Draw_FlushEventQueue();
//			if (EG_MessageBox(screen_ptr, EG_MESSAGEBOX_STOP
//		 	 , "Attention Novell customer", SUSE_MESSAGE, "Continue"
//			 , "Quit now", NULL, NULL, 0) == 1)
//				exit(1);
//		}
//#	endif
		EG_Draw_FlushEventQueue();
		EG_MessageBox(screen_ptr, EG_MESSAGEBOX_INFORMATION
		 , "Welcome to BeebEm!", WELCOME_MESSAGE, "OK", NULL, NULL
		 , NULL, 0);
		
#endif
	}
	return path;
}


/* The following functions are called within the emulator core to determine
 * the location of config files.
 *
 * If the file does not exist, most of these functions will try to create it
 * locally (in the users config directory) first.
 */

static BOOL MakeSureBufferIsSane(char *buffer, size_t length)
{
	/* Make sure buffer is sane.
	 */
	if (length<1){
		qERROR("Buffer is too small to contain a string.");
		return FALSE;
	}
	if (buffer == NULL){
		qERROR("Buffer is NULL.");
		return FALSE;
	}

	/* Make sure that whatever happens from now on the buffer
	 * is blank ("").
	 */
	buffer[0]=0;

	return TRUE;
}

static BOOL GetConfigFilePath(char *filepath, size_t length, char *file)
{
	/* Determine path (GetUserConfigPath will make sure
	 * the .beebem directory is created if it does not exist).
	 */
	if ( GetUserConfigPath(filepath, length) == NULL ){
		pERROR(dL"Unable to determine %s file path.", dR, file);
		return FALSE;
	}

	/* Append file to the path.
	 */
	if (! AppendFilenameToPath(filepath, file, length) ){
		return FALSE;
	}

	return TRUE;
}

static BOOL CreateFileFromMaster(char *source_filepath, char *master_filepath)
{
	/* Test filepath, if does not exist create it:
	 */
	if (TestFile(source_filepath) == FALSE){
		/* Make local copy (in ~/.beebem/ for UNIX)
		 */
		if (CopyFile(master_filepath, source_filepath) == FALSE){
			pERROR(dL"Unable to make copy of master file '%s'.", dR
			 , master_filepath);
			return FALSE;
		}
	}

	return TRUE;
}

/*	*	*	*	*	*	*	*	*	*
 */

//static char* CopyFilePathToBuffer(char *filepath, char *buffer
// , size_t buffer_length)
//{
//	/* Copy filepath to buffer. Fail if buffer too small.
//	 * Return pointer to buffer.
//	 */
//	if (strlen(filepath)+1 <= buffer_length){
//		strcpy(buffer, filepath);
//		return(buffer);
//	}else
//		return(NULL);
//}

/* Return "~/.beebem/roms.cfg", if does not exist creates it using
 * master copy in "share/beebem/config/roms.cfg".
 */
char* GetLocation_roms_cfg(char *buffer, size_t length)
{
//	char filepath[MAX_PATH_LEN];
//
//	if (MakeSureBufferIsSane(buffer, length) != TRUE) return NULL;
//
//	if (GetConfigFilePath(filepath, MAX_PATH_LEN, "roms.cfg") != TRUE)
//		return NULL;
//
//	if (CreateFileFromMaster(filepath, DATA_DIR"/config/roms.cfg") == FALSE)
//		return NULL;
//
//	return CopyFilePathToBuffer(filepath, buffer, length);



	if (MakeSureBufferIsSane(buffer, length) != TRUE)
		return NULL;

	if (GetConfigFilePath(buffer, length, "roms.cfg") != TRUE)
		return NULL;

	if (CreateFileFromMaster(buffer, DATA_DIR"/config/roms.cfg") == FALSE)
		return NULL;

	return buffer;
}


/* Return "~/.beebem/econet.cfg", if does not exist creates it using master
 * copy in "share/config/econet.cfg".
 */
char* GetLocation_econet_cfg(char *buffer, size_t length)
{
//	char filepath[MAX_PATH_LEN];
//
//	if (MakeSureBufferIsSane(buffer, length) != TRUE) return NULL;
//
//	if (GetConfigFilePath(filepath, MAX_PATH_LEN, "econet.cfg") != TRUE)
//		return NULL;
//
//	if (CreateFileFromMaster(filepath, DATA_DIR"/config/econet.cfg")
//	 == FALSE) return NULL;
//
//	return CopyFilePathToBuffer(filepath, buffer, length);


	
	if (MakeSureBufferIsSane(buffer, length) != TRUE)
		return NULL;

	if (GetConfigFilePath(buffer, length, "econet.cfg") != TRUE)
		return NULL;

	if (CreateFileFromMaster(buffer, DATA_DIR"/config/econet.cfg") == FALSE)
		return NULL;

	return buffer;
}


/* Return "~/.beebem/phroms.cfg", if does not exist creates it using master
 * copy in "share/config/phroms.cfg".
 */
char* GetLocation_phroms_cfg(char *buffer, size_t length)
{
//	char filepath[MAX_PATH_LEN];
//
//	if (MakeSureBufferIsSane(buffer, length) != TRUE) return NULL;
//
//	if (GetConfigFilePath(filepath, MAX_PATH_LEN, "phroms.cfg") != TRUE)
//		return NULL;
//
//	if (CreateFileFromMaster(filepath, DATA_DIR"/config/phroms.cfg")
//	 == FALSE) return NULL;
//
//	return CopyFilePathToBuffer(filepath, buffer, length);

	


	if (MakeSureBufferIsSane(buffer, length) != TRUE)
		return NULL;

	if (GetConfigFilePath(buffer, length, "phroms.cfg") != TRUE)
		return NULL;

	if (CreateFileFromMaster(buffer, DATA_DIR"/config/phroms.cfg") == FALSE)
		return NULL;

	return buffer;
}


/* Return "~/.beebem/cmos.ram", if does not exist creates it using master
 * copy in "share/state/cmos.ram".
 */
char* GetLocation_cmos_ram(char *buffer, size_t length)
{
//	char filepath[MAX_PATH_LEN];
//
//	if (MakeSureBufferIsSane(buffer, length) != TRUE) return NULL;
//
//	if (GetConfigFilePath(filepath, MAX_PATH_LEN, "cmos.ram") != TRUE)
//		return NULL;
//
//	if (CreateFileFromMaster(filepath, DATA_DIR"/state/cmos.ram")
//	 == FALSE) return NULL;
//
//	return CopyFilePathToBuffer(filepath, buffer, length);



	if (MakeSureBufferIsSane(buffer, length) != TRUE)
		return NULL;

	if (GetConfigFilePath(buffer, length, "cmos.ram") != TRUE)
		return NULL;

	if (CreateFileFromMaster(buffer, DATA_DIR"/state/cmos.ram") == FALSE)
		return NULL;

	return buffer;
}

#define TUBE_ROM "/roms/other/6502tube.rom"
char* GetLocation_tube(char *buffer, size_t length)
{
	if (MakeSureBufferIsSane(buffer, length) != TRUE)
		return NULL;

	if (length< strlen(DATA_DIR TUBE_ROM) )
		return NULL;

	strcpy(buffer, DATA_DIR TUBE_ROM);
	return buffer;	
}

#define TELETEXT_FONT "/resources/teletext.fnt"
char* GetLocation_teletext(char *buffer, size_t length)
{
        if (MakeSureBufferIsSane(buffer, length) != TRUE)
                return NULL;

        if (length< strlen(DATA_DIR TELETEXT_FONT) )
                return NULL;

        strcpy(buffer, DATA_DIR TELETEXT_FONT);
        return buffer;
}


/* Returns "share/roms/" if a roms directory does not exist in "~/.beebem/roms".
 * Will not create a copy of the roms in the users config directory.
 */
char* GetLocation_roms(char *buffer, size_t length)
{
//	char filepath[MAX_PATH_LEN];
//
//	if (MakeSureBufferIsSane(buffer, length) != TRUE) return NULL;
//
//	/* Determine users location.
//	 */
//	if ( GetUserConfigPath(filepath, MAX_PATH_LEN) == NULL ){
//                qERROR("Unable to determine users config filepath.");
//        }else{
//		/* If a roms directory exists in the users config dir use that
//		 * one instead of the shared one.
//		 */
//		if ( AppendDirToPath(filepath, "roms", MAX_PATH_LEN) ){
//			if ( TestDirectory(filepath) ){
//				return CopyFilePathToBuffer(filepath
//				 , buffer, length);
//			}
//		}
//	}
//
//	return CopyFilePathToBuffer(DATA_DIR"/roms/" , buffer, length);



	if (MakeSureBufferIsSane(buffer, length) != TRUE)
		return NULL;

	/* If a "roms" directory exists in the users config dir use that
	 * one instead of the read only system one.
	 */
	if ( GetUserConfigPath(buffer, length) == NULL ){
		qERROR("Unable to determine users config filepath.");
		return NULL;
	}
	if ( AppendDirToPath(buffer, "roms", length) )
		if ( TestDirectory(buffer) ) 
			return buffer;
			
	/* OK, user has not created their own "roms" directory, so use the
	 * system (and read only) one instead.
	 */
	if ( CopyString(buffer, DATA_DIR"/roms/", length) == FALSE)
		return NULL;

	return buffer;
}


/* Max length a path (including filename) can be for target OS.
 */
#define MAX_PATH_LEN	(1024+1)

/* Return "~/.beebem/scsi", if does not exist creates it using template files
 * in "share/state/scsi/".
 */
BOOL CopyScsiImagesToUser(char *filepath)
{
	int i;
	char dst_filepath[MAX_PATH_LEN];
	char src_filepath[MAX_PATH_LEN];

#define  NUMBER_OF_FILES 9
	char files[NUMBER_OF_FILES][20]={
		"sasi0.dat",
		"scsi0.dat", "scsi0.dsc",
		"scsi1.dat", "scsi1.dsc",
		"scsi2.dat", "scsi2.dsc",
		"scsi3.dat", "scsi3.dsc"
	};

	for (i=0; i<NUMBER_OF_FILES; i++){
		strcpy(dst_filepath, filepath);
		strcat(dst_filepath, files[i]);

		/* If SCSI/SASI image does not exist, create it
		 */
		if (! TestFile(dst_filepath) ){

			strcpy(src_filepath, DATA_DIR"/media/scsi/");
			strcat(src_filepath, files[i]);
	
			/* This make be slow (especially if C copy is used) so
			 * print warning.
		 	 */
			pINFO(dL"Creating %s (may take a while).", dR
			 , files[i]);

			/* Physically copy the disk images from shared.
		 	*/
			if (CreateFileFromMaster(dst_filepath, src_filepath)
			 == FALSE){
				pERROR(dL"Failed to create '%s'.", dR
				 , dst_filepath);
			}
		}
	}

	return TRUE;
}

char* GetLocation_scsi(char *buffer, size_t length)
{
//	char filepath[MAX_PATH_LEN];
//
//	if (MakeSureBufferIsSane(buffer, length) != TRUE) return NULL;
//
//	/* Determine users location.
//	 */
//	if ( GetUserConfigPath(filepath, MAX_PATH_LEN) == NULL ){
//		qERROR("Unable to determine users config filepath.");
//		return NULL;
//	}else{
//		/* Create a filepath to the users scsi disk images.
//		 */
//		if (! AppendDirToPath(filepath, "scsi", MAX_PATH_LEN) ){
//			qERROR("Unable to determine scsi folder location!");
//			return NULL;
//		}else{
//			/* If users scsi directory does not exist, create it.
//			 */
//			if (! TestDirectory(filepath) ){
//				if (CreateDirectory(filepath) != TRUE){
//					pERROR(dL"Unable to create '%s'."
//					 , dR, filepath);
//					return NULL;
//				}
//			}
//
//			/* Create drive images (if required).
//			 */
//			if (CopyScsiImagesToUser(filepath) != TRUE){
//				qERROR("Unable to create a local copy"
//				 " of SCSI disk images!");
//				return NULL;
//			}
//
//		}
//
//	}
//	
//	return CopyFilePathToBuffer(filepath, buffer, length);



	if (MakeSureBufferIsSane(buffer, length) != TRUE)
		return NULL;

	/* Determine users location.
	 */
	if ( GetUserConfigPath(buffer, length) == NULL ){
		qERROR("Unable to determine users config filepath.");
		return NULL;
	}else{
		/* Create a filepath to the users scsi disk images.
		 */
		if (! AppendDirToPath(buffer, "scsi", length) ){
			qERROR("Unable to determine scsi folder location!");
			return NULL;
		}else{
			/* If users scsi directory does not exist, create it.
			 */
			if (! TestDirectory(buffer) ){
				if (CreateDirectory(buffer) != TRUE){
					pERROR(dL"Unable to create '%s'."
					 , dR, buffer);
					return NULL;
				}
			}

			/* Create drive images (if required).
			 */
			if (CopyScsiImagesToUser(buffer) != TRUE){
				qERROR("Unable to create a local copy"
				 " of SCSI disk images!");
				 return NULL;
			}
		}
	}

	return buffer;
}
