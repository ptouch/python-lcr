/*
 * firmware.cpp
 *
 * This module handles building and parsing of firmware images
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
*/

#include "firmware.h"
#include "Common.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
#else
    typedef short WORD;
    typedef long LONG;
    typedef uint32 DWORD;
    typedef struct tagBITMAPFILEHEADER {
        WORD bfType;
        WORD bfSize;
        WORD bfReserved1;
        WORD bfReserved2;
        WORD bfOffBits;
    } BITMAPFILEHEADER, *PBITMAPFILEHEADER;
    typedef struct tagBITMAPINFOHEADER {
        DWORD biSize;
        LONG  biWidth;
        LONG  biHeight;
        WORD  biPlanes;
        WORD  biBitCount;
        DWORD biCompression;
        DWORD biSizeImage;
        LONG  biXPelsPerMeter;
        LONG  biYPelsPerMeter;
        DWORD biClrUsed;
        DWORD biClrImportant;
    } BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#endif

uint32 FLASH_TABLE_ADDRESS = 0x00020000;

unsigned char *pFrmwImageArray, *splBuffer;
uint32 splash_index, splash_data_start_flash_address, appl_config_data_start_address;
int splash_count;

#define FLASH_THREE_ADDRESS					0xFB000000	// actually it is re map to 0xF8000000
#define FLASH_TWO_ADDRESS					0xFA000000
#define FLASH_BASE_ADDRESS					0xF9000000

uint32 ChipSelectSize[3] = {0x01000000, 0x01000000, 0x01000000};
uint32 ChipSelectEnd[3]  = {0xFC000000, 0xFA000000, 0xFB000000};
uint32 ChipSelectBase[3] = {0xFB000000, 0xF9000000, 0xFA000000};

static int SPLASH_PerformLineCompression(unsigned char *SourceAddr, int ImageWidth, int ImageHeight, uint32 *compressed_size, uint8 numLines)
{
        uint16 Row, Col;
	uint32 lineLength, bytesPerPixel = 3, imageHeight, pixelIndex;
	unsigned char *line1Data, *line2Data;

	lineLength =  ImageWidth * bytesPerPixel;

	if(lineLength % 4 != 0)
    	{
		lineLength = (lineLength / 4 + 1) * 4;
    	}

	if(numLines > 8)
	{
		*compressed_size = 0xFFFFFFFF;
		return 1;
	}

	imageHeight = (ImageHeight / numLines) * numLines;

        for (Row = 0; Row < (imageHeight - numLines); Row++)
        {
        line1Data = (unsigned char *)(SourceAddr + (lineLength *  Row));
        line2Data = (unsigned char *)(SourceAddr + (lineLength * (Row + numLines)));

        	for (Col = 0; Col < ImageWidth; Col++)
        	{
			pixelIndex = Col * bytesPerPixel;

			if( (line1Data[pixelIndex  ] != line2Data[pixelIndex  ]) || 
				(line1Data[pixelIndex+1] != line2Data[pixelIndex+1]) || 
				(line1Data[pixelIndex+2] != line2Data[pixelIndex+2]))
			{
				*compressed_size = 0xFFFFFFFF;
				return 1;
			}
		}
    }

    *compressed_size = numLines * lineLength;	
    return 0;
}

static int SPLASH_PerformRLECompression(unsigned char *SourceAddr, unsigned char *DestinationAddr, int ImageWidth, int ImageHeight, uint32 *compressed_size)
{
    uint16 Row, Col;
    BOOL   FirstPixel = TRUE;
    uint8 Repeat = 1;
    uint32 Pixel = 0, S, D;
    uint32 LastColor = 0, pad;                 
    uint8 count = 0;

//    uint8 *SourceAddr		= (uint08 *)(SplashRLECfg->SourceAddr);
//    uint8 *DestinationAddr = (uint08 *)(SplashRLECfg->DestinationAddr);
    uint32 PixelSize		= 3;
    
    S = 0;
    D = 0;

    /* RLE Encode the Splash Image*/
    for (Row = 0; Row < ImageHeight; Row++)
    {
        for (Col = 0; Col < ImageWidth; Col++)
        {

	    memcpy(&Pixel, SourceAddr + S, PixelSize);

            /* if this is the first Pixel, remember it and move on... */
            if (FirstPixel)
            {
                LastColor  = Pixel;
                Repeat  = 1;
                FirstPixel = FALSE;
                count = 0;
            }
            else
            {
                if (Pixel == LastColor)
                {
                    if (count)
                    {
						if(count > 1) DestinationAddr[D++] = 0;
						DestinationAddr[D++] = count;
						memcpy(DestinationAddr + D, SourceAddr + (S - ((count + 1) * PixelSize)) , count * PixelSize);
						D += (count * PixelSize);
                        count = 0;
                    }

                    Repeat++;

                    if (Repeat == 255)
                    {
						DestinationAddr[D++] = Repeat;
						memcpy(DestinationAddr + D, &LastColor, PixelSize);
						D += PixelSize;
                        FirstPixel = TRUE;
                    }
                }
                else
                {
                    if (Repeat == 1)
                    {
                        count++;
                        if (count == 255)
                        {
							if(count > 1) DestinationAddr[D++] = 0;
							DestinationAddr[D++] = count;
							memcpy(DestinationAddr + D, SourceAddr + (S - ((count + 1) * PixelSize)), count * PixelSize);
                            D += (count * PixelSize);
                            count = 0;
                        }
                    }
                    else
                    {
                        DestinationAddr[D++] = Repeat;
						memcpy(DestinationAddr + D, &LastColor, PixelSize);
						D += PixelSize;
                        Repeat = 1;
                    }
                    LastColor = Pixel;
                }
            }

            /* Last Pixel of the line*/
            if (Col == (ImageWidth-1) && Repeat != 255 && count != 255)
            {
                if (count)
                {
					DestinationAddr[D++] = 0;
					DestinationAddr[D++] = count + 1;
					memcpy(DestinationAddr + D, SourceAddr + (S - ((count + 2) * PixelSize)), (count + 1) * PixelSize);
                    D += ((count + 1) * PixelSize);
                    count = 0;
                }
                else
                {
					DestinationAddr[D++] = Repeat;
					memcpy(DestinationAddr + D, &LastColor, PixelSize);
					D += PixelSize;
                }
                FirstPixel = TRUE;
            }
            S += PixelSize;
	    
        }
		// END OF LINE
		DestinationAddr[D++] = 0;
		DestinationAddr[D++] = 0;

        /* Scan lines are always padded out to next 32-bit boundary */
		if(D % 4 != 0)
		{
			pad = 4 - (D % 4);
			memset(DestinationAddr + D, 0, pad);
			D += pad;
		}
    }

    /* End of file: Control Byte = 0 & Color Byte = 1 */
	DestinationAddr[D++] = 0;
	DestinationAddr[D++] = 1;

    /* End of file should be padded out till 128-bit boundary */
	if(D % 16 != 0)
	{
		pad = 16 - (D % 16);
		memset(DestinationAddr + D, 0, pad);
		D += pad;
	}

    // update flash size
    *compressed_size = D;

    return 0;
}

static int SPLASH_PerformRLEUnCompression(unsigned char *SourceAddr, unsigned char *DestinationAddr, uint32 *size)
{
	uint32 PixelSize= 3, S = 0, D = 0;
	int i;

	while (S < *size)
	{
		uint32 ctrl_byte, color_byte;

		ctrl_byte = SourceAddr[S];
		color_byte = SourceAddr[S + 1];
		if (ctrl_byte == 0)
		{  
			if (color_byte == 1)	// End of image
				break;
			else if (color_byte == 0)	// End of Line.
			{
				i = 0;
				S +=2;
				if (S % 4 != 0)
				{
					int pad = 4 - (S % 4);
					S += pad;
				}
			}
			else if (color_byte >= 2)
			{
				S +=2;
				memcpy(DestinationAddr + D, SourceAddr + S, color_byte * PixelSize);
				D += color_byte * PixelSize;
				S += color_byte * PixelSize;
			}
			else
				return -1;
		}
		else if (ctrl_byte > 0)
		{
			S++;
			for (i = 0; i < ctrl_byte; i++)
			{
				memcpy(DestinationAddr + D, SourceAddr + S, PixelSize);
				D += PixelSize;
			}
			S += PixelSize;
		}
		else
			return -1;
	}
	*size = D;
	return 0;
}

int Frmw_CopyAndVerifyImage(const unsigned char *pByteArray, int size)
{
	FLASH_TABLE *flash_table, *flash_table_temp = NULL;

	if (pFrmwImageArray != NULL)
	{
		free(pFrmwImageArray);
		pFrmwImageArray = NULL;
		splash_data_start_flash_address = 0;
	}

	
    	pFrmwImageArray = (unsigned char *)malloc(size);
	if (pFrmwImageArray == NULL)
		return ERROR_NO_MEM_FOR_MALLOC;

	memcpy(pFrmwImageArray, pByteArray, size);

	flash_table = (FLASH_TABLE *)(pFrmwImageArray + FLASH_TABLE_ADDRESS);
	flash_table_temp = (FLASH_TABLE *)(pByteArray + FLASH_TABLE_ADDRESS);

	if(flash_table->Signature != FLASHTABLE_APP_SIGNATURE)
	{
		FLASH_TABLE_ADDRESS = 0x00008000;
		
		flash_table = (FLASH_TABLE *)(pFrmwImageArray + FLASH_TABLE_ADDRESS);
		
		if(flash_table->Signature != FLASHTABLE_APP_SIGNATURE)
		{
			FLASH_TABLE_ADDRESS = 0x00020000;
			
			flash_table = (FLASH_TABLE *)(pFrmwImageArray + FLASH_TABLE_ADDRESS);
			
			if(flash_table->Signature != FLASHTABLE_APP_SIGNATURE)
				return ERROR_FRMW_FLASH_TABLE_SIGN_MISMATCH;
		}
	}
	splash_data_start_flash_address = flash_table->Splash_Data[FLASH_TABLE_SPLASH_INDEX].Address;
	appl_config_data_start_address = flash_table->APPL_Config_Data[0].Address;
	
	return 0;
}

uint32 Frmw_GetVersionNumber()
{
	uint32 version_number;
	
	memcpy(&version_number, pFrmwImageArray + (appl_config_data_start_address - FLASH_BASE_ADDRESS), sizeof(version_number));

	return version_number;
}

int Frmw_GetSplashCount()
{
	uint32 splash_data_start_address = splash_data_start_flash_address - FLASH_BASE_ADDRESS;
	SPLASH_SUPER_BINARY_INFO binary_info;

	memcpy(&binary_info, pFrmwImageArray + splash_data_start_address, sizeof(binary_info));

	/* The GUI supports pattern image display for firmware images built with DLPC350_CONFIG.exe alone */
	if ((binary_info.Sig1 == 0x12345678) && (binary_info.Sig2 == 0x87654321))
		return binary_info.BlobCount;
	else
		return -1;
}

uint32 Frmw_GetSPlashFlashStartAddress()
{
	return splash_data_start_flash_address;
}

int Frmw_GetSpashImage(unsigned char *pImageBuffer, int index)
{
	FLASH_TABLE *flash_table;
	SPLASH_BLOB_INFO blob_info;
	uint32 blob_address, splash_image_address, splash_image_size;
	SPLASH_HEADER splash_header;
	unsigned char *image_buffer, *lineData;
	int i, j, lineLength;
	uint32 splash_data_start_address = splash_data_start_flash_address - FLASH_BASE_ADDRESS;

	blob_address = splash_data_start_address + sizeof(SPLASH_SUPER_BINARY_INFO) + index * sizeof(blobinfo);
	memcpy(&blob_info, pFrmwImageArray + blob_address, sizeof(blob_info));

	if (blob_info.BlobOffset == 0xffffffff)
		return ERROR_NO_SPLASH_IMAGE;

	if(blob_info.BlobOffset < FLASH_BASE_ADDRESS)
	{
		blob_info.BlobOffset += 0x03000000;
	}
	
	splash_image_address = blob_info.BlobOffset - FLASH_BASE_ADDRESS;
	memcpy(&splash_header, pFrmwImageArray + splash_image_address, sizeof(splash_header));
	splash_image_address += sizeof(splash_header);

	splash_image_size = blob_info.BlobSize - sizeof(splash_header);

	if (splash_header.Compression == SPLASH_4LINE_COMPRESSION)
	{
		unsigned char *fourLine_buffer;

		fourLine_buffer = (unsigned char *)malloc(splash_image_size);
		if(fourLine_buffer == NULL)
			return ERROR_NO_MEM_FOR_MALLOC;	

		memcpy(fourLine_buffer, pFrmwImageArray + splash_image_address, splash_image_size);

		image_buffer =  (unsigned char *)malloc(splash_header.Image_width * splash_header.Image_height * 3); // We only support 24 bit format.
		if(image_buffer == NULL)
			return ERROR_NO_MEM_FOR_MALLOC;

		for (i = 0; i < splash_header.Image_height; i+=4)
			memcpy((image_buffer + (splash_header.Image_width * 3 * i)), fourLine_buffer, splash_image_size);

		splash_image_size = splash_header.Image_width * splash_header.Image_height * 3;
	}
	else if (splash_header.Compression == SPLASH_RLE_COMPRESSION)
	{
		unsigned char* rle_buffer;

		rle_buffer = (unsigned char *)malloc(splash_image_size);
		if(rle_buffer == NULL)
			return ERROR_NO_MEM_FOR_MALLOC;
		
		image_buffer =  (unsigned char *)malloc(splash_header.Image_width * splash_header.Image_height * 3); // We only support 24 bit format.
		if(image_buffer == NULL)
			return ERROR_NO_MEM_FOR_MALLOC;

		memcpy(rle_buffer, pFrmwImageArray + splash_image_address, splash_image_size);

		SPLASH_PerformRLEUnCompression(rle_buffer, image_buffer, &splash_image_size);
		free(rle_buffer);
	}
	else if (splash_header.Compression == SPLASH_UNCOMPRESSED)
	{
		image_buffer = (unsigned char *)malloc(splash_image_size);
		if(image_buffer == NULL)
			return ERROR_NO_MEM_FOR_MALLOC;
		
		memcpy(image_buffer, pFrmwImageArray + splash_image_address, splash_image_size);
	}

	lineLength = splash_header.Image_width * 3;

	lineData = (unsigned char *) malloc(lineLength);
	for (i = 0; i < splash_header.Image_height; i++)
	{
		memcpy(lineData, image_buffer + (lineLength * i), lineLength);
		for (j = 0;j < splash_header.Image_width; j++)
		{
			unsigned char tempByte;

			tempByte = lineData[j * 3 + 2];
			lineData[j * 3 + 2] = lineData[j * 3 + 1];
			lineData[j * 3 + 1] = tempByte;
		}
		memcpy(image_buffer + (lineLength * i), lineData, lineLength);
	}

	free(lineData);
	memcpy(pImageBuffer, image_buffer, splash_image_size);
	free(image_buffer);
	
	return 0;
}

int Frmw_SPLASH_InitBuffer(int numSplash)
{
	SPLASH_SUPER_BINARY_INFO	binary_info;
	SPLASH_BLOB_INFO		blob_info;
	uint32 i;

	if(splBuffer != NULL)
	{
		free(splBuffer);
		splBuffer = NULL;
	}
	splash_index = 0;
	splash_count = 0;
	
	binary_info.Sig1 = 0x12345678;
	binary_info.Sig2 = 0x87654321;
	binary_info.BlobCount = numSplash;

	blob_info.BlobOffset = 0xFFFFFFFF;
	blob_info.BlobSize   = 0xFFFFFFFF;

	splBuffer = (unsigned char *) malloc(sizeof(binary_info) + (sizeof(blob_info) * numSplash));

	memcpy(splBuffer + splash_index, &binary_info, sizeof(binary_info));
	splash_index += sizeof(binary_info);

	for(i = 0; i < numSplash; i++)
	{
		memcpy(splBuffer + splash_index, &blob_info, sizeof(blob_info));
		splash_index += sizeof(blob_info);
	}
	return 0;
}

int Frmw_SPLASH_AddSplash(unsigned char *pImageBuffer, uint8 *compression, uint32 *compSize)
{
	BITMAPFILEHEADER fileHeader;  
	BITMAPINFOHEADER headerInfo;
	unsigned char *bitmapImage, *line1Data, *line2Data, *splashImage, *rle_buffer;
	int lineLength, bytesPerPixel, i, j;
	uint32 splashSize;
	SPLASH_HEADER splash_header;
	SPLASH_BLOB_INFO *blob_info;
	
	if((!splBuffer || !splash_data_start_flash_address))
		return ERROR_INIT_NOT_DONE_PROPERLY;

	memcpy(&fileHeader, pImageBuffer, sizeof(fileHeader));
	memcpy(&headerInfo, pImageBuffer + sizeof(fileHeader), sizeof(headerInfo));

	if (fileHeader.bfType != 0x4D42)  
        	return ERROR_NOT_BMP_FILE;
	if(headerInfo.biBitCount != 24)// && headerInfo.biBitCount != 32)
		return ERROR_NOT_24bit_BMP_FILE;

	bitmapImage = (unsigned char *)malloc(fileHeader.bfSize - fileHeader.bfOffBits);
	if (!bitmapImage)
		return ERROR_NO_MEM_FOR_MALLOC;

	memcpy(bitmapImage, pImageBuffer + fileHeader.bfOffBits, fileHeader.bfSize - fileHeader.bfOffBits);

	bytesPerPixel = headerInfo.biBitCount / 8;

        lineLength    =  headerInfo.biWidth * bytesPerPixel;

	if(lineLength % 4 != 0)
        {
		lineLength = (lineLength / 4 + 1) * 4;
        }
	line1Data = (unsigned char *)malloc(lineLength);

	if(line1Data == NULL)
	{
	    free(bitmapImage);
		return ERROR_NO_MEM_FOR_MALLOC;
	}

	line2Data = (unsigned char *)malloc(lineLength);

	if(line2Data == NULL)
	{
	    free(line1Data);
	    free(bitmapImage);
		return ERROR_NO_MEM_FOR_MALLOC;
	}

	// vertically flip the bitmap image
	for(i = 0; i < (headerInfo.biHeight / 2); i++)
	{
        memcpy(line1Data, bitmapImage + (lineLength * i), lineLength);
        memcpy(line2Data, bitmapImage + (lineLength * (headerInfo.biHeight - i - 1)), lineLength);

	unsigned char tempbyte;

	for(j = 0; j < headerInfo.biWidth; j++)
	{
            
	    tempbyte = line1Data[j * 3 + 2];
            line1Data[j * 3 + 2] = line1Data[j * 3 + 1];
            line1Data[j * 3 + 1] = tempbyte;
                        
            tempbyte = line2Data[j * 3 + 2];
            line2Data[j * 3 + 2] = line2Data[j * 3 + 1];
            line2Data[j * 3 + 1] = tempbyte;
	}

        memcpy(bitmapImage + (lineLength * (headerInfo.biHeight - i - 1)), line1Data, lineLength);
        memcpy(bitmapImage + (lineLength * i), line2Data, lineLength);
	}

	free(line1Data); 
	free(line2Data);
	
	unsigned char *rleBuffer = (unsigned char *)malloc((((headerInfo.biHeight * headerInfo.biWidth * bytesPerPixel) + 
							  ((headerInfo.biHeight * headerInfo.biWidth * 4) / 255) + (headerInfo.biWidth * 2) + 15) - 1));

	if (rleBuffer == NULL)
	{
		free(bitmapImage);
		return ERROR_NO_MEM_FOR_MALLOC;
	}
	
	switch(*compression)
	{
	case 0: // force uncompress
		splashSize  = headerInfo.biHeight * lineLength;
		splashImage = bitmapImage;
		break;

	case 1: // force rle compress
		SPLASH_PerformRLECompression(bitmapImage, rleBuffer, headerInfo.biWidth, headerInfo.biHeight, &splashSize);
		splashImage = rleBuffer;
		break;
	
	case 4: // force 4 line compress
		splashSize  = 4 * lineLength;
		splashImage = bitmapImage; 
		break;
	
	default: // auto compression
		uint32 lineCompSize, rleCompSize;

		SPLASH_PerformLineCompression(bitmapImage, headerInfo.biWidth, headerInfo.biHeight, &lineCompSize, 4);
		SPLASH_PerformRLECompression(bitmapImage, rleBuffer, headerInfo.biWidth, headerInfo.biHeight, &rleCompSize);

		splashSize  = headerInfo.biHeight * lineLength;

		if(lineCompSize < splashSize)
		{
			splashSize  = 4 * lineLength;
			splashImage = bitmapImage;
			*compression = 4;
		}
		else if(rleCompSize < splashSize)
		{
			splashSize  = rleCompSize;
			splashImage = rleBuffer; 
			*compression    = 1;
		}
		else
		{
			splashSize  = headerInfo.biHeight * lineLength;
			splashImage = bitmapImage; 
			*compression    = 0;
		}

		break;
	}
	
	splash_header.Signature		= 0x636C7053;
	splash_header.Image_width	= (uint16)headerInfo.biWidth;
	splash_header.Image_height	= (uint16)headerInfo.biHeight;
	splash_header.Pixel_format	= 1; // 24-bit packed
	splash_header.Subimg_offset 	= -1;
	splash_header.Subimg_end	= -1;
	splash_header.Bg_color		= 0;
	splash_header.ByteOrder		= 1;
	splash_header.ChromaOrder	= 0;
	splash_header.Byte_count	= splashSize;
	splash_header.Compression	= *compression;

	blob_info = (SPLASH_BLOB_INFO *)(splBuffer + sizeof(SPLASH_SUPER_BINARY_INFO) + (splash_count * sizeof(SPLASH_BLOB_INFO)));

	uint32 FlashEnd, currChipSelect, nextChipSelect;

	if(ChipSelectSize[0] != 0)
		FlashEnd = FLASH_THREE_ADDRESS + ChipSelectSize[0];
	else if(ChipSelectSize[2] != 0)
		FlashEnd = FLASH_TWO_ADDRESS   + ChipSelectSize[2];
	else if(ChipSelectSize[1] != 0)
		FlashEnd = FLASH_BASE_ADDRESS  + ChipSelectSize[1];

	ChipSelectEnd[0] = FLASH_THREE_ADDRESS + ChipSelectSize[0];
	ChipSelectEnd[1] = FLASH_BASE_ADDRESS  + ChipSelectSize[1];
	ChipSelectEnd[2] = FLASH_TWO_ADDRESS   + ChipSelectSize[2];

	if((splash_index + splash_data_start_flash_address + sizeof(splash_header) + splashSize) < FlashEnd)
	{
		nextChipSelect = -1;
		if((splash_index + splash_data_start_flash_address) < ChipSelectEnd[1] && 
		(splash_index + splash_data_start_flash_address + sizeof(splash_header) + splashSize) > ChipSelectEnd[1] &&
		(ChipSelectSize[1] != 0x01000000))
		{
			currChipSelect = 1;
	
			if(ChipSelectSize[2] != 0)
				nextChipSelect = 2;
			else if(ChipSelectSize[0] != 0)
				nextChipSelect = 0;
		}
		
		if((splash_index + splash_data_start_flash_address) < ChipSelectEnd[2] && 
		(splash_index + splash_data_start_flash_address + sizeof(splash_header) + splashSize) > ChipSelectEnd[2])
		{
			currChipSelect = 2;
			nextChipSelect = 0;
		}

		if(nextChipSelect != -1)
		{
            		printf("OVERFLOW FLASH_CS%d, MOVING SPLASH [%d] DATA TO FLASH_CS%d \n", currChipSelect, splash_count, nextChipSelect);
			printf("BYTES UNUSED IN FLASH_CS%d = 0x%08X <%d> bytes\n", currChipSelect, ChipSelectEnd[currChipSelect] -
						splash_data_start_flash_address - splash_index, ChipSelectEnd[currChipSelect] - splash_data_start_flash_address - splash_index);

			splBuffer = (unsigned char *)realloc(splBuffer, ChipSelectBase[nextChipSelect] - splash_data_start_flash_address);

			memset(splBuffer + splash_index, 0xFF, ChipSelectBase[nextChipSelect] - splash_data_start_flash_address - splash_index);

			splash_index = ChipSelectBase[nextChipSelect] - splash_data_start_flash_address;

			blob_info = (SPLASH_BLOB_INFO *)(splBuffer + sizeof(SPLASH_SUPER_BINARY_INFO) + (splash_count * sizeof(SPLASH_BLOB_INFO)));
		}
		blob_info->BlobOffset = splash_index + splash_data_start_flash_address;
		blob_info->BlobSize   = sizeof(splash_header) + splashSize;

		/* check if it is crossing the 3rd chipselect, if yes remap it to 0th chip select */
		if(blob_info->BlobOffset >= FLASH_THREE_ADDRESS)
		{
			blob_info->BlobOffset -= 0x03000000;
		}

		splBuffer = (unsigned char *)realloc(splBuffer, splash_index + sizeof(splash_header) + splashSize);

		memcpy(splBuffer + splash_index, &splash_header, sizeof(splash_header));
		
		splash_index += sizeof(splash_header);

		memcpy(splBuffer + splash_index, splashImage, splashSize);

		splash_index += splashSize;

		splash_count++;
	}
	else
	{
		splash_count++;

		printf("NO SPACE LEFT IN THE FLASH CAN'T WRITE SPLASH [%d]\n", splash_count);
	}

	free(rleBuffer);
	free(bitmapImage);
	*compSize = splashSize;
	return 0;
}

void Frmw_Get_NewFlashImage(unsigned char **newFrmwbuffer, uint32 *newFrmwsize)
{
	uint32 newfrmFileInLen = (splash_data_start_flash_address - FLASH_BASE_ADDRESS) + splash_index;

	pFrmwImageArray	= (unsigned char *)realloc(pFrmwImageArray, newfrmFileInLen);
	memcpy(pFrmwImageArray + (splash_data_start_flash_address - FLASH_BASE_ADDRESS), splBuffer, splash_index);
	
	*newFrmwbuffer = pFrmwImageArray;
	*newFrmwsize = newfrmFileInLen;
}

void Frmw_Get_NewSplashBuffer(unsigned char **newSplashBuffer, uint32 *newSplashSize)
{
	*newSplashBuffer = splBuffer;
	*newSplashSize = splash_index;
}

void Frmw_UpdateFlashTableSplashAddress(unsigned char *flashTableSectorBuffer, uint32 address_offset)
{
	FLASH_TABLE *flash_table;
	unsigned char *temp_flashTableSector = (unsigned char*) malloc(128 * 1024);

	splash_data_start_flash_address = address_offset + FLASH_BASE_ADDRESS;
	memcpy(temp_flashTableSector, pFrmwImageArray + FLASH_TABLE_ADDRESS, 128 * 1024);
	flash_table = (FLASH_TABLE *)(temp_flashTableSector);

	flash_table->Splash_Data[FLASH_TABLE_SPLASH_INDEX].Address = address_offset + FLASH_BASE_ADDRESS;
	memcpy(flashTableSectorBuffer, temp_flashTableSector, 128 * 1024);

	free(temp_flashTableSector);
}
