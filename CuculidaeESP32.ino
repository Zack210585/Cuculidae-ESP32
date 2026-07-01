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
#include "BluetoothSerial.h"
RTC_DS3231 rtc;
Preferences preferences;
HardwareSerial NodeMCUSerial(2);
BluetoothSerial ESP32Bluetooth;
int LastCheckedMinute = -1, STSecond = 0, STMinute = 0, STHour = 0;
int STDay = 0, STMonth = 0, STYear = 0, STTimeZone = 0, SVTimeZone = 0;
int STDST = 0, SVDST = 0, ST1224 = 0, SV1224 = 0, STAMPM = 0, SVAMPM = 0;
void setup() 
{
	Serial.begin(115200);
	NodeMCUSerial.begin(115200, SERIAL_8N1, 16, 17); 
	ESP32Bluetooth.begin("Cuculidae");
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
		if (incomingData.startsWith("ST\x1F"))// Set time from Screen
		{
			int parsedFields = sscanf(incomingData.c_str() + 3, "%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d", 
			&STSecond, &STMinute, &STHour, &STDay, &STMonth, &STYear, &STDST, &STTimeZone, &ST1224, &STAMPM);
			if (parsedFields == 10) 
			{
				Serial.println("Time Set from Screen, Received!");
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
		else if(incomingData.startsWith("WT?"))// Get time to Screen
		{
			Serial.println("Time Inquiry from Screen, Received!");
			DateTime now = rtc.now();
			preferences.begin("clock_settings", false);
			SVDST = preferences.getInt("PDST", 0);
			SVTimeZone = preferences.getInt("PTZ", 0);
			SV1224 = preferences.getInt("P1224", 0);
	    SVAMPM = preferences.getInt("PAMPM", 0);
			preferences.end();
			NodeMCUSerial.println(String("ST") + "\x1F" + String(now.second()) + "\x1F" + now.minute() + "\x1F" + now.hour() + "\x1F" + now.day() + "\x1F" + now.month() + "\x1F" + now.year() + "\x1F" + SVDST + "\x1F" + SVTimeZone+ "\x1F" + SV1224 + "\x1F" + SVAMPM);
		}
		else if (incomingData.startsWith("SSD\x1F"))
		{
			Serial.println("Set Sound Day from Screen, Received!");
		}		
		else if (incomingData.startsWith("SSDN\x1F"))
		{
			Serial.println("Set Sound Day/ Night from Screen, Received!");
		}
		else if (incomingData.startsWith("SMD\x1F"))
		{
			Serial.println("Set Movement Day from Screen, Received!");	
		}
		else if (incomingData.startsWith("SMDN\x1F"))
		{
			Serial.println("Set Movement Day/ Night from Screen, Received!");	
		}
		else if (incomingData.startsWith("SL\x1F"))
		{
			Serial.println("Set Lights from Screen, Received!");	
		}
		else if (incomingData.startsWith("SS\x1F"))
		{
			Serial.println("Set Smoke from Screen, Received!");	
		}
		if (incomingData.startsWith("ST\x1F") || (incomingData.startsWith("SSD\x1F")) || (incomingData.startsWith("SSDN\x1F")) || (incomingData.startsWith("SMD\x1F")) || (incomingData.startsWith("SMDN\x1F")) || (incomingData.startsWith("SL\x1F")) || (incomingData.startsWith("SS\x1F")) ||  incomingData.startsWith("WE\x1F") || incomingData.startsWith("CA\x1F"))
		{
			ESP32Bluetooth.println(incomingData);
		}
	}
	if (ESP32Bluetooth.available() > 0) 
	{
    // WF = Wifi
		// LO = Location
		// ST = Set Time
		// WE = Weather
		// CA = Calendar
		String incomingData = ESP32Bluetooth.readStringUntil('\n');
		incomingData.trim();
		if (incomingData.startsWith("ST\x1F")) 
		{
			int parsedFields = sscanf(incomingData.c_str() + 3, "%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d\x1F%d", 
			&STSecond, &STMinute, &STHour, &STDay, &STMonth, &STYear, &STDST, &STTimeZone, &ST1224, &STAMPM);
			if (parsedFields == 10) 
			{
				Serial.println("Time Set from Phone, Received!");
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
		else if (incomingData.startsWith("WF\x1F"))
		{
			Serial.println("Set Wifi from Phone, Received!");
		}		
		else if (incomingData.startsWith("LO\x1F"))
		{
			Serial.println("Set Location from Phone, Received!");
		}
		else if (incomingData.startsWith("SSD\x1F"))
		{
			Serial.println("Set Sound Day from Phone, Received!");
		}		
		else if (incomingData.startsWith("SSDN\x1F"))
		{
			Serial.println("Set Sound Day/ Night from Phone, Received!");
		}
		else if (incomingData.startsWith("SMD\x1F"))
		{
			Serial.println("Set Movement Day from Phone, Received!");	
		}
		else if (incomingData.startsWith("SMDN\x1F"))
		{
			Serial.println("Set Movement Day/ Night from Phone, Received!");	
		}
		else if (incomingData.startsWith("SL\x1F"))
		{
			Serial.println("Set Lights from Phone, Received!");	
		}
		else if (incomingData.startsWith("SS\x1F"))
		{
			Serial.println("Set Smoke from Phone, Received!");	
		}
		if (incomingData.startsWith("ST\x1F") || (incomingData.startsWith("SSD\x1F")) || (incomingData.startsWith("SSDN\x1F")) || (incomingData.startsWith("SMD\x1F")) || (incomingData.startsWith("SMDN\x1F")) || (incomingData.startsWith("SL\x1F")) || (incomingData.startsWith("SS\x1F")) ||  incomingData.startsWith("WE\x1F") || incomingData.startsWith("CA\x1F"))
		{
			NodeMCUSerial.println(incomingData);
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
		NodeMCUSerial.println(String("ST") + "\x1F" + String(now.second()) + "\x1F" + now.minute() + "\x1F" + now.hour() + "\x1F" + now.day() + "\x1F" + now.month() + "\x1F" + now.year() + "\x1F" + SVDST + "\x1F" + SVTimeZone + "\x1F" + SV1224 + "\x1F" + SVAMPM);
		LastCheckedMinute = now.minute();
	}
}
