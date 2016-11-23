// RadioButton.h

#ifndef _RADIOBUTTON_h
#define _RADIOBUTTON_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include "RadioButtonOption.h"

//#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))



template <typename T>
inline uint8_t arraysize(const T(v[])) { return sizeof(v) / sizeof(T); }

class RadioButton
{
	RadioButtonOption* Options;
	volatile int8_t _selectedIndex;

public:
	RadioButtonOption SelectedOption()
	{
		return Options[_selectedIndex];
	}

	int8_t SelectedIndex()
	{
		return _selectedIndex;
	}

	RadioButton(RadioButtonOption buttonOptions[], int selectedOption = -1)
	{
		Options = buttonOptions;
		for (uint8_t i = 0; i < arraysize(buttonOptions); i++)
		{
			Options[i].SetActive(false);
		}
		SelectOption(selectedOption);
	}

	void SelectOption(int index)
	{
		if (_selectedIndex > -1)
		{
			Options[_selectedIndex].SetActive(false);
		}
		if (index > -1)
		{
			Options[index].SetActive(true);
		}

		_selectedIndex = index;
	}
};

#endif

