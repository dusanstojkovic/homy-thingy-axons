/*
  Homy Roomba

  Homy Roomba sketch - Controls Roomba cleaning robot using Thingy.IO

  Copyright (c) 2017 Dusan Stojkovic
*/

#include <Thingy.h>
#include <Roomba.h>

Thingy::Axon axon;
Roomba roomba(5,6,7);

bool statusGreen = false;
bool statusRed = false;
bool spotLed = false;
bool cleanLed = false;
bool maxLed=false;
bool dirtDetectLed = false;

byte powerColor = 127;
byte powerIntensity = 127;

int driveVelocity = 200;
int driveRadius = 0;

bool mainBrush = false;
bool vacuum = false;
bool sideBrush = false;

void setVelocity(int velocity)
{
	driveVelocity = velocity;
	axon["Velocity"].put(String(velocity));
}

void setRadius(int radius)
{
		driveRadius = radius;
		axon["Radius"].put(String(radius));
}


void handle_command(const Thingy::Stream& stream, const String& value)
{
	if (value == "WKP")
	{
		roomba.wakeUp();
	}
	else if (value == "STA")
	{
		roomba.start();
	}
	else if (value == "CNT")
	{
		roomba.control();
	}
	else if (value == "SAF")
	{
		roomba.safe();
	}
	else if (value == "FUL")
	{
		roomba.full();
	}
	else if (value == "POW")
	{
		roomba.power();
	}
	else if (value == "RST")
	{
		roomba.reset();
	}
	else if (value == "SPT")
	{
		roomba.spot();
	}
	else if (value == "CLN")
	{
		roomba.clean();
	}
	else if (value == "MAX")
	{
		roomba.max();
	}
	else if (value == "MM*")
	{
		mainBrush = !mainBrush;
		roomba.motors(mainBrush, vacuum, sideBrush);
	}
	else if (value == "MV*")
	{
		vacuum = !vacuum;
		roomba.motors(mainBrush, vacuum, sideBrush);
	}
	else if (value == "MS*")
	{
		sideBrush = !sideBrush;
		roomba.motors(mainBrush, vacuum, sideBrush);
	}
	else if (value == "LG*")
	{
		statusGreen = !statusGreen;
		roomba.leds(statusGreen, statusRed, spotLed, cleanLed, maxLed, dirtDetectLed, powerColor, powerIntensity);
	}
	else if (value == "LR*")
	{
		statusRed = !statusRed;
		roomba.leds(statusGreen, statusRed, spotLed, cleanLed, maxLed, dirtDetectLed, powerColor, powerIntensity);
	}
	else if (value == "LS*")
	{
		spotLed = !spotLed;
		roomba.leds(statusGreen, statusRed, spotLed, cleanLed, maxLed, dirtDetectLed, powerColor, powerIntensity);
	}
	else if (value == "LC*")
	{
		cleanLed = !cleanLed;
		roomba.leds(statusGreen, statusRed, spotLed, cleanLed, maxLed, dirtDetectLed, powerColor, powerIntensity);
	}
	else if (value == "LM*")
	{
		maxLed = !maxLed;
		roomba.leds(statusGreen, statusRed, spotLed, cleanLed, maxLed, dirtDetectLed, powerColor, powerIntensity);
	}
	else if (value == "LD*")
	{
		dirtDetectLed = !dirtDetectLed;
		roomba.leds(statusGreen, statusRed, spotLed, cleanLed, maxLed, dirtDetectLed, powerColor, powerIntensity);
	}
	else if (value == "FSD")
	{
		roomba.forceSeekingDock();
	}
	else if (value == "DRV")
	{
		roomba.drive(driveVelocity, driveRadius);
	}
	else if (value == "DGF")
	{
		setVelocity(abs(driveVelocity));
		setRadius(0);
		roomba.drive(driveVelocity, driveRadius);
	}
	else if (value == "DGB")
	{
		setVelocity(-abs(driveVelocity));
		setRadius(0);
		roomba.drive(driveVelocity, driveRadius);
	}
	else if (value == "DSL")
	{
		setVelocity(abs(driveVelocity));
		setRadius(1);
		roomba.drive(driveVelocity, driveRadius);
	}
	else if (value == "DSR")
	{
		setVelocity(abs(driveVelocity));
		setRadius(-1);
		roomba.drive(driveVelocity, driveRadius);
	}
	else if (value == "DST")
	{
		setVelocity(0);
		setRadius(0);
		roomba.drive(driveVelocity, driveRadius);
	}
	else if (value == "DV+")
	{
		setVelocity(driveVelocity + 10);
		roomba.drive(driveVelocity, driveRadius);
	}
	else if (value == "DV-")
	{
		setVelocity(driveVelocity - 10);
		roomba.drive(driveVelocity, driveRadius);
	}
	else if (value == "DR+")
	{
		setRadius(driveRadius + 10);
		roomba.drive(driveVelocity, driveRadius);
	}
	else if (value == "DR-")
	{
		setRadius(driveRadius - 10);
		roomba.drive(driveVelocity, driveRadius);
	}
}

void setup()
{
	// Setup console
	Serial.begin(115200);
	delay(10);

	Serial.setDebugOutput(true);

	Serial << endl << endl  << F("Homy.Roomba") << " - " << __DATE__ << " " << __TIME__ << endl << "#" << endl << endl;

	// setting up connection to Roomba
	#if RESETSERIALPORT
		roomba.resetbaud();
	#else
		roomba.setup();
	#endif

	// welcome leds
	bool l1 = true; bool l2=true;
	for (int i=0; i<32; i++)
	{
		roomba.leds(false, true, l1, false, false, l2, i*8, 255);
		delay(50);
		l1 = ! l1; l2= !l2;
	}
	// setting up connection to Thingy
	axon.setup();

	if (!axon.begin())
	{
		Serial << F("Homy - Fatal error. Please restart.") << endl << endl << endl;
		return;
	}

	// settingg handles
	axon["Command"].setHandle(handle_command);
}

void loop()
{
	roomba.loop();

	axon.loop();
	if (!axon.connected()) return;
}
