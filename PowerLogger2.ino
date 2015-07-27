/*
  Running process using Process class.

 This sketch demonstrate how to run linux processes
 using an Arduino Yún.

 created 5 Jun 2013
 by Cristian Maglie

 This example code is in the public domain.

 http://arduino.cc/en/Tutorial/Process

 */

#include <Process.h>
#include <Console.h>

const int samplingIntervalMs = 10;
const int samplingCount = 32767;

void setup() {
  // Initialize Bridge
  Bridge.begin();

  // Initialize Console
  Console.begin();
  
  randomSeed(analogRead(0));

  // Wait until a Console Monitor is connected.
  //while (!Console);

  // run various example processes
  //runCurl();
  
  //postToThingSpeak("O56PGYKAKMGKVE7Y");
  
  runCpuInfo();
}

void loop() {
  //postToThingSpeak("O56PGYKAKMGKVE7Y");
  postToThingSpeak("MQU58OZHMEIEY3O5");
}

void runCurl() {
  // Launch "curl" command and get Arduino ascii art logo from the network
  // curl is command line program for transferring data using different internet protocols
  Process p;		// Create a process and call it "p"
  //p.begin("curl");	// Process that launch the "curl" command
  //p.addParameter("http://arduino.cc/asciilogo.txt"); // Add the URL parameter to "curl"
  //p.run();		// Run the process and wait for its termination
  //p.runShellCommand("curl http://arduino.cc/asciilogo.txt");
  
  p.runShellCommand("/usr/bin/pretty-wifi-info.lua");

  // Print arduino logo over the Console
  // A process output can be read with the stream methods
  while (p.available() > 0) {
    char c = p.read();
    Console.print(c);
  }
  // Ensure the last bit of data is sent.
  Console.flush();
}

void runCpuInfo() {
  // Launch "cat /proc/cpuinfo" command (shows info on Atheros CPU)
  // cat is a command line utility that shows the content of a file
  Process p;		// Create a process and call it "p"
  //p.begin("cat");	// Process that launch the "cat" command
  //p.addParameter("/proc/cpuinfo"); // Add the cpuifo file path as parameter to cut
  //p.run();		// Run the process and wait for its termination
  p.runShellCommand("cat /proc/cpuinfo");

  // Print command output on the Console.
  // A process output can be read with the stream methods
  while (p.available() > 0) {
    char c = p.read();
    Console.print(c);
  }
  // Ensure the last bit of data is sent.
  Console.flush();
}

void postToThingSpeak(String key) {
  
  long avgA0 = 0L;
  int minA0 = 9999;
  int maxA0 = 0;
  
  long avgA1 = 0L;
  int minA1 = 9999;
  int maxA1 = 0;
  
  long avgA2 = 0L;
  int minA2 = 9999;
  int maxA2 = 0;
  
  long avgA3 = 0L;
  int minA3 = 9999;
  int maxA3 = 0;
  
  int a0, a1, a2, a3;
  
  digitalWrite(13, LOW);
  for (int i=0; i<samplingCount; i++) {
    a0 = analogRead(0);
    avgA0 += a0;
    if (minA0>a0) {
      minA0 = a0;
    }
    if (maxA0<a0) {
      maxA0 = a0;
    }
    
    a1 = analogRead(1);
    avgA1 += a1;
    if (minA1>a1) {
      minA1 = a1;
    }
    if (maxA1<a1) {
      maxA1 = a1;
    }
    
    a2 = analogRead(2);
    avgA2 += a2;
    if (minA2>a2) {
      minA2 = a2;
    }
    if (maxA2<a2) {
      maxA2 = a2;
    }
    
    a3 = analogRead(3);
    avgA3 += a3;
    if (minA3>a3) {
      minA3 = a3;
    }
    if (maxA3<a3) {
      maxA3 = a3;
    }
    
  }
  
  digitalWrite(13, HIGH);
  
  avgA0 = avgA0 / samplingCount;
  avgA1 = avgA1 / samplingCount;
  avgA2 = avgA2 / samplingCount;
  avgA3 = avgA3 / samplingCount;
  
  int    zeroCurrentValue = 512;
  float currentSlope = 13.52;

  float voltageSlope = 205.71;
  float voltageDividerRatio = 7.832; //56K+8.2K 
  
  float voltage,currentSolar, currentBattery1, currentBattery2;
  
  voltage = avgA0 * voltageDividerRatio / voltageSlope;
  currentSolar = (avgA1-zeroCurrentValue)/currentSlope;
  currentBattery1 = (avgA2-zeroCurrentValue)/currentSlope;
  currentBattery2 = (avgA3-zeroCurrentValue)/currentSlope;

  Process p;
  String cmd = "curl --data \"key="+key;
  cmd = cmd + "&field1=" + voltage;
  cmd = cmd + "&field2=" + currentSolar;
  cmd = cmd + "&field3=" + currentBattery1;
  cmd = cmd + "&field4=" + currentBattery2;
  cmd = cmd + "\" http://api.thingspeak.com/update";
  
  Console.println(cmd);
  
  p.runShellCommand(cmd);
  
  while (p.available() > 0) {
    char c = p.read();
    Console.print(c);
  }
  
  // Ensure the last bit of data is sent.
  Console.flush();
  p.close();
}
