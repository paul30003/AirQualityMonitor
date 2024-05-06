#include <Arduino.h>
#include <WiFi.h>
#include "SdsDustSensor.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "settings.h"

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 4;
int sampleDelay = 900;
// int sampleDelay = 10;

float readingPm25 = 0;
float readingPm10 = 0;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

uint8_t mu[8] PROGMEM = {0x11, 0x11, 0x11, 0x11, 0x1F, 0x11, 0x10, 0x10};

SdsDustSensor sds(Serial2); // passing HardwareSerial& as parameter

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 27);
IPAddress gateway(192, 168, 0, 10);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

void clearValue(int column, int row, int num);

void setup()
{
  Serial.begin(115200);

  lcd.createChar(0, mu);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  delay(2000);

  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Firmware Version");
  lcd.setCursor(0, 1);
  lcd.print(version);
  lcd.setCursor(0, 2);
  lcd.print("Copyright 2024");
    lcd.setCursor(0, 3);
  lcd.print("P Fryer");

  delay(5000);

  sds.begin(); // this line will begin Serial1 with given baud rate (9600 by default)

  Serial.println();
  Serial.println(sds.queryFirmwareVersion().toString());  // prints firmware version
  Serial.println(sds.setQueryReportingMode().toString()); // ensures sensor is in 'query' reporting mode
  delay(5000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Air Quality Monitor");
  lcd.setCursor(0, 1);
  lcd.print("PM2.5: **.**");
  lcd.setCursor(15, 1);
  lcd.write(0b11100100);
  lcd.print("g/m3");
  lcd.setCursor(0, 2);
  lcd.print("PM10 : **.**");
  lcd.setCursor(15, 2);
  lcd.write(0b11100100);
  lcd.print("g/m3");
}

void loop()
{
  sds.wakeup();

  lcd.setCursor(0, 3);
  lcd.print("Sampling:");

  for (int i = 30; i > 0; i--)
  {
    digitalWrite(LED_BUILTIN, 1);
    delay(50);
    clearValue(10, 3, 3);
    lcd.setCursor(10, 3);
    lcd.print(i);
    digitalWrite(LED_BUILTIN, 0);
    delay(950);
  }

  PmResult pm = sds.queryPm();
  if (pm.isOk())
  {
    readingPm25 = pm.pm25;
    readingPm10 = pm.pm10;

    clearValue(7, 1, 8);
    lcd.setCursor(7, 1);
    lcd.print(String(readingPm25, 2));

    clearValue(7, 2, 8);
    lcd.setCursor(7, 2);
    lcd.print(String(readingPm10, 2));
  }
  else
  {
    // Serial.print("Could not read values from sensor, reason: ");
    // Serial.println(pm.statusToString());
  }

  digitalWrite(LED_BUILTIN, 0);
  WorkingStateResult state = sds.sleep();
  if (state.isWorking())
  {
    // Serial.println("Problem with sleeping the sensor.");
  }
  else
  {
    lcd.setCursor(0, 3);
    lcd.print("Sleeping:");
    for (int i = sampleDelay; i > 0; i--)
    {
      clearValue(10, 3, 3);
      lcd.setCursor(10, 3);
      lcd.print(i);
      delay(1000);
    }
  }
}

void clearValue(int column, int row, int num)
{
  lcd.setCursor(column, row);
  for (int i = 0; i < num; i++)
  {
    lcd.print(" ");
  }
}
