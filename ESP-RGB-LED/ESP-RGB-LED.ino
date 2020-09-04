#include <WiFi.h>
#include "time.h"

struct Color {
	int red;
	int green;
	int blue;
};

Color currentColor = {0, 0, 0};
Color targetColor = {0, 0, 0};

const char* ssid = "ssid";
const char* password = "password";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600; // GMT+7
const int   daylightOffset_sec = 0 * 3600; // no DST

const int redPin = 13;
const int greenPin = 12;
const int bluePin = 14;

// Setting PWM frequency, channels and bit resolution
const int freq = 10000; // 10 KHz
const int resolution = 8; // Bit resolution 2^8 = 256
const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 2;

void setup() {
	Serial.begin(115200);

	// configure LED PWM functionalitites
	ledcSetup(redChannel, freq, resolution);
	ledcSetup(greenChannel, freq, resolution);
	ledcSetup(blueChannel, freq, resolution);

	// attach the channel to the GPIO to be controlled
	ledcAttachPin(redPin, redChannel);
	ledcAttachPin(greenPin, greenChannel);
	ledcAttachPin(bluePin, blueChannel);
	
	//connect to WiFi
	Serial.printf("Connecting to %s ", ssid);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println(" CONNECTED");

	//init and get the time
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	struct tm timeinfo;
	while(!getLocalTime(&timeinfo)) { // wait until we can get time
		Serial.println("Failed to obtain time");
		delay(500);
	}

	//disconnect WiFi as it's no longer needed
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
}

void loop() {
	struct tm timeinfo;
	if(!getLocalTime(&timeinfo)) {
		Serial.println("Failed to obtain time");
	}
//	Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

	setColorFromHour(timeinfo.tm_hour);
	fadeColor();

	delay(1000);
}

void setColorFromHour(int h) {
	switch(h) {
		case 0 ... 6: // midnight to 6am
			targetColor = {180, 0, 0};
			break;
		case 7:
			targetColor = {225, 50, 50};
			break;
		case 8:
			targetColor = {175, 0, 100};
			break;
		case 9:
			targetColor = {150, 25, 150};
			break;
		case 17:
			targetColor = {200, 200, 0};
			break;
		case 18:
			targetColor = {225, 180, 0};
			break;
		case 19:
			targetColor = {225, 153, 0};
			break;
		case 20:
			targetColor = {255, 125, 0};
			break;
		case 21:
			targetColor = {255, 100, 0};
			break;
		case 22:
			targetColor = {255, 50, 0};
			break;
		case 23:
			targetColor = {255, 0, 0};
			break;
		default:
			Serial.println("off"); // 0, 0 , 0
			targetColor = {0, 0, 0};
			break;
	}
}

void fadeColor() {
	if(targetColor.red == currentColor.red &&
		 targetColor.green == currentColor.green &&
		 targetColor.blue == currentColor.blue) { // target reached.
		return;
	}
	
	if(currentColor.red < targetColor.red) currentColor.red += 1;
	else if(currentColor.red > targetColor.red) currentColor.red -= 1;
	if(currentColor.green < targetColor.green) currentColor.green += 1;
	else if(currentColor.green > targetColor.green) currentColor.green -= 1;
	if(currentColor.blue < targetColor.blue) currentColor.blue += 1;
	else if(currentColor.blue > targetColor.blue) currentColor.blue -= 1;

	ledcWrite(redChannel, currentColor.red);
	ledcWrite(greenChannel, currentColor.green);
	ledcWrite(blueChannel, currentColor.blue);
}
