/*
 *  csw.cc
 *  BeebEm3
 *
 *  Created by Jon Welch on 27/08/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zlib.h"
#include "main.h"
#include "csw.h"

#include "6502core.h"
#include "uef.h"
#include "serial.h"
#include "beebsound.h"
#include "beebwin.h"
#include "debug.h"
#include "uefstate.h"

FILE *csw_file;
unsigned char file_buf[BUFFER_LEN];
unsigned char *csw_buff;
unsigned char *sourcebuff;

int csw_tonecount;
int csw_state;
int csw_datastate;
int csw_bit;
int csw_pulselen;
int csw_ptr;
unsigned long csw_bufflen;
int csw_byte;
int csw_pulsecount;
int bit_count;
unsigned char CSWOpen = 0;
int CSW_BUF;
int CSW_CYCLES;

void LoadCSW(char *file)
{
	int end;
	int sourcesize;
	
//	csw_file = fopen("/Users/jonwelch/MyProjects/csw/AticAtac.csw", "rb");

	CloseCSW();

	csw_file = fopen(file, "rb");

	if (csw_file == NULL)
	{
//->		WriteLog("Failed to open file\n");
//++
		qERROR("Failed to open file.");
//<-
		return;
	}
	
	/* Read header */
	if (fread(file_buf, 1, 0x34, csw_file) != 0x34 ||
		strncmp((const char*)file_buf, "Compressed Square Wave", 0x16) != 0 ||
		file_buf[0x16] != 0x1a)
	{
//->		WriteLog("Not a valid CSW file\n");
//++
		qERROR("Not a valid CSW file.");
//<-
		fclose(csw_file);
		return;
	}
	
//->	WriteLog("CSW version: %d.%d\n", (int)file_buf[0x17], (int)file_buf[0x18]);
//++
	pDEBUG(dL"CSW version: %d,%d", dR, (int)file_buf[0x17], (int)file_buf[0x18]);
//<-
	
	int sample_rate = file_buf[0x19] + (file_buf[0x1a] << 8) + (file_buf[0x1b] << 16) + (file_buf[0x1c] << 24);
	int total_samples = file_buf[0x1d] + (file_buf[0x1e] << 8) + (file_buf[0x1f] << 16) + (file_buf[0x20] << 24);
	int compression_type = file_buf[0x21];
	int flags = file_buf[0x22];
	unsigned int header_ext = file_buf[0x23];

//->	WriteLog("Sample rate: %d\n", sample_rate);
//--	WriteLog("Total Samples: %d\n", total_samples);
//--	WriteLog("Compressing: %d\n", compression_type);
//--	WriteLog("Flags: %x\n", flags);
//--	WriteLog("Header ext: %d\n", header_ext);
//++
	pDEBUG(dL"Sample rate: %d", dR, sample_rate);
	pDEBUG(dL"Total Samples: %d", dR, total_samples);
	pDEBUG(dL"Compressing: %d", dR, compression_type);
	pDEBUG(dL"Header ext: %d", dR, header_ext);
//<-
	
	file_buf[0x33] = 0;
//->	WriteLog("Enc appl: %s\n", &file_buf[0x24]);
//++
	pDEBUG(dL"Enc appl: %s", dR, &file_buf[0x24]);
//<-
	
	/* Read header extension bytes */
	if (fread(file_buf, 1, header_ext, csw_file) != header_ext)
	{
//->		WriteLog("Failed to read header extension\n");
//++
		qERROR("Failed to read header extension.");
//<-
		fclose(csw_file);
		return;
	}
	
    end = ftell(csw_file);
	fseek(csw_file, 0, SEEK_END);
	sourcesize = ftell(csw_file) - end + 1;
	fseek(csw_file, end, SEEK_SET);
	
	csw_bufflen = 8 * 1024 * 1024;
	csw_buff = (unsigned char *) malloc(csw_bufflen);
	sourcebuff = (unsigned char *) malloc(sourcesize);
	
	fread(sourcebuff, 1, sourcesize, csw_file);
	fclose(csw_file);
	
	uncompress(csw_buff, &csw_bufflen, sourcebuff, sourcesize);
	
	free(sourcebuff);
	
//->	WriteLog("Source Size = %d\n", sourcesize);
//--	WriteLog("Uncompressed Size = %d\n", csw_bufflen);
//++
	pDEBUG(dL"Source Size = %d.", dR, sourcesize);
	pDEBUG(dL"Uncompressed Size = %d.", dR, csw_bufflen);
//<-
	
	CSW_CYCLES = 2000000 / sample_rate - 1;
	csw_state = 0;
	csw_bit = 0;
	csw_pulselen = 0;
	csw_ptr = 0;
	csw_pulsecount = -1;
	csw_tonecount = 0;
	bit_count = -1;
	
	strcpy(UEFTapeName, file);
	
	CSWOpen = 1;
	CSW_BUF = 0;
	TxD = 0;
	RxD = 0;
	TapeClock = 0;
	OldClock = 0;
	TapeTrigger = TotalCycles + CSW_CYCLES;
}

void CloseCSW(void)
{
	if (CSWOpen) 
	{
		free(csw_buff);
		CSWOpen = 0;
		TxD = 0;
		RxD = 0;
	}
}

int csw_data(void)
{
	
	csw_pulsecount++;
	
	if (csw_buff[csw_ptr] == 0)
	{
		if (csw_ptr + 4 < (int)csw_bufflen)
		{
			csw_ptr++;
			csw_pulselen = csw_buff[csw_ptr] + (csw_buff[csw_ptr + 1] << 8) + (csw_buff[csw_ptr + 2] << 16) + (csw_buff[csw_ptr + 3] << 24);
			csw_ptr+= 4;
		}
		else
		{
			csw_pulselen = -1;
			csw_state = 0;
		}
	}
	else
	{
		csw_pulselen = csw_buff[csw_ptr++];
	}
	
	//	WriteLog("Pulse %d, duration %d\n", csw_pulsecount, csw_pulselen);
	
	return csw_pulselen;
	
}

/* Called every sample rate 44,100 Hz */

int csw_poll(int clock)
{
int ret;
	
	ret = -1;
	
	if (bit_count == -1)
	{
		bit_count = csw_data();
		if (bit_count == -1)
		{
			CloseCSW();
			return ret;
		}
	}
	
	if (bit_count > 0)
	{
		bit_count--;
		return ret;
	}
	
	//	WriteLog("csw_pulsecount %d, csw_bit %d\n", csw_pulsecount, csw_bit);
	
	switch (csw_state)
	{
		case 0 :			// Waiting for tone
			
			if (csw_pulselen < 0x0d)					/* Count tone pulses */
			{
				csw_tonecount++;
				if (csw_tonecount > 20)					/* Arbitary figure */
				{
//->					WriteLog("Detected tone at %d\n", csw_pulsecount);
//++
					pDEBUG(dL"Detected tone at %d.", dR, csw_pulsecount);
//<-
					csw_state = 1;
				}
			}
			break;
			
		case 1 :			// In tone
			
			if (csw_pulselen > 0x14)					/* Noise so reset back to wait for tone again */
			{
				csw_state = 0;
				csw_tonecount = 0;
			} else
			if ( (csw_pulselen > 0x0d) && (csw_pulselen < 0x14) )			/* Not in tone any more - data start bit */
			{
//->				WriteLog("Entered data at %d\n", csw_pulsecount);
//++
				pDEBUG(dL"Entered data at %d.", dR, csw_pulsecount);
//<-
				bit_count = csw_data();					/* Skip next half of wave */
				csw_state = 2;
				csw_bit = 0;
				csw_byte = 0;
				csw_datastate = 0;
			}
			break;
			
		case 2 :			// In data
			
			switch (csw_datastate)
			{
				case 0 :		// Receiving data
					
					bit_count = csw_data();				/* Skip next half of wave */
					csw_byte >>= 1;

					if (csw_pulselen > 0x14) 
					{
						csw_state = 0;					/* Noisy pulse so reset to tone */
						csw_tonecount = 0;
						break;
					}
						
					if (csw_pulselen <= 0x0d)
					{
						bit_count += csw_data();
						bit_count += csw_data();
						csw_byte |= 0x80;
					}
					csw_bit++;
					if (csw_bit == 8)
					{
						ret = csw_byte;
						csw_datastate = 1;				/* Stop bits */
					}
					break;
					
				case 1 :		// Stop bits
					
					bit_count = csw_data();

					if (csw_pulselen > 0x14) 
					{
						csw_state = 0;					/* Noisy pulse so reset to tone */
						csw_tonecount = 0;
						break;
					}
						
					if (csw_pulselen <= 0x0d)
					{
						bit_count += csw_data();
						bit_count += csw_data();
					}
					csw_datastate = 2;					/* tone/start bit */
					break;
					
				case 2 :
					
					if (csw_pulselen > 0x14) 
					{
						csw_state = 0;					/* Noisy pulse so reset to tone */
						csw_tonecount = 0;
						break;
					}

					if (csw_pulselen <= 0x0d)			/* Back in tone again */
					{
//->						WriteLog("Back in tone again at %d\n", csw_pulsecount);
//++
						pDEBUG(dL"Back in tone again at %d.", dR, csw_pulsecount);
//<-
						csw_state = 0;
						csw_tonecount = 0;
						csw_bit = 0;
					}
					else /* Start bit */
					{
						bit_count = csw_data();			/* Skip next half of wave */
						csw_bit = 0;
						csw_byte = 0;
						csw_datastate = 0;				/* receiving data */
					}
					break;
			}
			break;
	}
	
	bit_count += csw_data();		/* Get next bit */

	return ret;
}
