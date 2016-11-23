// Config.h

#ifndef _CONFIG_h
#define _CONFIG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define clockspeed 16E6
#define prescaler 8
#define PPMHIGHTIME 16
#define UseInternalPullups

#define LCD_ADDRESS 0x20
#define LCD_ROWS 2
#define LCD_COLS 16

#define UIMenu 0
#define UIInfo 1

#define modeCol 0
#define yawCol 2
#define rollCol 7
#define pitchCol 12
#define graphCols 4

#define yawIndex 0
#define rollIndex 1
#define pitchIndex 2
#define modeIndex 3
#define frameSpaceIndex 4

#define pwmMin 1000
#define pwmMax 2000

#define pwmPerMode ((pwmMax - pwmMin)/6)

#define ppmOutputPin 5

#endif

