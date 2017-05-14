/*
  Homy Mouse

  Homy Mouse sketch - receives IR codes, sends 433Mhz OOK using Thingy.IO

  Copyright (c) 2017 Dusan Stojkovic
*/

#include <Thingy.h>

Thingy::Axon homyMouse;

// IR receiver
#include <IRremoteESP8266.h>
IRrecv irrecv(D2);
decode_results results;

// Eyes LED Section
void eyesLEDOn()
{
	for (int i = 0; i<= 255; i+=5)
	{
		analogWrite(D6, i);delay(5);
	}
}

void eyesLEDOff()
{
	for (int i = 255; i>= 0; i-=5)
	{
		analogWrite(D6, i);delay(5);
	}
}

// Tail LED Section
void tailLEDOn()
{
	for (int i = 0; i<= 255; i+=5)
	{
		analogWrite(D5, i);delay(5);
	}
}

void tailLEDOff()
{
	for (int i = 255; i>= 0; i-=5)
	{
		analogWrite(D5, i);delay(5);
	}
}

// 433 Mhz OOK Section
void turn(long deviceId, int button, byte on)
{
  int channel = 7;
  if (button == 1) channel = 0;
  if (button == 2) channel = 4;
  if (button == 3) channel = 2;
  if (button == 4) channel = 1;

	for (int rep=0; rep<(on=='X'?100:3); rep++)
	{
		digitalWrite(D3,HIGH);
	  delayMicroseconds(300);
	  digitalWrite(D3,LOW);
	  delayMicroseconds(31*300);
		for(int b = 19; b > -1; b--)
		{
			bool bit = (deviceId >> b) & 0b1;
			digitalWrite(D3,HIGH);
			delayMicroseconds((bit ? 1 : 3)*300);
			digitalWrite(D3,LOW);
			delayMicroseconds((bit ? 3 : 1)*300);
		}
		for(int b = 3; b > -1; b--)
		{
			bool bit = ((((on=='1') ? 0b0000 : 0b1000) | (channel & 0b0111)) >> b) & 0b1;
			digitalWrite(D3,HIGH);
			delayMicroseconds((bit ? 1 : 3)*300);
			digitalWrite(D3,LOW);
			delayMicroseconds((bit ? 3 : 1)*300);
		}
	}
}

void sendHomeEasyCode(long C)
{
  for (int rep=0; rep<4; rep++)
  {
    // sync
    digitalWrite(D3,HIGH);
    delayMicroseconds(347);
    digitalWrite(D3,LOW);
    delayMicroseconds(9943);

    for(int b = 23; b > -1; b--)
    {
      bool bit = (C >> b) & 0b1;

      digitalWrite(D3,HIGH);
      delayMicroseconds(bit ? 930 : 347);
      digitalWrite(D3,LOW);
      delayMicroseconds(!bit ? 930 : 347);
    }
  }
}

// Lights Section
void handle_light(const Thingy::Stream& stream, const String& value)
{
	int button = (stream.uid == "eef3a64a") ? 1 : (stream.uid == "7e414c49") ? 2 : (stream.uid == "46af021f") ? 3 : (stream.uid == "a6517ee0") ? 4 : 0;
	byte on = (value == "ON") ? '1': (value == "OFF") ? '0': 'X';
	if (button == 0 && on=='1') on = '0';
	if (button == 0 && on=='0') on = '1';

	eyesLEDOn();
	turn(912938, button, on);
	eyesLEDOff();
}

void setup()
{
	// Setup console
	Serial.begin(115200);
	delay(10);

	Serial.setDebugOutput(true);

	Serial << endl << endl  << F("Homy.Mouse") << " - " << __DATE__ << " " << __TIME__ << endl << "#" << endl << endl;

	homyMouse.setup();

	if (!homyMouse.begin())
	{
		Serial << F("Homy - Fatal error. Please restart.") << endl << endl << endl;
		return;
	}

	// settingg handles
	homyMouse["73e514b2"].setHandle(handle_light);
	homyMouse["eef3a64a"].setHandle(handle_light);
	homyMouse["7e414c49"].setHandle(handle_light);
	homyMouse["46af021f"].setHandle(handle_light);
	homyMouse["a6517ee0"].setHandle(handle_light);

	// eyes LED stream
	homyMouse["181d66cc"].setHandle([](const Thingy::Stream& stream, const String& value){
		if (value == "ON")
			eyesLEDOn();
		if (value == "OFF")
			eyesLEDOff();
	});

	// tail LED stream
	homyMouse["ce9ec25e"].setHandle([](const Thingy::Stream& stream, const String& value){
		if (value == "ON")
			tailLEDOn();
		if (value == "OFF")
			tailLEDOff();
	});

	// IR receiver start
	irrecv.enableIRIn();

	// Init light remote (443MHz Transmiter)
  pinMode(D3,OUTPUT); digitalWrite(D3,0);

	// Eyes LED
	pinMode(D6,OUTPUT);

	// Tail LED
	pinMode(D5,OUTPUT);
}

String light_command="ON";

void loop()
{
	homyMouse.loop();

	if (!homyMouse.connected()) return;

  // IR receiver check
	if (irrecv.decode(&results))
	{
		String ir_old = homyMouse["3296d346"].value;
		String ir_new = String(results.value);
		homyMouse["3296d346"].put(ir_new);
    irrecv.resume();

		// invert light command
		light_command = (ir_new == ir_old) ? (light_command == "OFF" ? "ON" : "OFF") : light_command;

		// red for 1
		if (ir_new == "516673321")  homyMouse["eef3a64a"].put(light_command);
		// green for 2
		if (ir_new == "2021205805") homyMouse["7e414c49"].put(light_command);
		// yellow for 3
		if (ir_new == "723345733")  homyMouse["46af021f"].put(light_command);
		// blue for 4
		if (ir_new == "1742852553") homyMouse["a6517ee0"].put(light_command);

		// up for all ON
		if (ir_new == "1819431269") homyMouse["73e514b2"].put("ON");
		// down for all OFF
		if (ir_new == "477692577") homyMouse["73e514b2"].put("OFF");
  }
}
