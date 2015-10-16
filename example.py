from __future__ import division
from ctypes import *

import time, os, sys
from math import *
from lcrlib import *

'''
	Rough script to show the basic of the wrapper library. 9/17/2015
	10/16/2015 - Removed lcrPatternDisplay loop.
	
'''


### LCR Parameters ###
patSeqMode 			= True # Set pattern sequence mode
triggerPer 			= 10000 # defines amount of time between patterns, in microseconds
exposurePer			= triggerPer	#defines amount of time a single pattern is displayed, in microseconds.
vidSrc				= True # True for HDMI port.

start_seq			= 2 
pause_seq			= 1
stop_seq			= 0

### Pattern Parameters ###
nLUT_entries 		= 24
repeatPat			= True
invertPat			= False
bitDepth			= 1
clearDMD			= True
bufferSwap			= True
trigOut				= False
trigType			= 0 # internal trigger
LEDs 				= 1 # red

lcrInit() # Establish USB connection with DLP.

lcrSetMode(patSeqMode)
lcrSetPatternDisplayMode(vidSrc)

lcrPatternDisplay(stop_seq) # Stop sequencer before adding entries to LUT.
lcrClearPatLut() # Clear LUT.


### Store pattern to the LUT.
for p in range(0, nLUT_entries):
	lcrAddToPatLut(trigType, 
		p,
		bitDepth,
		LEDs,
		invertPat,
		clearDMD,
		bufferSwap,
		trigOut)

### Configure the execution of patterns stored in lookup table.

lcrSetPatternConfig(nLUT_entries, repeatPat, 1, 0) # last arg is irrelevant if pattern source is video/hdmi port.
lcrSetExposureFramePeriod(exposurePer, triggerPer) # Set the exposure and frame period for each entry in the pattern LUT.
lcrSetPatternTriggerMode(True) # Set pattern trigger mode to VSYNC. 


lcrSendPatLut() # Sent patttern LUT to the DMD.

flag = lcrValidatePatLutData() # Validate the LUT. 
time.sleep(2.0)

raw_input('Press Enter to start.')

if flag != -1:
	lcrPatternDisplay(2)

raw_input('Press Enter to stop.')

### Close connection with LCR.
lcrPatternDisplay(stop_seq)
lcrExit()
