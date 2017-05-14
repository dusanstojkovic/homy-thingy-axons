/*
  Homy Room

  Homy Room sketch - monitors humidity and temperature  using Thingy.IO

  Copyright (c) 2017 Dusan Stojkovic
*/

#include <Thingy.h>
#include "DHT.h"

Thingy::Axon homyNode;

DHT dht(D4, DHT22);

void setup() {
//	Serial.begin(115200);
	//Serial.setDebugOutput(true);

	CONS << endl << endl  << F("Homy.Room") << " - " << __DATE__ << " " << __TIME__ << endl << "#" << endl << endl;

	//homyNode.settings_default();
	//homyNode.set_uid("").set_sid("").settings_save();
	homyNode.setup();

	if (!homyNode.begin())
	{
		CONS << F("Homy - Fatal error. Please restart.") << endl << endl << endl;
		return;
	}

	dht.begin();
}

void loop()
{
	homyNode.loop();

	if (!homyNode.connected()) return;

	homyNode["Humidity"].put("-");
	homyNode["Temperature"].put("-");

  unsigned int bat_a0 = analogRead(A0);
	float bat_V = bat_a0 /198.27;

	CONS << "Battery : " << String(bat_V,2) << " V   -   " << String(bat_a0, DEC) << endl;

	homyNode["Battery"].put(String(bat_a0, DEC));
	homyNode["Battery (V)"].put(String(bat_V,2)+" V");

	float h = dht.readHumidity();
  float t = dht.readTemperature();

	if (!isnan(h) && !isnan(t))
	{

		homyNode["Humidity"].put(String(h,2));
		homyNode["Temperature"].put(String(t,2));
  }

	CONS << F("Homy - Going for a sleep") << endl << endl << endl;

	ESP.deepSleep(600 *1000000UL); // 600 s
}
