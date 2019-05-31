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
// PT1:
//    Vcc --> 5v
//    GND --> GND
//    TXD (3v3) --> logic level converter --> Pin 2 (5v)
//    RXD --> NC
// PT2:
//    Vcc --> 5v
//    GND --> GND
//    TXD (3v3) --> logic level converter --> Pin 5 (5v)
//    RXD --> NC
//
// Written by: Joey Habeck
// Modified by Garrett Ailts to allow the use of two plantower devices
// Last Modified: 5/31/19
//
// Code adapted from Adafruit.
//
//=====================================


////////// BEGIN CODE //////////
#include <SoftwareSerial.h>
#include <SD.h>

////////////Pin Definitions////////////////
//int led = 4;          // LED light (not required)
#define rx 2           // Pin 2 on Arduino from TXD pin on PT
#define tx 3           // Pin 3 on Arduino from nothing
#define rx2 5          // Pin 5 on Arduino from TXD pin on PT2
#define tx2 6          // Pin 6 on Arduino from nothing

////////////Define Variables///////////////
unsigned long FT;     // flight-time
String dataLogHeader = "Sample, Clock (h:m:s), Time (millisecs), PT1Bin1, PT1Bin2, PT1Bin3, PT1Bin4, PT1Bin5, PT1Bin6, PT2Bin1, PT2Bin2, PT2Bin3, PT2Bin4, PT2Bin5, PT2Bin6";
String dataLog;                   // used for data logging
int nhits=1;                      // used to count successful data transmissions    
int ntot=1;                       // used to count total attempted transmissions
String filename = "ptLog.csv";  // file name that data wil be written to
const int CS = 8;                 // CS pin on SD card
File ptLog;                     // file that data is written to 

//Create Serial Objects for both Plantowers
SoftwareSerial pmsSerial(rx,tx);
SoftwareSerial pmsSerial2(rx2,tx2);

// define data structure for PT
struct pms5003data {
    uint16_t framelen;
    uint16_t pm10_standard, pm25_standard, pm100_standard;
    uint16_t pm10_env, pm25_env, pm100_env;
    uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
    uint16_t unused;
    uint16_t checksum;
  };
pms5003data data;
pms5003data data2;
pms5003data *pt1 = &data;
pms5003data *pt2 = &data2;

/////////////Function Prototypes////////////
boolean readPMSdata(Stream *s, pms5003data *d);

/// begin setup
void setup() {
  
  Serial.begin(115200);
  Serial.println("Hello, there.");
  Serial.println();
  Serial.println("Setting up Plantower OPC...");
  Serial.println();

  // set pinmodes
  //pinMode(led, OUTPUT);
  pinMode(rx, INPUT); 
  
  // sensor baud rate is 9600
  pmsSerial.begin(9600);
  pmsSerial2.begin(9600);
  

  Serial.print("Initializing SD card...");
  // Check if card is present/initalized: 
  if (!SD.begin(CS)){
  Serial.println("card initialization FAILED - something is wrong..."); // card not present or initialization failed
  while (1); // dont do anything more
  }
  
  Serial.println("card initialization PASSED... bon voyage!"); // initialization successful

  // Initialize file:
  ptLog = SD.open(filename, FILE_WRITE); // open file
  
  if (ptLog) {
    Serial.println( filename + " opened...");
    ptLog.println(dataLogHeader); // file heading
    ptLog.close();
    Serial.println("File initialized... begin data logging!");
  }
  else {
    Serial.println("error opening file");
    return;
  }

}

/// begin loop
void loop() {
  //digitalWrite(led,HIGH);
  FT = millis();

// log sample number, flight clock, time
    dataLog = "";
    dataLog += ntot;
    dataLog += ",";
    dataLog += flightTime(FT/1000);
    dataLog += ",";
    dataLog += FT;
    dataLog += ",";
    
  if (readPMSdata(&pmsSerial,pt1)) {

// if data is receieved, log it
      dataLog += data.particles_03um;
      dataLog += ",";
      dataLog += data.particles_05um;
      dataLog += ",";
      dataLog += data.particles_10um;
      dataLog += ",";
      dataLog += data.particles_25um;
      dataLog += ",";
      dataLog += data.particles_50um;
      dataLog += ",";
      dataLog += data.particles_100um;
      dataLog += ",";
  }
  if(readPMSdata(&pmsSerial2,pt2)){
      dataLog += data2.particles_03um;
      dataLog += ",";
      dataLog += data2.particles_05um;
      dataLog += ",";
      dataLog += data2.particles_10um;
      dataLog += ",";
      dataLog += data2.particles_25um;
      dataLog += ",";
      dataLog += data2.particles_50um;
      dataLog += ",";
      dataLog += data2.particles_100um;
      dataLog += ",";
  }
  
    
// samples of data collected
    nhits=nhits+1;
    
// total samples
    ntot = ntot+1;

// write data
    ptLog = SD.open(filename, FILE_WRITE); // open file

 if (ptLog) {
    //Serial.println("tempLog.csv opened..."); // file open successfully 
    ptLog.println(dataLog);
    ptLog.close();
  }
  else {
    Serial.println("error opening file"); // file open failed
    return;
  }
  
// print data log to serial monitor
  Serial.println(dataLog); 


}

////////// USER DEFINED FUNCTIONS //////////

boolean readPMSdata(Stream *s, pms5003data *d) {
  if (! s->available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
 
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
 
 /*//debugging
  for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
  }
  Serial.println();
*/
  
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
  // put it into a nice struct :)
  memcpy((void *)d, (void *)buffer_u16, 30);
 
  if (sum != d->checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
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
