/*
  SD card datalogger
 
 This example shows how to log data from three analog sensors 
 to an SD card using the SD library.
 	
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 
 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe
 
 This example code is in the public domain.
 	 
 */

#include <Wire.h>
#include "RTClib.h"
#include <SD.h>
//#include <LiquidCrystal.h>
#include <Time.h>

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 10;

const int relayControlPin =  2;      // the number of the relay control pin
const int fanControlPin =  3;        // the number of the cooling fan control pin

const int currentFanOnThreshold =  10;        // Once current is greater, turn on the cooling fan
const int currentRelayBypassThreshold =  25;        // once the current is greater, put relay in bypass

RTC_DS1307 rtc;

// initialize the library with the numbers of the interface pins
// 9 -> RS (pin4)
// 8 -> E  (pin6)
//LiquidCrystal lcd(9, 8, 5, 4, 3, 2);

char   dataLogFile[18];

int    sdCardPresent = 0;
int    fanState      = 0;
int    relayState    = 0;

void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // set up the LCD's number of columns and rows: 
  //lcd.begin(16, 2);
  
  //lcd.print("Init SD card...");
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  pinMode(relayControlPin, OUTPUT);
  pinMode(fanControlPin, OUTPUT);
  
  // put the relay in the bypass mode first
  digitalWrite(relayControlPin, LOW);
  relayState    = 0;
  
  // put the cooling fan in the ON mode
  digitalWrite(fanControlPin, HIGH);
  fanState      = 1;
  

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // we'll continue anyay, since we'll re-init in the loop:
    //return;
    sdCardPresent = 0;
    Serial.println("card not detected.");
    //lcd.setCursor(0, 1);  
    //lcd.print("No SD card");
  }
  else
  {
    sdCardPresent = 1;
    Serial.println("card initialized.");
    //lcd.setCursor(0, 1);  
    //lcd.print("SD card OK");
  }
  
#ifdef AVR
  Wire.begin();
#else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
#endif
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  
  // Generate the data log file based on the current time
  String temp="";
  DateTime tempTime = rtc.now();
  
  if (tempTime.day()<10)
  {
    temp += "0";
  }
  temp += tempTime.day();
  if (tempTime.hour()<10)
  {
    temp += "0";
  }
  temp += tempTime.hour();
  if (tempTime.minute()<10)
  {
    temp += "0";
  }
  temp += tempTime.minute();
  if (tempTime.second()<10)
  {
    temp += "0";
  }
  temp += tempTime.second();
  temp += ".txt";
  
  temp.toCharArray(dataLogFile, 18);
  
  digitalClockDisplay();
  
  delay(2000);
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.print(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void loop()
{
  String timeStamp = "";
  DateTime now = rtc.now();
  //timeStamp += now.year();
  //timeStamp += "-";
  timeStamp += now.month();
  timeStamp += "-";
  timeStamp += now.day();
  timeStamp += ",";
  timeStamp += now.hour();
  timeStamp += ":";
  timeStamp += now.minute();
  timeStamp += ":";
  timeStamp += now.second();
  timeStamp += "    ";
  
  //Keep checking SD card until it's loaded
  //  if (!sdCardPresent)
  //  {
  //    lcd.setCursor(0, 0);
  //    lcd.print(timeStamp);
  //    lcd.setCursor(0, 1);  
  //    lcd.print("Init SD card... ");
  //    if (!SD.begin(chipSelect)) {
  //      Serial.println("Card failed, or not present");
  //      // we'll continue anyay, since we'll re-init in the loop:
  //      //return;
  //      sdCardPresent = 0;
  //      Serial.println("card not detected.");
  //      lcd.setCursor(0, 1);  
  //      lcd.print("No SD card      ");
  //    }
  //    else
  //    {
  //      sdCardPresent = 1;
  //      Serial.println("card initialized.");
  //      lcd.setCursor(0, 1);  
  //      lcd.print("SD card OK      ");
  //    }
  //  }
    
  // make a string for assembling the data to log:
  String dataString = "", displayString="";

  // read three sensors and append to the string:
  int A0[20], A1[20], A2[20], A3[20];
  int avgA0, minA0, maxA0, avgA1, minA1, maxA1,avgA2, minA2, maxA2,avgA3, minA3, maxA3;
  avgA0 = 0;
  minA0 = 9999;
  maxA0 = 0;
  avgA1 = 0;
  minA1 = 9999;
  maxA1 = 0;
  avgA2 = 0;
  minA2 = 9999;
  maxA2 = 0;
  avgA3 = 0;
  minA3 = 9999;
  maxA3 = 0;
  for (int c = 0; c < 20; c++) {
    A0[c] = analogRead(0);
    if (A0[c]<minA0) {
      minA0 = A0[c];
    }
    if (A0[c]>maxA0) {
      maxA0 = A0[c];
    }
    avgA0 += A0[c];
    
    A1[c] = analogRead(1);
    if (A1[c]<minA1) {
      minA1 = A1[c];
    }
    if (A1[c]>maxA1) {
      maxA1 = A1[c];
    }
    avgA1 += A1[c];
    
    A2[c] = analogRead(2);
    if (A2[c]<minA2) {
      minA2 = A2[c];
    }
    if (A2[c]>maxA2) {
      maxA2 = A2[c];
    }
    avgA2 += A2[c];
    
    A3[c] = analogRead(3);
    if (A3[c]<minA3) {
      minA3 = A3[c];
    }
    if (A3[c]>maxA3) {
      maxA3 = A3[c];
    }
    avgA3 += A3[c];
  }
  avgA0 = avgA0 / 20;
  avgA1 = avgA1 / 20;
  avgA2 = avgA2 / 20;
  avgA3 = avgA3 / 20;
  
  // Convert the ADC values to current and voltage
  int    zeroCurrentValue = 510;
  float currentSlope = 13.6;
  float voltageSlope = 204.309;
  float voltageDividerRatio = 5.6731; //47K+10K   //2.9965;
  
  float current, voltage;
  char buffer[10];
  current = (avgA0-zeroCurrentValue)/currentSlope;
  voltage = avgA1 * voltageDividerRatio / voltageSlope;
  
  if (0==fanState &&abs(current)>currentFanOnThreshold) {
    // turn on the fan
    digitalWrite(fanControlPin, HIGH);
    fanState = 1;
  }
  
  if (1==fanState && abs(current)<currentFanOnThreshold-2 ) {
    // turn off the fan
    digitalWrite(fanControlPin, LOW);
    fanState = 0;
  }
  
  if (0==relayState && abs(current)<currentRelayBypassThreshold-5) {
    // put the relay in the loop
    digitalWrite(relayControlPin, HIGH);
    relayState = 1;
  }
  
  if (1==relayState && abs(current)>currentRelayBypassThreshold ) {
    // put the relay in bypass
    digitalWrite(relayControlPin, LOW);
    relayState = 0;
  }

  dtostrf(current, 10, 5, buffer);
  dataString += String(buffer);
  dataString += ",";
  
  displayString += String(buffer);
  displayString += ",";
  
  dtostrf(voltage, 10, 5, buffer);
  dataString += String(buffer);
  dataString += ",";
  
  displayString += String(buffer);
  
  dtostrf(current*voltage, 10, 5, buffer);
  dataString += String(buffer);
  dataString += ",";
  
  // convert A3 readings to current for solar panel
  current = (avgA3-zeroCurrentValue)/currentSlope;
  dtostrf(current, 10, 5, buffer);
  dataString += String(buffer);
  dataString += ",";

  dataString += String(avgA0);
  dataString += ",";
  dataString += String(minA0);
  dataString += ",";
  dataString += String(maxA0);
  dataString += ",";
  dataString += String(avgA1);
  dataString += ",";
  dataString += String(minA1);
  dataString += ",";
  dataString += String(maxA1);
  dataString += ",";
  dataString += String(avgA2);
  dataString += ",";
  dataString += String(minA2);
  dataString += ",";
  dataString += String(maxA2);
  dataString += ",";
  dataString += String(avgA3);
  dataString += ",";
  dataString += String(minA3);
  dataString += ",";
  dataString += String(maxA3);
  
  delay(10000);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(dataLogFile, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    
    dataFile.print(now.year());
    dataFile.print("-");
    dataFile.print(now.month());
    dataFile.print("-");
    dataFile.print(now.day());
    dataFile.print(",");
    dataFile.print(now.hour());
    dataFile.print(":");
    dataFile.print(now.minute());
    dataFile.print(":");
    dataFile.print(now.second());
    dataFile.print(",");
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
    //lcd.setCursor(0, 0);
    //lcd.print(timeStamp);
    //lcd.setCursor(0, 1);
    //lcd.print(dataString);
    //lcd.print("          ");
  }  
  // if the file isn't open, pop up an error:
  else {
    sdCardPresent = 0;
    Serial.println("error opening datalog.txt");
    String temp="F: ";
    temp+= dataLogFile;
    //lcd.setCursor(0, 0);
    //lcd.print(timeStamp);
    //lcd.setCursor(0, 1);
    //lcd.print(temp);
    return;
  } 
}









