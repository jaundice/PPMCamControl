// RadioButtonOption.h

#ifndef _RADIOBUTTONOPTION_h
#define _RADIOBUTTONOPTION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include "Config.h"

class RadioButtonOption
{
private:
	bool _isActive;

public :
	byte ButtonPin;
	byte LedPin;
	bool IsActive();
	uint16_t PPMTime;

	RadioButtonOption(byte buttonPin, byte ledPin, bool isActive, uint16_t ppmTime);
	void SetActive(bool isActive);
};

inline bool RadioButtonOption::IsActive()
{
	return _isActive;
}


inline RadioButtonOption::RadioButtonOption(byte buttonPin, byte ledPin, bool isActive,uint16_t ppmTime)
{
	ButtonPin = buttonPin;
	LedPin = ledPin;
	_isActive = isActive;
	PPMTime = ppmTime;

#ifdef UseInternalPullups
	pinMode(buttonPin, INPUT_PULLUP);
#else
	pinMode(buttonPin, INPUT);
#endif
	pinMode(ledPin, OUTPUT);
}

inline void RadioButtonOption::SetActive(bool isActive)
{
	if(isActive)
	{
		digitalWrite(LedPin, HIGH);
	}else
	{
		digitalWrite(LedPin, LOW);
	}
	_isActive = isActive;
}

#endif

