/*
 * API.h
 *
 * This module provides C callable APIs for each of the command supported by LightCrafter4500 platform and detailed in the programmer's guide.
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *
 * Added C linkages. PT
*/

#ifndef API_H
#define API_H

/* Bit masks. */
#define BIT0        0x01
#define BIT1        0x02
#define BIT2        0x04
#define BIT3        0x08
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80
#define BIT8      0x0100
#define BIT9      0x0200
#define BIT10     0x0400
#define BIT11     0x0800
#define BIT12     0x1000
#define BIT13     0x2000
#define BIT14     0x4000
#define BIT15     0x8000
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

#define STAT_BIT_FLASH_BUSY     BIT3
#define HID_MESSAGE_MAX_SIZE    512

#ifdef _WIN32
      #define API_API_EXPORT __declspec(dllexport)
      #define API_API_CALL
#else
      #define API_API_EXPORT /**< API export macro */
      #define API_API_CALL /**< API call macro */
#endif

typedef struct _hidmessageStruct
{
    struct _hidhead
    {
        struct _packetcontrolStruct
        {
            unsigned char dest		:3; /* 0 - ProjCtrl; 1 - RFC; 7 - Debugmsg */
            unsigned char reserved	:2;
            unsigned char nack		:1; /* Command Handler Error */
            unsigned char reply	:1; /* Host wants a reply from device */
            unsigned char rw		:1; /* Write = 0; Read = 1 */
        }flags;
        unsigned char seq;
        unsigned short length;
    }head;
    union
    {
        unsigned short cmd;
        unsigned char data[HID_MESSAGE_MAX_SIZE];
    }text;
}hidMessageStruct;

typedef struct _readCmdData
{
    unsigned char CMD2;
    unsigned char CMD3;
    unsigned short len;
}CmdFormat;

typedef struct _rectangle
{
    unsigned short firstPixel;
    unsigned short firstLine;
    unsigned short pixelsPerLine;
    unsigned short linesPerFrame;
}rectangle;

typedef enum
{
    SOURCE_SEL,
    PIXEL_FORMAT,
    CLK_SEL,
    CHANNEL_SWAP,
    FPD_MODE,
    CURTAIN_COLOR,
    POWER_CONTROL,
    FLIP_LONG,
    FLIP_SHORT,
    TPG_SEL,
    PWM_INVERT,
    LED_ENABLE,
    GET_VERSION,
    SW_RESET,
    DMD_PARK,
    BUFFER_FREEZE,
    STATUS_HW,
    STATUS_SYS,
    STATUS_MAIN,
    CSC_DATA,
    GAMMA_CTL,
    BC_CTL,
    PWM_ENABLE,
    PWM_SETUP,
    PWM_CAPTURE_CONFIG,
    GPIO_CONFIG,
    LED_CURRENT,
    DISP_CONFIG,
    TEMP_CONFIG,
    TEMP_READ,
    MEM_CONTROL,
    I2C_CONTROL,
    LUT_VALID,
    DISP_MODE,
    TRIG_OUT1_CTL,
    TRIG_OUT2_CTL,
    RED_STROBE_DLY,
    GRN_STROBE_DLY,
    BLU_STROBE_DLY,
    PAT_DISP_MODE,
    PAT_TRIG_MODE,
    PAT_START_STOP,
    BUFFER_SWAP,
    BUFFER_WR_DISABLE,
    CURRENT_RD_BUFFER,
    PAT_EXPO_PRD,
    INVERT_DATA,
    PAT_CONFIG,
    MBOX_ADDRESS,
    MBOX_CONTROL,
    MBOX_DATA,
    TRIG_IN1_DELAY,
    TRIG_IN2_CONTROL,
    SPLASH_LOAD,
    SPLASH_LOAD_TIMING,
    GPCLK_CONFIG,
    PULSE_GPIO_23,
    ENABLE_LCR_DEBUG,
    TPG_COLOR,
    PWM_CAPTURE_READ,
    PROG_MODE,
    BL_STATUS,
    BL_SPL_MODE,
    BL_GET_MANID,
    BL_GET_DEVID,
    BL_GET_CHKSUM,
    BL_SET_SECTADDR,
    BL_SECT_ERASE,
    BL_SET_DNLDSIZE,
    BL_DNLD_DATA,
    BL_FLASH_TYPE,
    BL_CALC_CHKSUM,
    BL_PROG_MODE,
}LCR_CMD;

extern "C" int API_API_EXPORT LCR_SetInputSource(unsigned int source, unsigned int portWidth);
extern "C" int API_API_EXPORT LCR_GetInputSource(unsigned int *pSource, unsigned int *portWidth);
extern "C" int API_API_EXPORT LCR_SetPixelFormat(unsigned int format);
extern "C" int API_API_EXPORT LCR_GetPixelFormat(unsigned int *pFormat);
extern "C" int API_API_EXPORT LCR_SetPortClock(unsigned int clock);
extern "C" int API_API_EXPORT LCR_GetPortClock(unsigned int *pClock);
extern "C" int API_API_EXPORT LCR_SetDataChannelSwap(unsigned int port, unsigned int swap);
extern "C" int API_API_EXPORT LCR_GetDataChannelSwap(unsigned int *pPort, unsigned int *pSwap);
extern "C" int API_API_EXPORT LCR_SetFPD_Mode_Field(unsigned int PixelMappingMode, bool SwapPolarity, unsigned int FieldSignalSelect);
extern "C" int API_API_EXPORT LCR_GetFPD_Mode_Field(unsigned int *pPixelMappingMode, bool *pSwapPolarity, unsigned int *pFieldSignalSelect);
extern "C" int API_API_EXPORT LCR_SetPowerMode(bool);
extern "C" int API_API_EXPORT LCR_SetLongAxisImageFlip(bool);
extern "C" bool API_API_EXPORT LCR_GetLongAxisImageFlip();
extern "C" int API_API_EXPORT LCR_SetShortAxisImageFlip(bool);
extern "C" bool API_API_EXPORT LCR_GetShortAxisImageFlip();
extern "C" int API_API_EXPORT LCR_SetTPGSelect(unsigned int pattern);
extern "C" int API_API_EXPORT LCR_GetTPGSelect(unsigned int *pPattern);
extern "C" int API_API_EXPORT LCR_SetLEDPWMInvert(bool invert);
extern "C" int API_API_EXPORT LCR_GetLEDPWMInvert(bool *inverted);
extern "C" int API_API_EXPORT LCR_SetLedEnables(bool SeqCtrl, bool Red, bool Green, bool Blue);
extern "C" int API_API_EXPORT LCR_GetLedEnables(bool *pSeqCtrl, bool *pRed, bool *pGreen, bool *pBlue);
extern "C" int API_API_EXPORT LCR_GetVersion(unsigned int *pApp_ver, unsigned int *pAPI_ver, unsigned int *pSWConfig_ver, unsigned int *pSeqConfig_ver);
extern "C" int API_API_EXPORT LCR_SoftwareReset(void);
extern "C" int API_API_EXPORT LCR_GetStatus(unsigned char *pHWStatus, unsigned char *pSysStatus, unsigned char *pMainStatus);
extern "C" int API_API_EXPORT LCR_SetPWMEnable(unsigned int channel, bool Enable);
extern "C" int API_API_EXPORT LCR_GetPWMEnable(unsigned int channel, bool *pEnable);
extern "C" int API_API_EXPORT LCR_SetPWMConfig(unsigned int channel, unsigned int pulsePeriod, unsigned int dutyCycle);
extern "C" int API_API_EXPORT LCR_GetPWMConfig(unsigned int channel, unsigned int *pPulsePeriod, unsigned int *pDutyCycle);
extern "C" int API_API_EXPORT LCR_SetPWMCaptureConfig(unsigned int channel, bool enable, unsigned int sampleRate);
extern "C" int API_API_EXPORT LCR_GetPWMCaptureConfig(unsigned int channel, bool *pEnabled, unsigned int *pSampleRate);
extern "C" int API_API_EXPORT LCR_SetGPIOConfig(unsigned int pinNum, bool enAltFunc, bool altFunc1, bool dirOutput, bool outTypeOpenDrain, bool pinState);
extern "C" int API_API_EXPORT LCR_GetGPIOConfig(unsigned int pinNum, bool *pEnAltFunc, bool *pAltFunc1, bool *pDirOutput, bool *pOutTypeOpenDrain, bool *pState);
extern "C" int API_API_EXPORT LCR_GetLedCurrents(unsigned char *pRed, unsigned char *pGreen, unsigned char *pBlue);
extern "C" int API_API_EXPORT LCR_SetLedCurrents(unsigned char RedCurrent, unsigned char GreenCurrent, unsigned char BlueCurrent);
extern "C" int API_API_EXPORT LCR_SetDisplay(rectangle croppedArea, rectangle displayArea);
extern "C" int API_API_EXPORT LCR_GetDisplay(rectangle *pCroppedArea, rectangle *pDisplayArea);
extern "C" int API_API_EXPORT LCR_MemRead(unsigned int addr, unsigned int *readWord);
extern "C" int API_API_EXPORT LCR_MemWrite(unsigned int addr, unsigned int data);
extern "C" int API_API_EXPORT LCR_ValidatePatLutData(unsigned int *pStatus);
extern "C" int API_API_EXPORT LCR_SetPatternDisplayMode(bool external);
extern "C" int API_API_EXPORT LCR_GetPatternDisplayMode(bool *external);
extern "C" int API_API_EXPORT LCR_SetTrigOutConfig(unsigned int trigOutNum, bool invert, unsigned int rising, unsigned int falling);
extern "C" int API_API_EXPORT LCR_GetTrigOutConfig(unsigned int trigOutNum, bool *pInvert,unsigned int *pRising, unsigned int *pFalling);
extern "C" int API_API_EXPORT LCR_SetRedLEDStrobeDelay(unsigned char rising, unsigned char falling);
extern "C" int API_API_EXPORT LCR_SetGreenLEDStrobeDelay(unsigned char rising, unsigned char falling);
extern "C" int API_API_EXPORT LCR_SetBlueLEDStrobeDelay(unsigned char rising, unsigned char falling);
extern "C" int API_API_EXPORT LCR_GetRedLEDStrobeDelay(unsigned char *, unsigned char *);
extern "C" int API_API_EXPORT LCR_GetGreenLEDStrobeDelay(unsigned char *, unsigned char *);
extern "C" int API_API_EXPORT LCR_GetBlueLEDStrobeDelay(unsigned char *, unsigned char *);
extern "C" int API_API_EXPORT LCR_EnterProgrammingMode(void);
extern "C" int API_API_EXPORT LCR_ExitProgrammingMode(void);
extern "C" int API_API_EXPORT LCR_GetProgrammingMode(bool *ProgMode);
extern "C" int API_API_EXPORT LCR_GetFlashManID(unsigned short *manID);
extern "C" int API_API_EXPORT LCR_GetFlashDevID(unsigned long long *devID);
extern "C" int API_API_EXPORT LCR_GetBLStatus(unsigned char *BL_Status);
extern "C" int API_API_EXPORT LCR_SetFlashAddr(unsigned int Addr);
extern "C" int API_API_EXPORT LCR_FlashSectorErase(void);
extern "C" int API_API_EXPORT LCR_SetDownloadSize(unsigned int dataLen);
extern "C" int API_API_EXPORT LCR_DownloadData(unsigned char *pByteArray, unsigned int dataLen);
extern "C" void API_API_EXPORT LCR_WaitForFlashReady(void);
extern "C" int API_API_EXPORT LCR_SetFlashType(unsigned char Type);
extern "C" int API_API_EXPORT LCR_CalculateFlashChecksum(void);
extern "C" int API_API_EXPORT LCR_GetFlashChecksum(unsigned int*checksum);
extern "C" int API_API_EXPORT LCR_SetMode(bool SLmode);
extern "C" int API_API_EXPORT LCR_GetMode(bool *pMode);
extern "C" int API_API_EXPORT LCR_LoadSplash(unsigned int index);
extern "C" int API_API_EXPORT LCR_GetSplashIndex(unsigned int *pIndex);
extern "C" int API_API_EXPORT LCR_SetTPGColor(unsigned short redFG, unsigned short greenFG, unsigned short blueFG, unsigned short redBG, unsigned short greenBG, unsigned short blueBG);
extern "C" int API_API_EXPORT LCR_GetTPGColor(unsigned short *pRedFG, unsigned short *pGreenFG, unsigned short *pBlueFG, unsigned short *pRedBG, unsigned short *pGreenBG, unsigned short *pBlueBG);
extern "C" int API_API_EXPORT LCR_ClearPatLut(void);
extern "C" int API_API_EXPORT LCR_AddToPatLut(int TrigType, int PatNum,int BitDepth,int LEDSelect,bool InvertPat, bool InsertBlack,bool BufSwap, bool trigOutPrev);
extern "C" int API_API_EXPORT LCR_GetPatLutItem(int index, int *pTrigType, int *pPatNum,int *pBitDepth,int *pLEDSelect,bool *pInvertPat, bool *pInsertBlack,bool *pBufSwap, bool *pTrigOutPrev);
extern "C" int API_API_EXPORT LCR_SendPatLut(void);
extern "C" int API_API_EXPORT LCR_SendSplashLut(unsigned char *lutEntries, unsigned int numEntries);
extern "C" int API_API_EXPORT LCR_GetPatLut(int numEntries);
extern "C" int API_API_EXPORT LCR_GetSplashLut(unsigned char *pLut, int numEntries);
extern "C" int API_API_EXPORT LCR_SetPatternTriggerMode(bool);
extern "C" int API_API_EXPORT LCR_GetPatternTriggerMode(bool *);
extern "C" int API_API_EXPORT LCR_PatternDisplay(int Action);
extern "C" int API_API_EXPORT LCR_SetPatternConfig(unsigned int numLutEntries, bool repeat, unsigned int numPatsForTrigOut2, unsigned int numSplash);
extern "C" int API_API_EXPORT LCR_GetPatternConfig(unsigned int *pNumLutEntries, bool *pRepeat, unsigned int *pNumPatsForTrigOut2, unsigned int *pNumSplash);
extern "C" int API_API_EXPORT LCR_SetExposure_FramePeriod(unsigned int exposurePeriod, unsigned int framePeriod);
extern "C" int API_API_EXPORT LCR_GetExposure_FramePeriod(unsigned int *pExposure, unsigned int *pFramePeriod);
extern "C" int API_API_EXPORT LCR_SetTrigIn1Delay(unsigned int Delay);
extern "C" int API_API_EXPORT LCR_GetTrigIn1Delay(unsigned int *pDelay);
extern "C" int API_API_EXPORT LCR_SetInvertData(bool invert);
extern "C" int API_API_EXPORT LCR_PWMCaptureRead(unsigned int channel, unsigned int *pLowPeriod, unsigned int *pHighPeriod);
extern "C" int API_API_EXPORT LCR_SetGeneralPurposeClockOutFreq(unsigned int clkId, bool enable, unsigned int clkDivider);
extern "C" int API_API_EXPORT LCR_GetGeneralPurposeClockOutFreq(unsigned int clkId, bool *pEnabled, unsigned int *pClkDivider);
extern "C" int API_API_EXPORT LCR_MeasureSplashLoadTiming(unsigned int startIndex, unsigned int numSplash);
extern "C" int API_API_EXPORT LCR_ReadSplashLoadTiming(unsigned int *pTimingData);
extern "C" int API_API_EXPORT LCR_GetGammaCorrection(unsigned char *pTable, bool *pEnable);
extern "C" int API_API_EXPORT LCR_SetGammaCorrection(unsigned char table, bool enable);
extern "C" int API_API_EXPORT LCR_GetColorSpaceConversion(unsigned char *pAttr, unsigned short *pCoefficients);

#endif // API_H
