#include <Adafruit_NeoPixel.h>

#include "NTPClient.h"
#include "ESP8266WiFi.h"
#include "WiFiUdp.h"

const char *ssid = "horseman";
const char *password = "123321123";


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

#define LED_PIN 12
#define RAINBOW_SPEED 0.1

Adafruit_NeoPixel LED = Adafruit_NeoPixel(32, LED_PIN, NEO_GRB + NEO_KHZ800);
uint8_t r = 255, g = 0, b = 0;
uint16_t offset = 0;
uint32_t adiosAmigos = 3728435522;
uint32_t age = 0, startupTime = 0, timeStamp = 0, fadeTimeStamp = 0, brightTimeStamp = 0;
uint8_t mode = 0;
uint8_t mini = 20;
uint8_t maxi = 50;

void setup() {

	LED.begin();
	updateBrightness();
	LED.show();

	Serial.begin(115200);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	timeClient.begin();
	timeClient.update();
	delay(1000);
	age = timeClient.getEpochTime();
	Serial.println(age);
	timeClient.end();
	timeStamp = millis();
	updateBrightness();
}

void loop() {

	uint32_t seconds = adiosAmigos - age - startupTime;

	if (millis() - timeStamp >= 1000) {
		startupTime += (millis() - timeStamp)/1000;
		timeStamp = millis();
		Serial.println(age);
		Serial.println(startupTime);
		Serial.println(seconds);
	}

	if (millis() - brightTimeStamp >= 2000) {
		brightTimeStamp = millis();
		updateBrightness();
	}
	if(mode)
		rainbowCycle(seconds);
	else
		colorWipe(getFade(), seconds);
	offset += 1;

	if ((startupTime / 3600) % 2 == 0)
		mode = 0;
	else
		mode = 1;
	
}

void updateBrightness() {
	uint32_t avg = 0, x = 0, mx_local = 0, cnt = 1;
	for (int i = 0; i < 20; i++) {
		x = analogRead(A0);
		if (x > mx_local) {
			mx_local = x;
			avg += x;
			cnt++;
		}
		delay(20);
	}
	avg /= cnt;
	mini = (avg < mini ? avg : mini);
	maxi = (avg > maxi ? avg : maxi);
	
	Serial.println(avg);
	if (avg < 10) {
		LED.setBrightness(0);
		return;
	}
	if (avg < 30) {
		LED.setBrightness(100);
		return;
	}
	if (avg < 50) {
		LED.setBrightness(150);
		return;
	}
	if (avg < 100) {
		LED.setBrightness(255);
		return;
	}
}

uint32_t getFade() {
	if (millis() - fadeTimeStamp >= 1000) {
		fadeTimeStamp = millis();

		if (r > 0 && b == 0)
			r--, g++;

		if (g > 0 && r == 0)
			g--, b++;

		if (b > 0 && g == 0)
			r++, b--;
	}
	return LED.Color(r, g, b);

}

void colorWipe(uint32_t c, uint32_t seconds) {
	for (uint16_t i = 0; i < LED.numPixels(); i++) {
		if (seconds&(1LL << i))
			LED.setPixelColor(LED.numPixels() - i - 1, c);
		else
			LED.setPixelColor(LED.numPixels() - i - 1, LED.Color(0, 0, 0));
	}
	LED.show();
}

void rainbowCycle(uint32_t seconds) {
	uint16_t i, j;

	for (i = 0; i < LED.numPixels(); i++) {
		if(seconds&(1<<i))
			LED.setPixelColor(LED.numPixels() - i - 1, Wheel(((i * 256 / LED.numPixels()) + (int)(offset*RAINBOW_SPEED)) & 255));
		else
			LED.setPixelColor(LED.numPixels() - i - 1, LED.Color(0,0,0));
	}
	LED.show();
}

uint32_t Wheel(byte WheelPos) {
	if (WheelPos < 85) {
		return LED.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
	else if (WheelPos < 170) {
		WheelPos -= 85;
		return LED.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	else {
		WheelPos -= 170;
		return LED.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
}