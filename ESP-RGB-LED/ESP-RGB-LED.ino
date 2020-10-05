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

const int ldrThreshold = 500;
const int touchThreshold = 40;
bool overrideFlag = false;

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long touch0Millis = 0;
unsigned long touch3Millis = 0;

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
	currentMillis = millis();
	if(touchRead(T0) < touchThreshold) {
		if(touch0Millis == 0) {
			touch0Millis = currentMillis;
		}

		if((currentMillis-touch0Millis) >= 10) {
			overrideFlag = false;
		}
	}
	else {
		touch0Millis = 0;
	}

	if(touchRead(T3) < touchThreshold) {
		if(touch3Millis == 0) {
			touch3Millis = currentMillis;
		}

		if((currentMillis-touch3Millis) >= 10) {
			overrideFlag = true;
		}
	}
	else {
		touch3Millis = 0;
	}

	if(currentMillis - previousMillis >= 100) {
		struct tm timeinfo;
		if(!getLocalTime(&timeinfo)) {
			Serial.println("Failed to obtain time");
		}
//		Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
		setColorFromHour(timeinfo.tm_hour);
		fadeColor();
		
	  previousMillis = currentMillis;
	}
}

void setColorFromHour(int h) {
//	if (overrideFlag) { // buggy.. hard to switch off
//		targetColor = {255, 255, 255};
//		return;
//	}

	if(analogRead(36) > ldrThreshold) { // light is on
		targetColor = {0, 0, 0};
		return;
	}

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
		case 10:
			targetColor = {100, 50, 175};
			break;
		case 11:
			targetColor = {50, 50, 200};
			break;
		case 12:
			targetColor = {0, 0, 225};
			break;
		case 13:
			targetColor = {0, 100, 225};
			break;
		case 14:
			targetColor = {0, 170, 160};
			break;
		case 15:
			targetColor = {2, 175, 76};
			break;
		case 16:
			targetColor = {95, 200, 70};
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
