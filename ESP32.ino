//ESP32-S3 Blue Pin 17 RX to ESP32 WROOM Pin 17 TX2
//ESP32-S3 Black Pin 18 TX to ESP32 WROOM Pin 16 RX2
//VCC: Connect to 3.3V on the ESP32.
//GND: Connect to GND on the ESP32.
//SDA: Connect to GPIO 21 SDA
//SCL: Connect to GPIO 22 SCL
#include <Wire.h>
#include "RTClib.h"
#include <Arduino.h>
#include <Preferences.h>
#include "BluetoothSerial.h" // Added per request
RTC_DS3231 rtc;
Preferences preferences;
HardwareSerial NodeMCUSerial(2);
BluetoothSerial ESP32Bluetooth; // Added per request
int LastCheckedMinute = -1, STSecond = 0, STMinute  = 0, STHour = 0, STDay = 0, STMonth = 0, STYear = 0, STTimeZone = 0, SVTimeZone = 0, STDST = 0, SVDST = 0, ST1224 = 0, SV1224 = 0, STAMPM = 0, SVAMPM = 0;
void setup() 
{
	Serial.begin(115200);
	NodeMCUSerial.begin(115200, SERIAL_8N1, 16, 17); 
	ESP32Bluetooth.begin("Cuculidae"); // Added per request
	if (!rtc.begin()) 
	{
		Serial.println("Couldn't find RTC module!");
		delay(100);
		if (!rtc.begin()) 
		{
			Serial.println("Couldn't find RTC module!");
			while (1);
		}
		else Serial.println("RTC init!");
	}
	else Serial.println("RTC init!");
	if (rtc.lostPower()) 
	{
		Serial.println("RTC lost power, setting the time...");
	}
	preferences.begin("clock_settings", false);
	SVDST = preferences.getInt("PDST", 0);
	SVTimeZone = preferences.getInt("PTZ", 0);
	SV1224 = preferences.getInt("P1224", 0);
	SVAMPM = preferences.getInt("PAMPM", 0);
	preferences.end();
	DateTime now = rtc.now();
	delay(500);
}
void SerialReceive() 
{
	if (NodeMCUSerial.available() > 0) 
	{
		String incomingData = NodeMCUSerial.readStringUntil('\n');
		incomingData.trim(); 
		if (incomingData.startsWith("ST@")) 
		{
			int parsedFields = sscanf(incomingData.c_str() + 3, "%d@%d@%d@%d@%d@%d@%d@%d@%d@%d", 
			&STSecond, &STMinute, &STHour, &STDay, &STMonth, &STYear, &STDST, &STTimeZone, &ST1224, &STAMPM);
			if (parsedFields == 10) 
			{
				Serial.println("Time Set Received!");
				preferences.begin("clock_settings", false);
				if (STDST != SVDST)
				{
					preferences.putInt("PDST", STDST);
					SVDST = STDST;
				}
				if (STTimeZone != SVTimeZone)
				{
					preferences.putInt("PTZ", STTimeZone);
					SVTimeZone = STTimeZone;
				}
				if (ST1224 != SV1224)
				{
					preferences.putInt("P1224", ST1224);
					SV1224 = ST1224;
				}
				if (STAMPM != SVAMPM)
				{
					preferences.putInt("PAMPM", STAMPM);
					SVAMPM = STAMPM;
				}
				preferences.end(); 
			}
			rtc.adjust(DateTime(STYear, STMonth, STDay, STHour, STMinute, STSecond));
		}
		else if(incomingData.startsWith("WT?"))
		{
			Serial.println("Time Inquiry Received!");
			DateTime now = rtc.now();
			preferences.begin("clock_settings", false);
			SVDST = preferences.getInt("PDST", 0);
			SVTimeZone = preferences.getInt("PTZ", 0);
			SV1224 = preferences.getInt("P1224", 0);
	    SVAMPM = preferences.getInt("PAMPM", 0);
			preferences.end();
			NodeMCUSerial.println(String("ST") + "@" + String(now.second()) + "@" + now.minute() + "@" + now.hour() + "@" + now.day() + "@" + now.month() + "@" + now.year() + "@" + SVDST + "@" + SVTimeZone+ "@" + SV1224 + "@" + SVAMPM);
			ESP32Bluetooth.println(String("ST") + "@" + String(now.second()) + "@" + now.minute() + "@" + now.hour() + "@" + now.day() + "@" + now.month() + "@" + now.year() + "@" + SVDST + "@" + SVTimeZone+ "@" + SV1224 + "@" + SVAMPM); // Added per request
			LastCheckedMinute = now.minute();
		}
		else if (incomingData.length() > 0)
		{
			Serial.println(incomingData);
		}
	}

	// Bluetooth checking implementation block added directly below without altering the block above
	if (ESP32Bluetooth.available() > 0) 
	{
		String incomingData = ESP32Bluetooth.readStringUntil('\n');
		incomingData.trim(); 
		if (incomingData.length() > 0) 
		{
			Serial.println(incomingData); // Prints the Bluetooth serial data to terminal per request
		}
		if (incomingData.startsWith("ST@")) 
		{
			int parsedFields = sscanf(incomingData.c_str() + 3, "%d@%d@%d@%d@%d@%d@%d@%d@%d@%d", 
			&STSecond, &STMinute, &STHour, &STDay, &STMonth, &STYear, &STDST, &STTimeZone, &ST1224, &STAMPM);
			if (parsedFields == 10) 
			{
				Serial.println("Time Set Received!");
				preferences.begin("clock_settings", false);
				if (STDST != SVDST)
				{
					preferences.putInt("PDST", STDST);
					SVDST = STDST;
				}
				if (STTimeZone != SVTimeZone)
				{
					preferences.putInt("PTZ", STTimeZone);
					SVTimeZone = STTimeZone;
				}
				if (ST1224 != SV1224)
				{
					preferences.putInt("P1224", ST1224);
					SV1224 = ST1224;
				}
				if (STAMPM != SVAMPM)
				{
					preferences.putInt("PAMPM", STAMPM);
					SVAMPM = STAMPM;
				}
				preferences.end(); 
			}
			rtc.adjust(DateTime(STYear, STMonth, STDay, STHour, STMinute, STSecond));
		}
		else if(incomingData.startsWith("WT?"))
		{
			Serial.println("Time Inquiry Received!");
			DateTime now = rtc.now();
			preferences.begin("clock_settings", false);
			SVDST = preferences.getInt("PDST", 0);
			SVTimeZone = preferences.getInt("PTZ", 0);
			SV1224 = preferences.getInt("P1224", 0);
	    SVAMPM = preferences.getInt("PAMPM", 0);
			preferences.end();
			NodeMCUSerial.println(String("ST") + "@" + String(now.second()) + "@" + now.minute() + "@" + now.hour() + "@" + now.day() + "@" + now.month() + "@" + now.year() + "@" + SVDST + "@" + SVTimeZone+ "@" + SV1224 + "@" + SVAMPM);
			ESP32Bluetooth.println(String("ST") + "@" + String(now.second()) + "@" + now.minute() + "@" + now.hour() + "@" + now.day() + "@" + now.month() + "@" + now.year() + "@" + SVDST + "@" + SVTimeZone+ "@" + SV1224 + "@" + SVAMPM); // Added per request
			LastCheckedMinute = now.minute();
		}
	}
}
void loop() 
{
	SerialReceive();
	DateTime now = rtc.now();
	if (now.minute() != LastCheckedMinute) 
	{
		Serial.println("Auto Time Update Sent!");
		preferences.begin("clock_settings", false);
		SVDST = preferences.getInt("PDST", 0);
		SVTimeZone = preferences.getInt("PTZ", 0);
		SV1224 = preferences.getInt("P1224", 0);
	  SVAMPM = preferences.getInt("PAMPM", 0);
		preferences.end();
		NodeMCUSerial.println(String("ST") + "@" + String(now.second()) + "@" + now.minute() + "@" + now.hour() + "@" + now.day() + "@" + now.month() + "@" + now.year() + "@" + SVDST + "@" + SVTimeZone+ "@" + SV1224 + "@" + SVAMPM);
		ESP32Bluetooth.println(String("ST") + "@" + String(now.second()) + "@" + now.minute() + "@" + now.hour() + "@" + now.day() + "@" + now.month() + "@" + now.year() + "@" + SVDST + "@" + SVTimeZone+ "@" + SV1224 + "@" + SVAMPM); // Added per request
		LastCheckedMinute = now.minute();
	}
}
