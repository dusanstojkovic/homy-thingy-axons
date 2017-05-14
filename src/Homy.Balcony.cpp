/*
  Homy Balcony

  Homy Balcony sketch - Handels balcony irrigation system using Thingy.IO

  Copyright (c) 2017 Dusan Stojkovic
*/

#include <Thingy.h>
#include <Ticker.h>
#include <Button.h>
#include <Streaming.h>

#define PIN_VALVE_1 D1 //5 - P2 D
#define PIN_BUTTON_1 D5 //A1 - P2 A
#define PIN_VALVE_2 D2 //6 - P3 D
#define PIN_BUTTON_2 D6 //A2 - P3 A
#define PIN_PUMP D3 //7 P4 - D
#define PIN_BUTTON_PUMP D7 //A3 P4 - A
#define PIN_FLOWMETER_DATA D4 // P? - IRQ

#define SUBSCRIPTION_INTERVAL 5
#define FLOW_METER_UPDATE_RATE 250
#define FLOW_METER_CALIBRATION_FACTOR 0.36 // ml/pulse

Thingy::Axon axonBalcony;
Ticker ticker_pumping_status;
Ticker ticker_flow_meter_status;

// Plant 1
bool plant1Active = false;
Button plant1Button(PIN_BUTTON_1);

void plant1On()
{
	plant1Active = true;
	digitalWrite(PIN_VALVE_1, LOW);
	delay(10);
	axonBalcony["Line 1 Status"].put("OPEN");
	LOG << F("Plant 1 On") << endl;
}
void plant1Off()
{
	plant1Active = false;
	digitalWrite(PIN_VALVE_1, HIGH);
	delay(10);
	axonBalcony["Line 1 Status"].put("CLOSE");
	LOG << F("Plant 1 Off") << endl;
}

// Plant 2
bool plant2Active = false;
Button plant2Button(PIN_BUTTON_2);

void plant2On()
{
	plant2Active = true;
	digitalWrite(PIN_VALVE_2, LOW);
	delay(10);
	axonBalcony["Line 2 Status"].put("OPEN");
	LOG << F("Plant 2 On") << endl;
}

void plant2Off()
{
	plant2Active = false;
	digitalWrite(PIN_VALVE_2, HIGH);
	delay(10);
	axonBalcony["Line 2 Status"].put("CLOSE");
	LOG << F("Plant 2 Off") << endl;
}

// Flow Meter
volatile unsigned long flowMeterPulseCount;
unsigned long flowMeterPulses;
unsigned long flowMeterOldTime;
unsigned long flowMeterStartTime;
float flowMeterRate = 0.0;
float flowMeterQuantity = 0.0;

void flowMeterPulseCounter()
{
	flowMeterPulseCount++;
}

void flowMeterProcess()
{
	unsigned long lPulseCount = flowMeterPulseCount;
	flowMeterPulseCount -= lPulseCount;
	flowMeterPulses += lPulseCount;

	flowMeterRate = (1000.0  / (millis() - flowMeterOldTime)) * lPulseCount * FLOW_METER_CALIBRATION_FACTOR; // in L/min
	flowMeterOldTime = millis();

	flowMeterQuantity = flowMeterPulses * FLOW_METER_CALIBRATION_FACTOR;

	LOG << "   ... [" << (millis()-flowMeterStartTime) << "] Flow = " << flowMeterRate << " ml/s :: "<< (3.6*flowMeterRate) <<" L/h :: Pulses = "<< lPulseCount << "   -   Quantity = " << flowMeterQuantity << " ml :: Pulses = "<< flowMeterPulses << endl;
}

void flowMeterOn()
{
	flowMeterPulses = 0;
	flowMeterPulseCount = 0;
	flowMeterOldTime = millis();
	flowMeterStartTime = millis();
	flowMeterRate = 0.0;
	flowMeterQuantity = 0.0;
	ticker_flow_meter_status.attach_ms(FLOW_METER_UPDATE_RATE, flowMeterProcess);
	attachInterrupt(digitalPinToInterrupt(PIN_FLOWMETER_DATA), flowMeterPulseCounter, FALLING);
	LOG << F("Flow Meter On") << endl;
}

void flowMeterOff()
{
	detachInterrupt(digitalPinToInterrupt(PIN_FLOWMETER_DATA));
	ticker_flow_meter_status.detach();
	flowMeterProcess();
	flowMeterQuantity = 0.0;
	flowMeterRate = 0.0;
	flowMeterStartTime = 0;
	flowMeterOldTime = 0;
	flowMeterPulseCount = 0;
	flowMeterPulses = 0;
	LOG << F("Flow Meter Off") << endl;
}

// Pump
bool pumpActive = false;
Button pumpButton(PIN_BUTTON_PUMP);

void pumpOn()
{
	flowMeterOn();
	pumpActive = true;
	digitalWrite(PIN_PUMP, LOW);
	delay(10);
	axonBalcony["Pump Status"].put("ON");
	LOG << F("Pump On") << endl;
}

void pumpOff()
{
	pumpActive = false;
	digitalWrite(PIN_PUMP, HIGH);
	delay(10);
	flowMeterOff();
	axonBalcony["Pump Status"].put("OFF");
	LOG << F("Pump Off") << endl;
}

bool pumping = false;
float pumpingQuantity = 0.0;

void pumpingStatusPut()
{
	LOG << F("Pumping (Duration = ") << (millis()-flowMeterStartTime) << F(" ms :: Flow = ") << flowMeterRate << F(" ml/s :: ") << (3.6*flowMeterRate) << F(" L/h :: Quantity = ") << flowMeterQuantity << F(" ml :: Pulses = ") << flowMeterPulses << F(") @ ") << millis();
	axonBalcony["Quantity - Actual"].put(String(flowMeterQuantity,2));
	axonBalcony["Flow"].put(String(flowMeterRate,2));
}

void pumpingStop()
{
	pumping = false;
	pumpOff();
	plant1Off();
	plant2Off();
	ticker_pumping_status.detach();
	pumpingStatusPut();

	axonBalcony["Pumping Status"].put("OFF");
	LOG << F("Pumping stopped") << endl;
}

void pumpingStart()
{
	if (pumping)
		pumpingStop();
	pumpOn();
	pumping = true;
	LOG << F("Pumping ") << pumpingQuantity << F(" ml starting... @ ")  << millis() << endl;
	axonBalcony["Pumping Status"].put("ON");
	ticker_pumping_status.attach(SUBSCRIPTION_INTERVAL, pumpingStatusPut);
}

void setup()
{
  // Setup console
	Serial.begin(115200);
	delay(10);

	Serial << endl << endl  << F("Homy.Balcony") << " - " << __DATE__ << " " << __TIME__ << endl << "#" << endl << endl;

	// setup buttons
	plant1Button.begin();
	plant2Button.begin();
	pumpButton.begin();

	// setup valves and pump
	pinMode(PIN_VALVE_1, OUTPUT); digitalWrite(PIN_VALVE_1, HIGH);
	pinMode(PIN_VALVE_2, OUTPUT); digitalWrite(PIN_VALVE_2, HIGH);
	pinMode(PIN_PUMP, OUTPUT);   digitalWrite(PIN_PUMP, HIGH);

	// init valves and pump
	digitalWrite(PIN_VALVE_1, LOW); delay(10); digitalWrite(PIN_VALVE_2, LOW); delay(10);
	digitalWrite(PIN_PUMP, LOW); delay(10);
	delay(100);
	digitalWrite(PIN_PUMP, HIGH);delay(10);

	// Thingy
	axonBalcony.setup();

	/*
	// Persisted state
  if (SPIFFS.exists("/state.on"))
  {
    Serial << F("Homy.Balcony - ON (?)") << endl;
    on();
  }

  if (SPIFFS.exists("/state.off"))
  {
    Serial << F("Homy.Balcony - OFF(?)") << endl;
    off();
  }
	*/

  //
	if (!axonBalcony.begin())
	{
		Serial << F("Homy.Balcony - Fatal error. Please restart.") << endl << endl << endl;
		return;
	}

	digitalWrite(PIN_VALVE_1, HIGH); delay(10); digitalWrite(PIN_VALVE_2, HIGH); delay(10);

	// setting handles
	axonBalcony["Pumping Command"].setHandle([](const Thingy::Stream& sender, const String& value){
		if (value == "ON") pumpingStart();
		if (value == "OFF") pumpingStop();
	});

	axonBalcony["Quantity - Target"].setHandle([](const Thingy::Stream& sender, const String& value){
		pumpingQuantity = value.toFloat();
	});

	axonBalcony["Pump Command"].setHandle([](const Thingy::Stream& sender, const String& value){
		if (value == "ON") pumpOn();
		if (value == "OFF") pumpOff();
		if (value == "OFF" && pumping) pumpingStop();
	});

	axonBalcony["Line 1 Command"].setHandle([](const Thingy::Stream& sender, const String& value){
		if (value == "OPEN") plant1On();
		if (value == "CLOSE") plant1Off();
	});

	axonBalcony["Line 2 Command"].setHandle([](const Thingy::Stream& sender, const String& value){
		if (value == "OPEN") plant2On();
		if (value == "CLOSE") plant2Off();
	});

	Serial << F("Ready!") << endl;
	delay(100);
}

void loop()
{
	axonBalcony.loop();

	// handle buttons
	bool pumpOnAction = !pumpActive && pumpButton.released();
	bool pumpOffAction = pumpActive && pumpButton.released();
	bool plant1OnAction = !plant1Active && plant1Button.released();
	bool plant1OffAction = plant1Active && (plant1Button.released() || pumpOffAction);
	bool plant2OnAction = !plant2Active && plant2Button.released();
	bool plant2OffAction = plant2Active && (plant2Button.released() || pumpOffAction);

	// handle pumping conditions
	bool pumpingTerminate = pumping && (pumpingQuantity-flowMeterQuantity <= 0.0);

	// issuing action
	if (pumpingTerminate) pumpingStop();

	if (pumpOnAction) pumpOn();
	if (pumpOffAction) pumpOff();
	if (pumpOffAction && pumping) pumpingStop();

	if (plant1OnAction) plant1On();
	if (plant1OffAction) plant1Off();

	if (plant2OnAction) plant2On();
	if (plant2OffAction) plant2Off();
}
