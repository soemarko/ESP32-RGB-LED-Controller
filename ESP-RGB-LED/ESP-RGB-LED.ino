#include <WiFi.h>
#include "time.h"
#include "RGBConverter.h"

struct Color {
	int red;
	int green;
	int blue;
};

Color currentColor = {0, 0, 0};
Color targetColor = {0, 0, 0};

RGBConverter colorConverter;
int lastChangedHour = -1;

//const char* ssid = "ssid";
//const char* password = "password";
#include "wificreds.h" // create wificreds.h and use above as the content with your ssid and password

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

void randomizedColor(double lit = 0.75) {
	uint8_t rgb[2];
	double hue = random(10001) / 10000.0;
	double sat = random(750, 1000) / 1000.0;

	colorConverter.hslToRgb(hue, sat, lit, rgb);

	targetColor = {rgb[0], rgb[1], rgb[2]};
}

void setup() {
	Serial.begin(115200);
	randomSeed(analogRead(0));

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
		lastChangedHour = -1;
		return;
	}

	if(lastChangedHour == h) {
		return;
	}

	switch(h) {
		case 0 ... 6: // midnight to 6am
			targetColor = {180, 0, 0};
			break;
		case 7 ... 14:
			randomizedColor(1.0); // full brightness
			break;
		case 15 ... 22:
			randomizedColor(0.9);
			break;
		case 23:
			randomizedColor(0.75);
			break;
		default:
			Serial.println("off"); // 0, 0 , 0
			targetColor = {0, 0, 0};
			break;
	}
	
	lastChangedHour = h;
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
