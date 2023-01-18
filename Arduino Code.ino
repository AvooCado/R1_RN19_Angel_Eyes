#define FASTLED_INTERNAL // for disabling the pragma message
#include <FastLED.h> // include FastLED library

#define NUM_HEADLIGHT_LEDS 52 // number of leds inside the headlight (24 inner ring + 24 outer ring + 4 lens)
#define NUM_UNDERGLOW_LEDS 13 // number of leds for underglow

#define NUM_STRIPS 3 // number of leds strips (1 headlight + 1 headlight + 1 underglowstrip)

CRGB headlightr[NUM_HEADLIGHT_LEDS]; // notice the r for right side
CRGBSet ledsr(headlightr, NUM_HEADLIGHT_LEDS);
CRGBSet innerringr(ledsr(0,23));
CRGBSet outerringr(ledsr(24,47));
CRGBSet eyer(ledsr(48,51));


CRGB headlightl[NUM_HEADLIGHT_LEDS]; // notice the l for left side
CRGB underglow[NUM_UNDERGLOW_LEDS];

CLEDController *controllers[NUM_STRIPS]; // needed for controlling multiple leds lookup fastled wiki
struct CRGB * rightside[] ={innerringr, outerringr, eyer}; 

// ===================== Constants =======================

const uint8_t gbrightness = 255; // global brightness setting
const uint8_t ringinput = 4; // input pin for double Ring pins
const uint8_t eyeinput = 5; // input pin for 4 leds in the "eye" aka the lens (notice the underglow part is connected to this pin)

#define numberringleds 24
#define numbereyeleds 4

// ===================== Variables =======================

uint8_t ringstate = HIGH; // high because input pin is triggering on low
uint8_t eyestate = HIGH; // same as above

uint8_t ringon = 0;
uint8_t eyeon = 0;
uint8_t ringmodecounter = 0; // 0 = standardmode etc.
uint8_t eyemodecounter = 0;

uint8_t hue = 0; // color variable for fastled lookup HSV colors
uint8_t lastbrightnessring = 0;
uint8_t dimring = 60;
uint8_t lastbrightnesseye = 0;
uint8_t lastbrightnessunderglow = 0;

uint8_t dotring = 0;
uint8_t doteye = 0;

uint8_t sweepdonering = 0;


void setup()
{
	Serial.begin(9600);

	controllers[0] = &FastLED.addLeds<NEOPIXEL,7>(headlightr, NUM_HEADLIGHT_LEDS);
	controllers[1] = &FastLED.addLeds<NEOPIXEL,8>(headlightl, NUM_HEADLIGHT_LEDS);
	controllers[2] = &FastLED.addLeds<NEOPIXEL,9>(underglow, NUM_UNDERGLOW_LEDS);

	fill_solid(headlightr, NUM_HEADLIGHT_LEDS, CRGB::Black);
	fill_solid(headlightl, NUM_HEADLIGHT_LEDS, CRGB::Black);
	fill_solid(underglow, NUM_UNDERGLOW_LEDS, CRGB::Black);
    controllers[0]->showLeds(gbrightness);
    controllers[1]->showLeds(gbrightness);
    controllers[2]->showLeds(gbrightness);

    pinMode(ringinput, INPUT_PULLUP);
    pinMode(eyeinput, INPUT_PULLUP);	
}

void loop()
{
	ringstate = digitalRead(ringinput);
  	eyestate = digitalRead(eyeinput);

  	Serial.println(eyestate);

  	headlightnormal();
  	eyenormal();
}

void headlightnormal()
{
	if (ringon == 0 && ringstate == LOW)
	{
		if (sweepdonering == 0 && ringmodecounter == 0)
		{	
			EVERY_N_MILLISECONDS(30)
			{
				fill_solid(rightside[0], dotring, CHSV(201, 255, gbrightness));
				fill_solid(rightside[1], dotring, CHSV(201, 255, gbrightness));
				lastbrightnessring = gbrightness;
				if (dotring < numberringleds)
				{
					dotring++;
				}
				else if (dotring == numberringleds)
				{
					sweepdonering = 1;
				}
				controllers[0]->showLeds();
			}
		}
		else if (sweepdonering == 1 && ringmodecounter == 0)
		{	
			EVERY_N_MILLISECONDS(30)
			{
				headlightr[dotring-1] = CHSV(201, 0, gbrightness);
				headlightr[(numberringleds*2)-dotring-1] = CHSV(201, 0, gbrightness);
				lastbrightnessring = gbrightness;
				if (dotring > 0)
				{
					dotring--;
				}
				else if (dotring == 0)
				{
					sweepdonering = 2;
				}
				controllers[0]->showLeds();
			}
		}
		else if (sweepdonering == 2 && ringmodecounter == 0)
		{
			EVERY_N_MILLISECONDS(5)
			{
				fill_solid(rightside[0], numberringleds, CHSV(201, 0, lastbrightnessring));
				fill_solid(rightside[1], numberringleds, CHSV(201, 0, lastbrightnessring));
				if (lastbrightnessring > dimring)
				{
					lastbrightnessring--;
				}
				else if (lastbrightnessring == 0)
				{
					sweepdonering = 3;
				}
				controllers[0]->showLeds();
			}
		}
	}
	else if (ringstate == HIGH)
	{
		EVERY_N_MILLISECONDS(5)
		{
			fill_solid(rightside[0], numberringleds, CHSV(0, 0, lastbrightnessring));
			fill_solid(rightside[1], numberringleds, CHSV(0, 0, lastbrightnessring));
			if (lastbrightnessring > 0)
			{
				lastbrightnessring--;
			}
			else if(lastbrightnessring == 0)
			{
				ringon = 0;
				ringmodecounter = 0;
				dotring = 0;
				sweepdonering = 0;
			}
			controllers[0]->showLeds();
		}
	}
}

void eyenormal()
{
	if (eyeon == 0 && eyestate == LOW)
	{
		EVERY_N_MILLISECONDS(5)
		{
			fill_solid(rightside[2], numbereyeleds, CHSV(201, 255, lastbrightnesseye));
			if (lastbrightnesseye < gbrightness && eyemodecounter == 0)
			{
				lastbrightnesseye++;
			}
			else if (lastbrightnesseye == gbrightness)
			{
				eyeon = 1;
			}
			controllers[0]->showLeds();
		}
	}
	else if (eyestate == HIGH)
	{
		EVERY_N_MILLISECONDS(5)
		{
			fill_solid(rightside[2], numbereyeleds, CHSV(201, 255, lastbrightnesseye));
			eyeon = 0;
			eyemodecounter = 0;
			if (lastbrightnesseye > 15)
			{
				lastbrightnesseye--;
			}
			controllers[0]->showLeds();
		}
	}
}
