/*
 * Config.h
 *
 * This module provides all the system level configurations
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
*/

#ifndef CONFIG_H_
#define CONFIG_H_

#define CFG_TCP_CMD_PORT        0x5555
#define CFG_DEBUG_LEVEL			DEBUG_LEVEL_MSG

#ifdef BUILD_WVGA
#define CFG_DMD_WIDTH           608
#define CFG_DMD_HEIGHT          684
#endif
#ifdef BUILD_NHD
#define CFG_DMD_WIDTH           640	
#define CFG_DMD_HEIGHT          360
#endif

#define CFG_CAMERA_WIDTH		1280
#define CFG_CAMERA_HEIGHT		1024
#define CFG_EEPROM_DEVICE		"24C02"
#define CFG_KEY_POLING_PERIOD	250000
#define CFG_HBEAT_PERIOD		500000
#define CFG_DEFAULT_IP			"\xC0\xA8\x01\x64"


#endif /* CONFIG_H_ */
