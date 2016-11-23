#include "Config.h"
#include "RadioButton.h"
#include "RadioButtonOption.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PPMFRAMESPACE 4000

inline float pwmPercentage(uint16_t pwmValue)
{
	return (static_cast<float>(pwmValue - pwmMin) / static_cast<float>(pwmMax - pwmMin));
}

uint8_t UIMode = 1;
uint8_t PrevUIMode = 1;
volatile uint8_t PrevQuadraturePins;
volatile uint8_t PrevModePins;

volatile uint8_t ppmOutputIndex = 4;
const uint16_t ticksPerMicrosecond = (static_cast<uint32_t>(clockspeed) / static_cast<uint32_t>(prescaler)) / 1000000;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
RadioButtonOption rbopts[]{
	RadioButtonOption(53,22,false,1142),
	RadioButtonOption(52,23,false,1284),
	RadioButtonOption(51,24,false,1426),
	RadioButtonOption(50,25,false,1568),
	RadioButtonOption(10,26,false,1710),
	RadioButtonOption(11,27,false,1852),
};
RadioButton modeButton(rbopts, 0);

volatile uint16_t PwmTimes[]{
	1500,
	1500,
	1500,
	1500,
	PPMFRAMESPACE
};

static inline uint32_t CalculateTimeout(uint16_t microSeconds)
{
	return (microSeconds * ticksPerMicrosecond);
}


void InitPPM()
{
	// set up timer with prescaler = 8 and CTC mode
	TCCR1B |= (1 << WGM12) | (1 << CS11);

	// initialize counter
	TCNT1 = 0;

	// initialize compare value
	OCR1A = CalculateTimeout(PPMFRAMESPACE);

	// enable compare interrupt
	TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{


	uint32_t timeout;
	digitalWrite(ppmOutputPin, HIGH);
	timeout = CalculateTimeout(PwmTimes[ppmOutputIndex]);
	digitalWrite(ppmOutputPin, LOW);
	OCR1A = timeout; //set new timer setpoint
	ppmOutputIndex = (ppmOutputIndex + 1) % 5;
}

inline void InitRegisters()
{
	//encoders
	DDRK = B00000000; //we will use A08-A15 (PORTK, PCInt2, analogue pins) for quadrature encoders
#ifdef UseInternalPullups
	PORTK = ~DDRK; //set internal pullup resistors on
#endif
	PCICR |= (1 << 2);
	PCMSK2 |= B11111111;

	//switches
	DDRB = (DDRB & B11000000);
#ifdef UseInternalPullups
	PORTB |= B00111111;
#endif
	PCICR |= (1 << 0);
	PCMSK2 |= B00111111;

};

const int8_t truth_table[] = { 0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0 };

inline int8_t FindTruth(uint8_t ov, uint8_t nv, uint8_t offset)
{
	uint8_t ol = (ov >> offset) & B00000011;
	uint8_t nw = (nv >> offset) & B00000011;

	uint8_t key = (ol << 2) | nw;

	return truth_table[key & 0b1111];

}


ISR(PCINT2_vect) //PORTK interrupt handler
{
	uint8_t prev = PrevQuadraturePins;
	uint8_t current = PINK;
	uint8_t changed = current & ~PrevQuadraturePins;

	if (changed & B00000011)//yaw
	{
		//quad0 changed
		PwmTimes[yawIndex] += FindTruth(prev, current, 0);
	}
	if (changed & B00001100)//roll
	{
		//quad1 changed
		PwmTimes[rollIndex] += FindTruth(prev, current, 2);

	}
	if (changed & B00110000)//pitch
	{
		//quad2 changed
		PwmTimes[pitchIndex] += FindTruth(prev, current, 4);
	}
	if (changed & B11000000)//menu
	{
		//quad3 changed
		//todo
		FindTruth(prev, current, 6);
	}
	PrevQuadraturePins = current;
}

ISR(PCINT0_vect) //PORTB interrupt handler
{
	uint8_t prev = PrevModePins;
	uint8_t current = PORTB & B00111111;
	uint8_t changed = current & ~prev;

	for (int i = 0; i < 7; i++)
	{
		uint8_t mask = (1 << i);
		if (changed & mask && current & mask)
		{
			modeButton.SelectOption(i);
			PwmTimes[modeIndex] = modeButton.SelectedOption().PPMTime;
			break;
		}
	}
	PrevModePins = current;
}


inline void UpdateMode()
{
	lcd.setCursor(modeCol, 1);
	lcd.print(modeButton.SelectedIndex());
}

inline void UpdateYaw()
{
	lcd.setCursor(yawCol, 1);
	lcd.print(PwmTimes[yawIndex]);

	//lcd.draw_horizontal_graph(1, yawCol, graphCols, pwmPercentage(PwmTimes[yawIndex]));
}

inline void UpdateRoll()
{
	lcd.setCursor(rollCol, 1);
	lcd.print(PwmTimes[rollIndex]);
	//lcd.draw_horizontal_graph(1, rollCol, graphCols, pwmPercentage(PwmTimes[rollIndex]));
}

inline void UpdatePitch()
{
	lcd.setCursor(pitchCol, 1);
	lcd.print(PwmTimes[pitchIndex]);
	//lcd.draw_horizontal_graph(1, pitchCol, graphCols, pwmPercentage(PwmTimes[pitchIndex]));
}


inline void InitInfo()
{
	lcd.clear();

	lcd.setCursor(modeCol, 0);
	lcd.print("M");

	lcd.setCursor(yawCol, 0);
	lcd.print("Yaw");

	lcd.setCursor(rollCol, 0);
	lcd.print("Roll");

	lcd.setCursor(pitchCol, 0);
	lcd.print("Pitch");
}



inline void InitMenu()
{
	lcd.clear();
}



void setup()
{

	pinMode(ppmOutputPin, OUTPUT);
	//Serial.begin(115200);
	//Serial.print("loading");

	lcd.init();

	lcd.display();

	lcd.backlight();
	lcd.setBacklight(10);

	lcd.clear();
	lcd.print("Loading...");
	lcd.init_bargraph(LCDI2C_HORIZONTAL_BAR_GRAPH);

	InitRegisters();
	InitPPM();
	PwmTimes[modeIndex] = modeButton.SelectedOption().PPMTime;

	InitInfo();

}



void loop()
{
	if (UIMode != PrevUIMode)
	{
		switch (UIMode)
		{
		case UIInfo: {
			InitInfo();
			break;
		}
		case UIMenu: {
			InitMenu();
			break;
		}
		}
		PrevUIMode = UIMode;
	}

	switch (UIMode)
	{
	case UIInfo: {
		UpdateMode();
		UpdateYaw();
		UpdateRoll();
		UpdatePitch();
		break;
	}
	case UIMenu: {
		break;
	}
	}
	delay(50);
}


