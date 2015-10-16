"""
	Python binding functions for the Texas Instrument LightCrafter DLP 4500 C/C++ API, ver. 2.0.0

	Dependencies:
		1. ctypes, an advanced foreign function interface for Python 2.3 and higher.
			The package is already included in Python 2.5 and higher.
		2. Linux: lcrlib.so / Windows: lcrlib.dll

	* Have not yet implemented programming mode functions from LCR API.
		Probably not neccessary/useful for most purposes.

	History:
		Created:
			pt - 08/20/2015
		Edits:
			pt - 08/26/2015 - added dll for Windows.
			
"""

from __future__ import division
from ctypes import *
import platform
import os, time, collections

from math import *

global lib
### Check operating system and load appropriate library.
current_platform = platform.system()
print 'Current Platform: ' + str(current_platform)
if current_platform == 'Linux':
	lib = cdll.LoadLibrary('lcrlib/lcr-api/bin/linux/libLightCrafter4500.so.1.0.0')
	print 'Linux SO loaded.'
elif current_platform == 'Windows':
	lib = cdll.LoadLibrary('lcr')
	print 'Window DLL loaded.'
else:
	print 'No libary loaded.'

### BIT MASKS
BITMASKS = [
	('BIT0', 0x01), 
	('BIT1', 0x02), 
	('BIT2', 0x04), 
	('BIT3', 0x08), 
	('BIT4', 0x10), 
	]

def parse_byte(byte_value, nByte):
	'''
	'''
	byte_list = []
	for i in range(0, nByte):
		BIT = BITMASKS[i][1]
		byte_list.append(byte_value & BIT)

	return byte_list

### Error checking and input valdiation functions.
def error_handler(value, function_name):
	"""
		Checks for flags.

	"""
	if value == -1:
		raise Exception(function_name + ' failed!')
def validate_linear_input(values, function_name):
	"""
		Checks if input is between 0.0 - 1.0
	"""
	for i in range(0, len(values)):
		if 0 <= values[i] <= 1:
			continue
		else:
			raise Exception('Inputs to ' + function_name + ' must be between 0.0-1.0.')
def validate_boolean_input(values, function_name):
	"""
		Checks if input is True or False (1 or 0)
	"""

	if isinstance(values, bool):
		return
	else:
		raise Exception('Inputs to ' + function_name + ' must be True or False.')

### USB Bindings
USB_Connected = 0
Device_Handle  = 0

def _isConnected():
	"""
		Checks the LCR USB connection

		RETURNS: 
			1  = connected
			0  = not connected
	"""
	USB_Connected = lib.USB_IsConnected()
	if USB_Connected == 0:
		USB_Connected = -1

	return USB_Connected

def _usbInit():
	"""
		Initialize the HIDAPI library. 
		Needs be called at the beginning of execution. 
		RETURNS:
			0  = success
			-1 = error
	"""
	return lib.hid_init()

def _usbOpen():
	"""
		Opens the LCR device using VID, PID
		For the TI DLP 4500
			Vendor ID:  0451
			Product ID: 6401

		lib.USB_Open() calls hid_open(VID, PID, NULL)
		hid_open opens HID device using Vendor ID, Product ID, and optionally a serial number.
		
		RETURNS:
			0  = success
			-1 = error
	"""
	# flag = _usbInit()
	# error_handler(flag, _usbInit.__name__)

	# VID = c_ushort(0451)
	# PID = c_ushort(6401)

	# pVID = pointer(VID)
	# pPID = pointer(PID)		

	# hid_open	     = lib.hid_open
	# hid_open.restype = POINTER(c_ushort)
	# Device_Handle    = hid_open(pVID, pPID, None)

	# if Device_Handle == 0:
	# 	USB_Connected = 0
	# else:
	# 	USB_Connected = 1

	# return USB_Connected
	return lib.USB_Open()

def _usbExit():
	"""
		Ends connection to LCR DLP 4500
	"""
	return lib.USB_Exit()

### ----------------------- ###

#### LCR Bindings
def lcrInit():
	"""
		This function calls the appropriate USB commands to open a connection with the LCR DLP 4500. 
		RETURNS: 
			1  = connected
			0  = not connected	
	"""

	flag0 = _usbOpen()
	error_handler(flag0, _usbOpen.__name__)
	flag1 = _isConnected()
	error_handler(flag1, _isConnected.__name__)
	return flag1

def lcrReset():
	"""
		Resets the LCR DLPC350 controller.

		RETURNS:
			0  = success
			-1 = error

	"""
	return lib.LCR_SoftwareReset()

def lcrSetPowerMode(standby_status):
	"""
		Places the DLPC350 in a low-power state and powers down the DMD interface.
		Standby mode should only be enabled after all data for the last frame to be displayed has been transferred. 

		PARAMS:
			TRUE  = Standby mode. 
			FALSE = Normal operation.
		FLAG:
			>=0 = success
			<0  = failure
	"""
	validate_boolean_input(standby_status,lcrSetPowerMode.__name__)
	standby = c_bool(standby_status)
	flag = lib.LCR_SetPowerMode(standby)

def lcrGetVersion():
	""" 
		Reads version information of DLPC350 firmware.
		 
		PARAMS:
			pApp_ver  - O - Application Software Revision BITS 0:15 PATCH NUMBER, BITS 16:23 MINOR REVISION, BIS 24:31 MAJOR REVISION
			pAPI_ver  - O - API Software Revision BITS 0:15 PATCH NUMBER, BITS 16:23 MINOR REVISION, BIS 24:31 MAJOR REVISION
			pSWConfig_ver  - O - Software Configuration Revision BITS 0:15 PATCH NUMBER, BITS 16:23 MINOR REVISION, BIS 24:31 MAJOR REVISION
			pSeqConfig_ver  - O - Sequence Configuration Revision BITS 0:15 PATCH NUMBER, BITS 16:23 MINOR REVISION, BIS 24:31 MAJOR REVISION
		 
		FLAG:
			0  = success
			-1 = error
	"""

	app_ver 	   = c_uint()
	API_ver 	   = c_uint()
	SWConfig_ver   = c_uint()
	SeqConfig_ver  = c_uint()

	pApp_ver 	   = pointer(app_ver)
	pAPI_ver	   = pointer(API_ver)
	pSWConfig_ver  = pointer(SWConfig_ver)
	pSeqConfig_ver = pointer(SeqConfig_ver)

	get_version = lib.LCR_GetVersion
	flag 		= get_version(pApp_ver, pAPI_ver, pSWConfig_ver, pSeqConfig_ver)
	error_handler(flag, LCR_GetVersion.__name__)

	version = {'app_ver': app_ver.value, 'api_ver': API_ver.value, 'SWConfig_ver': SWConfig_ver.value, 'SeqConfig_ver': SeqConfig_ver.value}
	return version

def lcrGetInputSource():
	"""
		Reads back the input source selection to be displayed by the DLPC350.
		Options: 
			30-bit, Parallel Port, Internal Test Pattern, Flash Memory, FPD-Link interface.

		PARAMS:
			source: 	Selects the input source and interface mode.
						0 = Parallel interface with 8, 16, 20, 24, or 30-bit RGB or YcrCb data formats.
						1 = Internal test pattern; Use lcrSetTPGSelect() to select pattern.
						2 = Flash. Images are 24-bit single-frame, still images stored in flash that are uploaded on command. 
						3 = FPD-link interface.

			portWidth: 	Parallel interface bit-depth
						0 = 30-bits
						1 = 24-bits
						2 = 20-bits
						3 = 16-bits
						4 = 10-bits
						5 = 8-bits
		FLAGS:
			0   = success
			<0  = failure
	"""
	source 		= c_uint()
	portWidth   = c_uint()
	
	flag = lib.LCR_GetInputSource(byref(source), byref(portWidth))
	error_handler(flag, lcrGetInputSource.__name__)

	input_source = source.value
	port_width = portWidth.value

	return input_source, port_width

def lcrSetInputSource(source, portWidth):
	"""
		Selects the input source selection to be displayed by the DLPC350.
		Options: 
			30-bit, Parallel Port, Internal Test Pattern, Flash Memory, FPD-Link interface.

		PARAMS:
			source: 	Selects the input source and interface mode.
						0 = Parallel interface with 8, 16, 20, 24, or 30-bit RGB or YcrCb data formats.
						1 = Internal test pattern; Use lcrSetTPGSelect() to select pattern.
						2 = Flash. Images are 24-bit single-frame, still images stored in flash that are uploaded on command. 
						3 = FPD-link interface.

			portWidth: 	Parallel interface bit-depth
						0 = 30-bits
						1 = 24-bits
						2 = 20-bits
						3 = 16-bits
						4 = 10-bits
						5 = 8-bits
		FLAGS:
			>=0 = success
			<0  = failure
	"""
	if source != 0:
		portWidth = 0

	source_c = c_uint(source)
	portWidth_c = c_uint(portWidth)

	input_source = lib.LCR_SetInputSource
	flag = input_source(source_c, portWidth_c)
	error_handler(flag, lcrGetInputSource.__name__)

def lcrGetTPGSelect():
	"""
		Reads back the selected internal test pattern.

		PARAMS:
			pattern 		- Selected internal test pattern:
            	            0x0 = Solid Field
                	        0x1 = Horizontal Ramp
                    	    0x2 = Vertical Ramp
                        	0x3 = Horizontal Lines
                         	0x4 = Diagonal Lines
                         	0x5 = Vertical Lines
                         	0x6 = Grid
                         	0x7 = Checkerboard
                         	0x8 = RGB Ramp
                         	0x9 = Color Bars
                         	0xA = Step Bars
	"""
	pattern = c_uint()
	flag = lib.LCR_GetTPGSelect(byref(pattern))
	error_handler(flag, lcrGetTPGSelect.__name__)
	return pattern.value

def lcrSetTPGSelect(pattern):
	"""
		When the internal test pattern is the selected input, the Internal Test Patterns Select defines the test
		pattern displayed on the screen. These test patterns are internally generated and injected into the
		beginning of the DLPC350 image processing path. Therefore, all image processing is performed on the
		test images. All command registers should be set up as if the test images are input from an RGB 8:8:8
		external source.

		PARAMS:
			pattern 		- Selected internal test pattern:
            	            0 = Solid Field
                	        1 = Horizontal Ramp
                    	    2 = Vertical Ramp
                        	3 = Horizontal Lines
                         	4 = Diagonal Lines
                         	5 = Vertical Lines
                         	6 = Grid
                         	7 = Checkerboard
                         	8 = RGB Ramp
                         	9 = Color Bars
                         	A = Step Bars
	"""
	pattern = c_uint(pattern)
	flag 	= lib.LCR_SetTPGSelect(pattern)
	error_handler(flag, lcrSetTPGSelect.__name__)

def lcrGetDisplay():
	""" Will add later, need to create rectangle ctypes structure."""
	pass

def lcrGetMode():
	"""
		This function gets the current display mode used by the DLPC350.

		PARAMS
			set_mode:	True  = Pattern Display Mode. Assumes 1-bit to 8-bit image with a pixel resolution of 912x1140...
								and bypasses all image processing functions of the DLPC350. 
						False = Video Display Mode. Assumes a streaming video image from 30-bit RGB / FPD-link interface. 
						   		Pixel Resolution of up to 1280 x 800 up to 120 Hz. (Default mode)
		RETURNS
			mode: 		1 = Pattern Display Mode established.
						0 = Video Display Mode established.
	"""

	mode  = c_bool()
	get_mode = lib.LCR_GetMode
	flag 	 = get_mode(byref(mode))
	error_handler(flag, lcrGetMode.__name__)

	return mode.value

def lcrSetMode(set_mode):
	"""
		This function selects between Video or Pattern Display Mode.

		PARAMS
			set_mode:	True  = Pattern Display Mode. Assumes 1-bit to 8-bit image with a pixel resolution of 912x1140...
								and bypasses all image processing functions of the DLPC350. 
						False = Video Display Mode. Assumes a streaming video image from 30-bit RGB / FPD-link interface. 
						   		Pixel Resolution of up to 1280 x 800 up to 120 Hz. (Default mode)
		FLAG
			0  = success 
			-1 = failure
	"""
	validate_boolean_input(set_mode, lcrSetMode.__name__)
	mode  = c_bool(set_mode)

	flag  = lib.LCR_SetMode(mode) # associate 
	error_handler(flag, lcrSetMode.__name__)

def lcrGetLedEnables():
	"""
		This function reads back the state of LED control method as well as enables/disables status of specified LEDs.

		PARAMS:
			seqCtrl:				 	1 = All LED states are controlled by the Sequencer. Ignore the other LED enable settings.
										0 = All LED states are controlled by the other LED enable settings. Ignore Sequencer control.
			redLED/greenLED/blueLED:	1 = X LED is enabled.
										0 = X LED is disabled.

		FLAG:
			0  = success
			-1 = failure

		RETURNS
			led_controller: seqCtrl
			led_states: 	List of all status of all LEDs.
	"""

	seqCtrl 		= c_bool()
	red  			= c_bool()
	green 			= c_bool()
	blue			= c_bool()

	get_led_enables = lib.LCR_GetLedEnables

	flag 			= get_led_enables(byref(seqCtrl), byref(red), byref(green), byref(blue))
	error_handler(flag, lcrGetLedEnables.__name__)

	led_controller 	= seqCtrl.value
	leds_state 	   	= [red.value, green.value, blue.value]

	return led_controller, leds_state

def lcrSetLedEnables(seqCtrl, RLED, GLED, BLED):
	"""
		This function sets the state of LED control method as well as enables/disables status of specified LEDs.

		PARAMS:
			seqCtrl:				 	1 = All LED states are controlled by the Sequencer. Ignore the other LED enable settings.
										0 = All LED states are controlled by the other LED enable settings. Ignore Sequencer control.
			redLED/greenLED/blueLED:	1 = X LED is enabled.
										0 = X LED is disabled.

		FLAG:
			0  = success
			-1 = failure
	"""
	seqCtrl = bool(seqCtrl)
	RLED 	= bool(RLED)
	GLED 	= bool(GLED)
	BLED 	= bool(BLED)


	seqCtrl_state = c_bool(seqCtrl)
	red_state     = c_bool(RLED)
	green_state   = c_bool(GLED)
	blue_state    = c_bool(BLED)

	set_led_state = lib.LCR_SetLedEnables

	flag = set_led_state(seqCtrl_state,
						 red_state,
						 green_state,
						 blue_state)
	error_handler(flag, lcrSetLedEnables.__name__)

def lcrGetLedCurrents():
	"""
		This function gets the pulse duration of the specific LED PWM modulation output pin.
		The resolution is 8-bit (256 steps) and corresponds to a percentage of the LED current. 
		The allowable PWM value is 0 to 100%

		If LED PWM polarity is set to normal polarity, 0xFF gives max PWD current. 

		RETURNS
			LEDsCurrents:    returns a list of integers of the red, green, blue leds currents, respectively.

		FLAG
			0  = success 
			-1 = failure

	"""

	red   = c_ubyte() #unsigned char
	green = c_ubyte() #unsighed char
	blue  = c_ubyte() #unsighed char

	get_led_currents = lib.LCR_GetLedCurrents #assign python object to C/C++ function.

	flag = get_led_currents(byref(red), byref(green), byref(blue))
	error_handler(flag, lcrGetLedCurrents.__name__) #checks for success.

	LEDsCurrents = [int(red.value), int(green.value), int(blue.value)]
	return LEDsCurrents

def lcrSetLedCurrents(RED = 0, GREEN = 0, BLUE = 0):
	"""
		This function controls the pulse duration of the specific LED PWM modulation output pin.
		The resolution is 8-bit (256 steps) and corresponds to a percentage of the LED current. 
		The allowable PWM value is 0 to 100%

		If LED PWM polarity is set to normal polarity, 0xFF gives max PWD current. 

		PARAMS
			RED/GREEN/BLUE: X LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
							0 percent duty cycle --> X LED driver generates no current
							100 percent duty cycle --> X LED driver generates maximum current
							The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design.
							and thus varies by design.

		FLAG
			0  = success 
			-1 = failure
	"""

	validate_linear_input([RED, GREEN, BLUE], lcrSetLedCurrents.__name__)

	# Convert linear inputs into 8-bit steps.
	RED   = int(floor(RED*256))
	GREEN = int(floor(GREEN*256))
	BLUE  = int(floor(BLUE*256))

	redCurrent   = c_ubyte(RED)   #unsigned char
	greenCurrent = c_ubyte(GREEN) #unsigned char
	blueCurrent  = c_ubyte(BLUE)  #unsigned char

	set_led_currents = lib.LCR_SetLedCurrents #construct object for calling the c function.

	flag = set_led_currents(redCurrent, greenCurrent, blueCurrent) # 0 = success, -1 = failure.
	error_handler(flag, lcrSetLedCurrents.__name__) #error handling.

def lcrGetLongAxisImageFlip():
	"""
		The Long-Axis Image Flip defines whether the input image is flipped across the long axis of the DMD. If
		this parameter is changed while displaying a still image, the input still image should be re-sent. If the
		image is not re-sent, the output image might be slightly corrupted. In Structured Light mode, the image
		flip will take effect on the next bit-plane, image, or video frame load.

		RETURNS:
			TRUE/1  = Image flipped along long axis.
			FALSE/0 = Image not flipped.
	"""
	return lib.LCR_GetLongAxisImageFlip()

def lcrSetLongAxisImageFlip(long_flip):
	"""
		The Long-Axis Image Flip defines whether the input image is flipped across the long axis of the DMD. If
		this parameter is changed while displaying a still image, the input still image should be re-sent. If the
		image is not re-sent, the output image might be slightly corrupted. In Structured Light mode, the image
		flip will take effect on the next bit-plane, image, or video frame load.

		PARAMS:
			TRUE/1  = Image flipped along long axis enable.
			FALSE/0 = Do not flip image.
	"""
	validate_boolean_input(long_flip, lcrSetLongAxisImageFlip.__name__)
	flip_state = c_bool(long_flip)

	set_long_axis = lib.LCR_SetLongAxisImageFlip
	flag = set_long_axis(flip_state)
	error_handler(flag, lcrSetLongAxisImageFlip.__name__)

def lcrGetShortAxisImageFlip():
	"""
		The Short-Axis Image Flip defines whether the input image is flipped across the short axis of the DMD. If
		this parameter is changed while displaying a still image, the input still image should be re-sent. If the
		image is not re-sent, the output image might be slightly corrupted. In Structured Light mode, the image
		flip will take effect on the next bit-plane, image, or video frame load.
		
		RETURNS:
			TRUE/1 = Image flipped along short axis.
			FALSE/0 = Image not flipped.
	"""
	return lib.LCR_GetShortAxisImageFlip()

def lcrSetShortAxisImageFlip(short_flip):
	"""
		The Short-Axis Image Flip defines whether the input image is flipped across the short axis of the DMD. If
		this parameter is changed while displaying a still image, the input still image should be re-sent. If the
		image is not re-sent, the output image might be slightly corrupted. In Structured Light mode, the image
		flip will take effect on the next bit-plane, image, or video frame load.
		
		PARAMS:
			TRUE = Image flipped along short axis.
			FALSE = Do not flip image.
		FLAG:
			0  = success
			-1 = failure
	"""
	validate_boolean_input(short_flip, lcrSetShortAxisImageFlip.__name__)
	flip_state = c_bool(short_flip)

	set_short_axis = lib.LCR_SetShortAxisImageFlip
	flag = set_short_axis(flip_state)
	error_handler(flag, lcrSetShortAxisImageFlip.__name__)

def lcrGetPatternDisplayMode():
	"""
		See lcrSetPatternDisplayMode()

		RETURNS:
			True/1  = Pattern Display Data is streamed through the 24-bit RGB/FPD-link interface.
			False/0 = Pattern Display Data is taken from splash (flash?) memory.
	"""
	external = c_bool()
	flag 	 = lib.LCR_GetPatternDisplayMode(byref(external))
	error_handler(flag, lcrGetPatternDisplayMode.__name__)
	return external.value

def lcrSetPatternDisplayMode(external_state):
	"""
		The Pattern Display Data Input Source command selects the source of the data for pattern display:
		streaming through the 24-bit RGB/FPD-link interface or stored data in the splash image memory area from
		external Flash. Before executing this command, stop the current pattern sequence. After executing this
		command, send the Validation command (lcrValidatePatLutData()) once before starting the pattern sequence.

		PARAMS:
			external  	TRUE = Pattern Display Data is streamed through the 24-bit RGB/FPD-link interface
						FALSE = Pattern Display Data is fetched from splash memory
		FLAG:
			0  = success
			-1 = failure

	"""
	external = c_bool(external_state)
	flag = lib.LCR_SetPatternDisplayMode(external)
	error_handler(flag, lcrSetPatternDisplayMode.__name__)

def lcrLoadSplash(index):
	"""
		Loads an image from flash memory and then performs a buffer swap to display the loaded image on the DMD.

		PARAMS:
			index = image index.

		FLAG:
			0  = success
			-1 = failure
	"""
	index = c_uint(index)
	flag  = lib.LCR_LoadSplash(index)
	error_handler(flag, lcrLoadSplash.__name__)

def lcrGetSplashIndex():
	"""
		Reads back image index that was loaded most recently via lcrLoadSplash().

		PARAMS:
			index = image index.

		FLAG:
			0  = success
			-1 = failure
	"""
	prev_index = c_uint()
	flag 	   = lib.lcrGetSplashIndex(byref(prev_index))
	error_handler(flag, lcrGetSplashIndex.__name__)
	return prev_index.value

def lcrSendSplashLut(nEntries):
	"""
		Sends the image LUT to the DLPC350 controller.

		PARAMS:
			numEntries 		= number of entries to be sent to the controller.
			lutEntries		= pointer to the array in which LUT entries to be sent are stored.

		FLAG: 
			0  = success
			-1 = failure
	"""
	num_entries = c_uint(nEntries)
	lut_entries = c_ubyte()

	flag = lib.LCR_SendSplashLut(byref(lut_entries), num_entries)
	error_handler(flag, lcrSendSplashLut)
	return lut_entries.value

def lcrClearPatLut():
	"""
		Locally clears the stored pattern LUT. Does not send any commands to the controller.

		FLAG:
			0  = success
			-1 = failure
	"""
	flag = lib.LCR_ClearPatLut()
	error_handler(flag, lcrClearPatLut.__name__)

def lcrAddToPatLut(trigType, patIndex, bitDepth, LEDselect, invert, insertBlk, bufferSwap, trigOut):
	"""
		Function makes an entry in the locally stored pattern LUT as per input arguments passed.
		Does not send any commands to the controller.
		See table 2-65 in programmer's guide for detailed description of Pattern LUT entries.

		PARAMS:
			trigType 	= Selects the trigger type for the pattern.
						0 = Internal
						1 = External positive
						2 = External negative
						3 = No input trigger (Continue from previous; pattern still has full exposure time)
			patIndex	= Pattern number (0 based index). For patter number 0x3F, there is no pattern display.
						  The max number supported is 24 for 1 bit-depth patterns.
						  Setting the pattern number to be 25, with bit-depth of 1 will insert a white-fill pattern. 
						  Inverting this pattern will insert at black-fill pattern.
						  These white/black patterns have the same exposure time as defined in Pattern Display Exporsure and
						  Frame Period Command. 
						  Table 2-66 of Programmer's guide illustrates bit planes.
			bitDepth 	= Select desired bit-depth.
						0 = Reserved.
						1 = 1-bit
						2 = 2-bit
						...
						8 = 8-bit

			LEDselect	= Choose the LEDS that are on. b0 = Red, b1 = Green, b2 = blue
						0 = No LED (pass through)
						1 = Red
						2 = Green
						3 = Yellow(Green+Red)
						4 = blue
						5 = Magenta(Blue + Red)
						6 = Cyan (Blue + Green)
						7 = White (All LEDS)

			invert 		True = invert pattern. False = Do not invert.

			insertBlk	True = Insert black-fill pattern after current pattern. This setting requires 230 microseconds of time
								before the start of the next pattern.

			bufferSwap	True  = Perform a buffer swap.
						False = Do not perform a swap.

			trigOut		True = Trigger out 1 will continue to be high. There will be no falling edge between the end of the previous pattern
								and the start of the current pattern. w exposure time is shared between all patterns definued under a common trigger out.
						False = Trigger out 1 has a rising edge at the start of a pattern and a falling edge at the end of the pattern.
	
		FLAG:
			0  = success
			-1 = failure

	"""

	trig_type  	= c_int(trigType)
	pat_index	= c_int(patIndex)
	bit_depth  	= c_int(bitDepth)
	LED_select 	= c_int(LEDselect)
	invert_pat 	= c_bool(invert)
	insert_blk 	= c_bool(insertBlk)
	buffer_swap = c_bool(bufferSwap)
	trig_out 	= c_bool(trigOut)

	add_to_pat_lut = lib.LCR_AddToPatLut
	flag = add_to_pat_lut(trig_type, pat_index, bit_depth, LED_select, invert_pat, insert_blk, buffer_swap, trig_out)
	error_handler(flag, lcrAddToPatLut.__name__)
	return flag

def lcrGetPatLutItem(index):
	"""
		Reads back an entry at the specified index from the locally stored pattern LUT and populates the input arguments passed to this function.
		Does not send any commands to the controller.
		See lcrAddToPatLut()

		index = Entry at this index from patttern LUT to be read back.
	"""

	index = c_int(index)

	trig_type  	= c_int()
	pat_index	= c_int()
	bit_depth  	= c_int()
	LED_select 	= c_int()
	invert_pat 	= c_bool()
	insert_blk 	= c_bool()
	buffer_swap = c_bool()
	trig_out 	= c_bool()

	gpli = lib.LCR_GetPatLutItem

	flag = gpli(index, byref(trig_type),
					   byref(pat_index),
					   byref(bit_depth),
					   byref(LED_select),
					   byref(invert_pat),
					   byref(insert_blk),
					   byref(buffer_swap),
					   byref(trig_out))
	error_handler(flag, lcrGetPatLutItem.__name__)

	pat_lut_item = {'trigType': trig_type.value,
					'patIndex': pat_index.value,
					'bitDepth': bit_depth.value,
					'LEDselect': LED_select.value,
					'invert': invert_pat.value,
					'insertBlk': insert_blk.value,
					'bufferSwap': buffer_swap.value,
					'trigOut': trig_out.value}

	return pat_lut_item

def lcrSendPatLut():
	"""
		Sends pattern LUT by calling lcrAddToPatLut to the DLPC350 controller.

		FLAG
			0  = success
			-1 = failure
	"""
	flag = lib.LCR_SendPatLut()
	error_handler(flag, lcrSendPatLut.__name__)
	return flag

def lcrGetPatLut(nEntries):
	"""
		Reads the pattern LUT from the DLPC350 controller and stores it in the local array.
		Pattern LUT entries should be queries using the API lcrGetPatLutItem().

		PARAMS:
			numEntries 		= Number of entries expected in pattern LUT.

		RETURN:
			number of bytes actually read from the controller LUT.

	"""
	nEntries = c_int(nEntries)
	numBytes = lib.LCR_GetPatLut(nEntries)
	return numBytes


def lcrGetSplashLut(num_entries):
	"""
		Reads the image LUT from DLPC350 controller.

		PARAMS:
			num_entries:	number of image LUT entries to be read from the controller.

		RETURNS:
			pLUT:			pointer to array in which the read entries should be stored.

		FLAG:
			0  = success
			-1 = failure
	"""

	pLUT 	 = c_ubyte()
	nEntries = c_int(num_entries)

	get_splash_lut = lib.LCR_GetSplashLut
	flag = get_splash_lut(byref(pLUT), nEntries)
	error_handler(flag, lcrGetSplashLut.__name__)

	return pLUT.value



def lcrSetPatternTriggerMode(int_ext_or_vSync):
	"""
		Selects between one of the three pattern Trigger Modes.
		Before calling this function, stop the current pattern sequence. i.e. lcrPatternDisplay(0)
		After calling this function, send the Validation command once before starting the pattern sequence.

		PARAMS:
			int_ext_or_vSync 	1 = Pattern Trigger Mode 1: Internally or externally (through TRIG_IN1 and TRIG_IN2) generated trigger. 
								0 = Pattern Trigger Mode 0: VSYNC serves to trigger the pattern display sequence.

		FLAG
			0  = success
		    -1 = failure
	"""
	validate_boolean_input(int_ext_or_vSync, lcrSetPatternTriggerMode.__name__)
	trigger = c_bool(int_ext_or_vSync)

	flag 	= lib.LCR_SetPatternTriggerMode(trigger)
	error_handler(flag, lcrSetPatternTriggerMode.__name__)

def lcrGetPatternTriggerMode():
	"""
		See lcrSetPatternTriggerMode()

		RETURNS:
			trigger 	1 = Pattern Trigger Mode 1: Internally or externally (through TRIG_IN1 and TRIG_IN2) generated trigger. 
						0 = Pattern Trigger Mode 0: VSYNC serves to trigger the pattern display sequence.

	"""
	trigger = c_bool()
	flag 	= lib.LCR_GetPatternTriggerMode(byref(trigger))
	error_handler(flag, lcrGetPatternTriggerMode.__name__)
	return trigger.value

def lcrPatternDisplay(command):
	"""
		Starts or stops the programmed patterns sequence.

		PARAMS:
			command 	  = Pattern display start/stop.
						0 = Stop pattern display sequence. The next "Start" command will restart the pattern sequence from the beginning.
						1 = Pause Pattern Display Sequence. The next "start" command will start the sequence by re-displaying the current pattern in the sequence.
						2 = Start the Pattern Display Sequence.

		FLAG:
			0  = success
			-1 = failure
	"""
	command 		= c_int(command)
	pattern_display = lib.LCR_PatternDisplay
	flag 			= pattern_display(command)

	error_handler(flag, lcrPatternDisplay.__name__)


def lcrSetPatternConfig(nLutEntries, repeat, nPatsTrigOut2, nSplash):
	"""
		Controls the execution of patterns stored in the lookup table.
		Before calling this function, stop the current pattern sequence using lcrPatternDisplay(0).
		After calling this function, send the Validation command using lcrValidatePatLutData() before starting the pattern sequence.

		PARAMS:
			nLutEntries 	  = Number of LUT entries.

			repeat 			0 = execute the pattern sequence once
							1 = repeat the pattern sequence. 

			nPatsTrigOut2     = Number of patterns to display (range 1 - 256)
							  = if in repeat mode, then this value dictates how often TRIG_OUT_2 is generated.

			nSplash       	  = number of image index LUT entries (range 1 - 64)
							  = this fieldis irrelevant for pattern display data input source set to a value other than internal.
		FLAG:
			0  = success
			-1 = failure			 
	"""
	
	numLutEntries 		= c_uint(nLutEntries)
	repeat		  		= c_uint(repeat)
	numPatsTrigOut2 	= c_uint(nPatsTrigOut2)
	numSplash			= c_uint(nSplash)

	set_pattern_config 	= lib.LCR_SetPatternConfig

	flag = set_pattern_config(numLutEntries, repeat, numPatsTrigOut2, numSplash)
	error_handler(flag, lcrSetPatternConfig.__name__)

def lcrGetPatternConfig():
	"""
		See lcrSetPatternConfig()

		RETURNS:
			nLutEntries 	  = Number of LUT entries.
			
			repeat 			0 = execute the pattern sequence once
							1 = repeat the pattern sequence. 

			nPatsTrigOut2     = Number of patterns to display (range 1 - 256)
							  = if in repeat mode, then this value dictates how often TRIG_OUT_2 is generated.

			nSplash       	  = number of image index LUT entries (range 1 - 64)
							  = this fieldis irrelevant for pattern display data input source set to a value other than internal.

	"""
	numLutEntries 	= c_uint()
	repeat		  	= c_uint()
	numPatsTrigOut2 = c_uint()
	numSplash		= c_uint()
	get_pattern_config = lib.LCR_GetPatternConfig
	flag = get_pattern_config(byref(numLutEntries),
							  byref(repeat),
							  byref(numPatsTrigOut2),
							  byref(numSplash))
	
	error_handler(flag, lcrGetPatternConfig.__name__)

	pattern_config = {'nLutEntries': numLutEntries.value,
					  'repeat': repeat.value,
					  'nPatsTrigOut2': numPatsTrigOut2.value,
					  'nSplash': numSplash.value}
	return pattern_config

def lcrSetExposureFramePeriod(exposurePeriod, framePeriod):
	"""
		Dictates the time a pattern is exposed and the frame period.
		Either the exposure time must be equivalent to the frame period, or the exposure time must be 
		less than the frame period by 230 microseconds. 

		Before executing this command, stop the current pattern sequence. i.e. lcrPatternDisplay(0)
		Afterwards, call lcrValidatePatLutData() before starting patttern sequence.

		PARAMS:
			exposurePeriod 		= exposure time in microseconds.
			framePeriod 		= frame period in microseconds.

		FLAG:
			0  = success
			-1 = failure.
	"""
	exposurePeriod 	= int(exposurePeriod)
	framePeriod 	= int(framePeriod)
	exposure_time 	= c_uint(exposurePeriod)
	frame_time 	  	= c_uint(framePeriod)

	set_exposure = lib.LCR_SetExposure_FramePeriod
	flag 		 = set_exposure(exposure_time, frame_time)
	error_handler(flag, lcrSetExposureFramePeriod.__name__)

def lcrGetExposureFramePeriod():
	"""
		See lcrSetExposureFramePeriod()
		
		RETURNS:
			exposurePeriod 		= exposure time in microseconds.
			framePeriod 		= frame period in microseconds.
	"""

	exposure_time = c_uint()
	frame_time 	  = c_uint()

	flag = lib.LCR_GetExposure_FramePeriod(byref(exposure_time), byref(frame_time))
	error_handler(flag, lcrGetExposureFramePeriod.__name__)

	periods = {'exposure': exposure_time.value, 
			   'frame': frame_time.value}

	return periods

def lcrValidatePatLutData():
	"""
		Checks the programmed pattern display modes and indicates any invalid settings.
		Function should be called after all pattern display Configurations have been completed.

	PARAMS:
		status 		BIT0 = validity of exposure or frame period settings. (BIT0 : 0x01)
					   1 = selected exposure or frame period settings are invalid.
					   0 = valid.
					BIT1 = validity of pattern numbers in LUT. (BIT1 : 0x02)
					   1 = invalid
					   0 = valid
					BIT2 = status of Trigger OUT1. (BIT2 : 0x04)
					   1 = Warning, continous Trigger Out1 request or overlapping black sectors.
					   0 = Trigger OUT1 settings are valid.
                    BIT3 = Status of post sector settings. (BIT3 : 0x08)
                       1 = Warning, post vector was not inserted prior to external triggered vector
                       0 = Post vector settings are valid
                    BIT4 = Status of frame period and exposure difference (BIT4 : 0x10)
                       1 = Warning, frame period or exposure difference is less than 230usec
                       0 = Frame period or exposure difference is valid
		
		FLAG:
			0  = success.
			-1 = failure.

	"""
	status = c_uint()

	flag = lib.LCR_ValidatePatLutData(byref(status))
	if flag == -1:
		print status.value
		raise Exception(function_name + ' failed!')
	valid = parse_byte(status.value, 5)

	if all(v == 0 for v in valid):
		return 0

	if valid[0]:
		print 'Selected exposure or frame period settings are invalid.'
		return -1

	if valid[1]:
		print 'Pattern numbers in LUT is invalid.'
		return -1

	if valid[2]:
		print 'Warning, continous Trigger Out1 request or overlapping black sectors.'
		return -1

	if valid[3]:
		print 'Warning, post vector was not inserted prior to external triggered vector.'
		return -1

	if valid[4]:
		print 'Warning, frame period or exposure difference is less than 250 microseconds.'
		return -1

def lcrMeasureSplashLoadTiming(startIndex, nSplash):
	"""
		Measure the load time for the images stored using at specified index.
		Read the result using lcrReadSplashLoadTiming().
		
		PARAMS:
	  		startIndex  	= index of the first image whose load time is to be measured
	  
	  		nSplash 		= number of images for which load time is to be measured.
	  	
		FLAG:
			0  = success
			-1 = failure.
	"""
	start_index = c_uint(startIndex)
	numSplash 	= c_uint(nSplash)

	flag 		= lib.LCR_MeasureSplashLoadTiming(start_index, numSplash)
	error_handler(flag, lcrMeasureSplashLoadTiming.__name__)

def lcrReadSplashLoadTiming():
	"""
		See lcrMeasureSplashLoadTiming()
	"""
	timing_data = c_uint()
	flag 		= lib.LCR_ReadSplashLoadTiming(byref(timing_data))
	error_handler(flag, lcrReadSplashLoadTiming.__name__)
	return timing_data.value

def lcrExit():
	'''
	'''

	lib.USB_Close()
	flag = lib.USB_Exit()

	if flag == 0:
		print '\n Connection exited. \n'
	else:
		print '\n Connection did not exit. \n'

def test_connection():
	flag = lcrInit()
	if flag >= 1:
		print '\n Connection successful! \n'
	else:
		print flag

	
	# lcrReset()

if __name__ == '__main__':
	test_connection()

