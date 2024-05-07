//ESP21 Wrover Module!
#include "WifiPass.h"	//define wifi SSID & pass
#include <SerialWebLog.h>
#include "EmonLib.h"
#include <ArduinoOTA.h>

/////////////////////////////////////
#define HOST_NAME "PowerMeter"
#define HOME_VOLTAGE 230.0
// Force EmonLib to use 10bit ADC resolution
#define ADC_INPUT 	A0
#define ADC_BITS    10
#define ADC_COUNTS  (1<<ADC_BITS)
/////////////////////////////////////

SerialWebLog mylog;
EnergyMonitor emon1;
unsigned long lastMeasurement = 0;
unsigned long loopCount = 0;
float watts = 0;
bool logWatts = false;

void handleWatts();
void logOn();
void logOff();

void setup() {

	uint8_t bssid[6] = {0x60,0x8d,0x26,0xeb,0xe2,0x8b}; //60:8d:26:eb:e2:8b Orangebox6
	mylog.setup(HOST_NAME, ssid, password, bssid);

	//mylog.setup(HOST_NAME, ssid, password);

	//add an extra endpoint
	mylog.getServer()->on("/watts", handleWatts);
	mylog.addHtmlExtraMenuOption("Watts", "/watts");

	mylog.getServer()->on("/logOn", logOn);
	mylog.addHtmlExtraMenuOption("LogOn", "/logOn");

	mylog.getServer()->on("/logOff", logOff);
	mylog.addHtmlExtraMenuOption("LogOff", "/logOff");


	emon1.current(ADC_INPUT, 100);

	ArduinoOTA.setHostname(HOST_NAME);
	ArduinoOTA.setRebootOnSuccess(true);
	ArduinoOTA.begin();
	mylog.print("ready!\n");
}

void loop() {
	
	ArduinoOTA.handle();
	mylog.update();

	unsigned long currentMillis = millis();
	if(currentMillis - lastMeasurement > 1000){
		lastMeasurement = currentMillis;
		double amps = emon1.calcIrms(1480); // Calculate Irms only
	    watts = amps * HOME_VOLTAGE;
		//pos fact incorrect compensation (tested with a 800w microwave)
		watts = watts * 1.077 - 60;
		if (watts < 0) watts = 0;
		if(logWatts) mylog.printf("watts: %f\n", watts);
	}
}


void handleWatts(){
	String metrics = GenerateMetrics();
	mylog.getServer()->send(200, "text/plain", metrics);
}

void logOn(){
	mylog.getServer()->send(200, "text/plain", "log ON OK!");
	logWatts = true;
	mylog.print("Turn Log ON\n");
}
void logOff(){
	mylog.getServer()->send(200, "text/plain", "log OFF OK!");
	logWatts = false;
	mylog.print("Turn Log OFF\n");
}


String GenerateMetrics() {
	String message = "";
	String idString = "{}";
	message += "# HELP watts Watts\n";
	message += "# TYPE temp gauge\n";
	message += "watts";
	message += idString;
	message += String(watts, 1);
	message += "\n";
	return message;
}