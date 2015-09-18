/*
 * BMPParser.h
 *
 * This module handles parsing BMP Image content
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
*/

#ifndef BMPPARSER_H_
#define BMPPARSER_H_

#include "Error.h"
#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint32 Width;
	uint32 Height;
	uint16 NumColors;
	uint8 BitDepth;
} BMP_Image_t;

typedef ErrorCode_t (BMP_DataFunc_t)(void *Param, uint8 *Data, uint32 Size);
typedef ErrorCode_t (BMP_PixelFunc_t)(void *Param, uint32 X, uint32 Y, 
													uint8 *PixValue, uint32 Count);

ErrorCode_t BMP_ParseImage(BMP_DataFunc_t *GetData, void *DataParam,
                                        BMP_PixelFunc_t *DrawPixels, void *DrawParam,
										uint8 OutBitDepth);

ErrorCode_t BMP_InitImage(BMP_Image_t *Image, uint32 Width, uint32 Height, uint8 BitDepth);

ErrorCode_t BMP_StoreImage(BMP_Image_t *Image, BMP_DataFunc_t *PutData, void *DataParam,
                                        BMP_PixelFunc_t *GetPixels, void *PixelParam);

uint32 BMP_ImageSize(BMP_Image_t *Image);



#ifdef __cplusplus
}
#endif

#endif /* BMPPARSER_H_ */
