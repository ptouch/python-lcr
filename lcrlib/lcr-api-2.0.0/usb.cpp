/*
 * usb.cpp
 *
 * This module has the wrapper functions to access USB driver functions.
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *
 * Added C linkages. PT
*/


// #include "mainwindow.h"
// #include "ui_mainwindow.h"
// #ifdef Q_OS_WIN32
// #include <setupapi.h>
// #endif
// #include <QMessageBox>
// #include <QTimer>
#include "usb.h"
#include <stdio.h>
#include <stdlib.h>
#include "hidapi-master/hidapi/hidapi.h"

/***************************************************
*                  GLOBAL VARIABLES
****************************************************/
static hid_device *DeviceHandle;	//Handle to write
//In/Out buffers equal to HID endpoint size + 1
//First byte is for Windows internal use and it is always 0
unsigned char OutputBuffer[USB_MAX_PACKET_SIZE+1];
unsigned char InputBuffer[USB_MAX_PACKET_SIZE+1];

static bool USBConnected = false;      //Boolean true when device is connected

extern "C" bool USB_IsConnected()
{
    return USBConnected;
}

extern "C" int USB_Init(void)
{
    return hid_init();
}

extern "C" int USB_Exit(void)
{
    return hid_exit();
}

extern "C" int USB_Open()
{
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    DeviceHandle = hid_open(MY_VID, MY_PID, NULL);

    if(DeviceHandle == NULL)
    {
        USBConnected = false;
        return -1;
    }
    USBConnected = true;
    return 0;
}

extern "C" int USB_Write()
{
    if(DeviceHandle == NULL)
        return -1;

    return hid_write(DeviceHandle, OutputBuffer, USB_MIN_PACKET_SIZE+1);

}

extern "C" int USB_Read()
{
    if(DeviceHandle == NULL)
        return -1;

    return hid_read_timeout(DeviceHandle, InputBuffer, USB_MIN_PACKET_SIZE+1, 2000);
}

extern "C" int USB_Close()
{
    hid_close(DeviceHandle);
    USBConnected = false;

    return 0;
}

