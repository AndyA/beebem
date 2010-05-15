#ifndef _HARDWARE_H_
#define _HARDWARE_H_

// Opus DDFS Board Drive Controller Chip
// (C) September 2001 - Richard Gellman

struct DriveControlBlock {
        int FDCAddress; // 1770 FDC chip address
        int DCAddress; // Drive Control Register Address
        char *BoardName; // FDC Board name
        bool TR00_ActiveHigh; // Set TRUE if the TR00 input is Active High
};

#define EDCB struct DriveControlBlock

unsigned char PSetDriveControl(unsigned char value);
unsigned char PGetDriveControl(unsigned char value);
void PGetBoardProperties(struct DriveControlBlock *FDBoard);

void SetAs_AcornFDC(void);
void SetAs_WatfordFDC(void);
void SetAs_OpusFDC(void);

#endif
