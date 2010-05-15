/* FDC DLL stuff for BeebEm SDL (/UNIX).
 *
 * Now uses callbacks instead of DLL.
 */

#include "hardware.h"
#include "log.h"
#include "windows.h"


/* Callbacks:
 */
typedef unsigned char (*Callback_FDC_SetDriveControl)(unsigned char);
typedef unsigned char (*Callback_FDC_GetDriveControl)(unsigned char);
typedef void (*Callback_FDC_GetBoardProperties)(struct DriveControlBlock*);

Callback_FDC_SetDriveControl 	func_set_drive_control = NULL;
Callback_FDC_GetDriveControl	func_get_drive_control = NULL;
Callback_FDC_GetBoardProperties	func_get_board_properties = NULL;


/* API:
 */

unsigned char PSetDriveControl(unsigned char value)
{
	if (func_set_drive_control != NULL)
		return( func_set_drive_control(value) );
	else
		qERROR("No FDC Controller board set!");

	return(0);
}

unsigned char PGetDriveControl(unsigned char value)
{
	if (func_get_drive_control != NULL)
		return( func_get_drive_control(value) );
	else
		qERROR("No FDC Controller board set!");

	return(0);
}

void PGetBoardProperties(struct DriveControlBlock *FDBoard)
{
	if (func_get_board_properties != NULL)
		func_get_board_properties( FDBoard );
	else
		qERROR("No FDC Controller board set!");
}


/* The controllers:
 */

//-----------------------------------------------------------------------
// Acorn 1770 DFS Board Drive Controller Chip DLL
// (C) September 2001 - Richard Gellman
//-----------------------------------------------------------------------

char *AcornName="Acorn 1770 DFS Extension board for BBC Model B";

// The dll must assume the 1770 system accepts a Master 128 type control 
// reg thus:

// Bit 0 Drive Select 0
// Bit 1 Drive Select 1
// Bit 2 Reset controller
// Bit 3 Drive Select 2
// Bit 4 Side Select
// Bit 5 Density Select = 0 - double ; 1 - single
// Bits 6 & 7 not used

unsigned char AcornFDC_SetDriveControl(unsigned char value) {
        unsigned char temp;
        // from native to master 128
        temp=value & 3; // drive selects
        temp|=(value&12)<<2; // Side select and density select
        return(temp);
}

unsigned char AcornFDC_GetDriveControl(unsigned char value) {
        unsigned char temp;
        // from master 128 to native
        temp=value & 3; // drive selects
        temp|=(value&48)>>2; // side and density selects
        return(temp);
}

void AcornFDC_GetBoardProperties(struct DriveControlBlock *FDBoard) {
        FDBoard->FDCAddress=0xfe84;
        FDBoard->DCAddress=0xfe80;
        FDBoard->BoardName=AcornName;
        FDBoard->TR00_ActiveHigh=FALSE;
}

void SetAs_AcornFDC(void)
{
	func_set_drive_control = AcornFDC_SetDriveControl;
	func_get_drive_control = AcornFDC_GetDriveControl;
	func_get_board_properties = AcornFDC_GetBoardProperties;	
}


//-----------------------------------------------------------------------
// Watford DDFS Board Drive Controller Chip DLL
// (C) September 2001 - Richard Gellman
//-----------------------------------------------------------------------

char *WatfordName="Watford DDFS Extension board for BBC Model B";

// The dll must assume the 1770 system accepts a Master 128 type control reg thus:
// Bit 0 Drive Select 0
// Bit 1 Drive Select 1
// Bit 2 Reset controller
// Bit 3 Drive Select 2
// Bit 4 Side Select
// Bit 5 Density Select = 0 - double ; 1 - single
// Bits 6 & 7 not used

unsigned char WatfordFDC_SetDriveControl(unsigned char value) {
        unsigned char temp;
        // from native to master 128
        if (value & 4) temp=2; else temp=1; // drive select
        temp|=(value & 2)<<3; // side select
        temp|=(value & 1)<<5; // density select
        return(temp);
}

unsigned char WatfordFDC_GetDriveControl(unsigned char value) {
	unsigned char temp=0;
        // from master 128 to native
        if (value & 1) temp=0; // Drive select 0
        if (value & 2) temp=4; // Drive select 1
        temp|=(value & 16)>>3; // Side select
        temp|=(value & 32)>>5;
        return(temp);
}

void WatfordFDC_GetBoardProperties(struct DriveControlBlock *FDBoard) {
        FDBoard->FDCAddress=0xfe84;
        FDBoard->DCAddress=0xfe80;
        FDBoard->BoardName=WatfordName;
        FDBoard->TR00_ActiveHigh=TRUE;
}

void SetAs_WatfordFDC(void)
{
	func_set_drive_control = WatfordFDC_SetDriveControl;
	func_get_drive_control = WatfordFDC_GetDriveControl;
	func_get_board_properties = WatfordFDC_GetBoardProperties;
}


//-----------------------------------------------------------------------
// Opus DDFS Board Drive Controller Chip DLL
// (C) September 2001 - Richard Gellman
//-----------------------------------------------------------------------

char *OpusName="Opus DDOS Extension board for BBC Model B";

// The dll must assume the 1770 system accepts a Master 128 type control reg thus:
// Bit 0 Drive Select 0
// Bit 1 Drive Select 1
// Bit 2 Reset controller
// Bit 3 Drive Select 2
// Bit 4 Side Select
// Bit 5 Density Select = 0 - double ; 1 - single
// Bits 6 & 7 not used

unsigned char OpusFDC_SetDriveControl(unsigned char value) {
        unsigned char temp;
        // from native to master 128
        temp=(value & 2)<<3; // Side Select
        temp|=((value^255) & 64)>>1; // Density Select
        if (value & 1) temp|=2; else temp|=1; // Drive select
        return(temp);
}

unsigned char OpusFDC_GetDriveControl(unsigned char value) {
        unsigned char temp;
        // from master 128 to native
        temp=((value^255) & 32)<<1; // Density select
        temp|=(value & 16)>>3; // Side select
        if (value & 2) temp|=1; // Drive Select;
        return(temp);
}

void Opus_FDC_GetBoardProperties(struct DriveControlBlock *FDBoard) {
        FDBoard->FDCAddress=0xfe80;
        FDBoard->DCAddress=0xfe84;
        FDBoard->BoardName=OpusName;
        FDBoard->TR00_ActiveHigh=FALSE;
}

void SetAs_OpusFDC(void)
{
	func_set_drive_control = OpusFDC_SetDriveControl;
	func_get_drive_control = OpusFDC_GetDriveControl;
	func_get_board_properties = Opus_FDC_GetBoardProperties;
}

