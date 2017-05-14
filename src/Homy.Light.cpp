/*
  Homy Light

  Homy Light sketch - Handles Corridor light using Thingy.IO

  Copyright (c) 2017 Dusan Stojkovic
*/

#define   PIN_BUTTON             D3
#define   PIN_LIGHT              D7
#define   PIN_RELAY              D6

#include <Thingy.h>
#include <Button.h>

typedef enum { NONE, ON, OFF, TOGGLE } Action;

Thingy::Axon homyLight;

Button button(PIN_BUTTON);
Button light_pin(PIN_LIGHT);

Action action = NONE;

bool light;

void toggle()
{
  light = (light_pin.read() == Button::RELEASED);
  Serial << F("Homy.Light.Corridor - SWITCHING - ") << (light ? F("OFF") : F("ON")) << endl;
  int state = digitalRead(PIN_RELAY);
  digitalWrite(PIN_RELAY, !state);
}

void on()
{
  light = (light_pin.read() == Button::RELEASED);
  if (!light) toggle();
}

void off()
{
  light  = !(light_pin.read() == Button::PRESSED);
  if (light) toggle();
}

void sync()
{
  light  = (light_pin.read() == Button::RELEASED);
  if (light)
  {
      homyLight["State"].put("ON", 1, true);
      SPIFFS.rename("/state.off", "/state.on");
      Serial << F("Homy.Light.Corridor - ON") << endl;
  }
  else
  {
    homyLight["State"].put("OFF", 1, true);
    SPIFFS.rename("/state.on", "/state.off");
    Serial << F("Homy.Light.Corridor - OFF") << endl;
  }
}

void handle_command(const Thingy::Stream& stream, const String& value)
{
  if (value == "ON")
    action = ON;
  if (value == "OFF")
    action = OFF;
  if (value == "TOGGLE")
    action = TOGGLE;
}

void setup()
{
	// Setup console
	Serial.begin(115200);
	delay(10);

	Serial << endl << endl  << F("Homy.Light.Corridor") << " - " << __DATE__ << " " << __TIME__ << endl << "#" << endl << endl;

  // Button
  button.begin();

  //Light
  light_pin.begin();
  pinMode(PIN_LIGHT, INPUT);

  // Relay
  pinMode(PIN_RELAY, OUTPUT);

  // Thingy
	homyLight.setup();

  // Persisted state
  if (SPIFFS.exists("/state.on"))
  {
    Serial << F("Homy.Light.Corridor - ON (?)") << endl;
    on();
  }

  if (SPIFFS.exists("/state.off"))
  {
    Serial << F("Homy.Light.Corridor - OFF(?)") << endl;
    off();
  }

  //
	if (!homyLight.begin())
	{
		Serial << F("Homy.Light.Corridor - Fatal error. Please restart.") << endl << endl << endl;
		return;
	}

  sync();

	// settingg handles
	homyLight["Command"].setHandle(handle_command);
}

void loop()
{
  homyLight.loop();

  if (button.toggled())
    action = TOGGLE;

  if (action == ON)
    on();
  if (action == OFF)
    off();
  if (action == TOGGLE)
    toggle();

  if (action != NONE)
    action = NONE;

  if (light_pin.toggled())
    sync();
}
