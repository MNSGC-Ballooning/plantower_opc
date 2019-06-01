//=====================================
// Version 2
//
// Arduino code for Plantower (PT) opc operation.
//
// Summary: 
//  -- Prepares data for logging  
//  -- Bin numbers correspond to:
//     - bin 1 --> Particles > 0.3um / 0.1L air
//     - bin 2 --> Particles > 0.5um / 0.1L air
//     - bin 3 --> Particles > 1.0um / 0.1L air
//     - bin 4 --> Particles > 2.5um / 0.1L air
//     - bin 5 --> Particles > 5.0um / 0.1L air
//     - bin 6 --> Particles > 10.0um / 0.1L air
//
// Wiring from PT to Arduino UNO (the following are the pins on the PT):
//  Vcc --> 5v
//  GND --> GND
//  TXD (3v3) --> logic level converter --> Pin 2 (5v)
//  RXD --> do not connect
//
// Written by: Joey Habeck
// Last Modified: 1/4/18
//
// Code adapted from Adafruit.
//
//=====================================


////////// BEGIN CODE //////////

#include <SD.h>

// define digital pins

#define pms2Serial Serial

int rx  = 2;          // Pin 2 on Arduino from TXD pin on Pt
int tx  = 3;          // Pin 3 on Arduino from nothing
//int rx2 = 5;
//int tx2 = 6;
unsigned long FT;     // flight-time

// more definitions
String dataLog;                   // used for data logging
String dataLog2;
int nhits=1;                      // used to count successful data transmissions    
int ntot=1;                       // used to count total attempted transmissions

String filename = "";  // file name that data wil be written to
String filename2 = "";

const int CS = 8;                 // CS pin on SD card
File ptLog;                     // files that data is written to 
File pt2Log;
// library for software serial comms
#include <SoftwareSerial.h>
SoftwareSerial pmsSerial(rx,tx);
//SoftwareSerial pms2Serial(rx2,tx2);

// define data structure for PT
typedef struct pms5003data {
    uint16_t framelen;
    uint16_t pm10_standard, pm25_standard, pm100_standard;
    uint16_t pm10_env, pm25_env, pm100_env;
    uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
    uint16_t unused;
    uint16_t checksum;
  }; 
  struct pms5003data data;
  struct pms5003data data2;

/// begin setup
void setup() {
  
  
  //Serial.println("Hello, there.");
  //Serial.println();
  //Serial.println("Setting up Plantower OPC...");
  //Serial.println();

  // set pinmodes
  pinMode(rx, INPUT);
  //pinMode(rx2, INPUT); 
  
  // sensor baud rate is 9600
  pmsSerial.begin(9600);
  pms2Serial.begin(9600);
  

  //Serial.print("Initializing SD card...");
  // Check if card is present/initalized: 
  if (!SD.begin(CS)){
  //Serial.println("card initialization FAILED - something is wrong..."); // card not present or initialization failed
  while (1); // dont do anything more
  }
  
  //Serial.println("card initialization PASSED... bon voyage!"); // initialization successful

  // Initialize file:
 for (int i = 0; i < 100; i++) {
    if (!SD.exists("ptLog" + String(i / 10) + String(i % 10) + ".csv")) {
      filename = "ptLog" + String(i / 10) + String(i % 10) + ".csv";
      openPTlog();
      break;
    }
  }

  for (int i = 0; i < 100; i++) {
    if (!SD.exists("pt2Log" + String(i / 10) + String(i % 10) + ".csv")) {
      filename2 = "pt2Log" + String(i / 10) + String(i % 10) + ".csv";
      openPT2log();
      break;
    }
  }
  
  //Serial.println("Plantower log created: " + filename);
  //Serial.println("Plantower log created: " + filename2);

  //Plantower
  String PHeader = "ID, Sample, Clock (h:m:s), Time (millisecs), Bin1, Bin2, Bin3, Bin4, Bin5, Bin6";
  ptLog.println(PHeader);//set up Flight log format
  pt2Log.println(PHeader);//set up Flight log format
  //Serial.println("PT logs header added");

  closePTlog();

  closePT2log();

}

/// begin loop
void loop() {
  FT = millis();

// log sample number, flight clock, time
    dataLog = "";
    dataLog2 = "";
    dataLog += "PT1";
    dataLog2 += "PT2";
    dataLog += ",";
    dataLog2 += ",";
    dataLog += ntot;
    dataLog2 += ntot;
    dataLog += ",";
    dataLog2 += ",";
    dataLog += flightTime(FT/1000);
    dataLog2 += flightTime(FT/1000);
    dataLog += ",";
    dataLog2 += ",";
    dataLog += FT;
    dataLog2 += FT;
    dataLog += ",";
    dataLog2 += ",";
    delay(50);

    updatePT1();
    updatePT2();
    
// samples of data collected
    nhits=nhits+1;
    
// total samples
    ntot = ntot+1;

}



// Reads in time from Arduino clock (seconds) and converts it to hr:min:sec; written by: Simon Peterson 
String flightTime(unsigned long t) {
  String fTime = "";
  fTime += (String(t / 3600) + ":");
  t %= 3600;
  fTime += String(t / 600);
  t %= 600;
  fTime += (String(t / 60) + ":");
  t %= 60;
  fTime += (String(t / 10) + String(t % 10));
  
  return fTime;
}
