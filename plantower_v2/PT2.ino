void updatePT2(){  
  
  if (readPMSdata2(&pms2Serial)) {

// if data is receieved, log it
    dataLog2 += data2.particles_03um;
    dataLog2 += ",";
    dataLog2 += data2.particles_05um;
    dataLog2 += ",";
    dataLog2 += data2.particles_10um;
    dataLog2 += ",";
    dataLog2 += data2.particles_25um;
    dataLog2 += ",";
    dataLog2 += data2.particles_50um;
    dataLog2 += ",";
    dataLog2 += data2.particles_100um;
    dataLog2 += ",";
    

// write data
   openPT2log();

   if (pt2Log) {
      //Serial.println("tempLog.csv opened..."); // file open successfully 
      pt2Log.println(dataLog2);
      closePT2log();
   }
   else {
      //Serial.println("error opening file"); // file open failed
      return;
   }
  
// print data log to serial monitor
   //Serial.println(dataLog2); 
  }

}

////////// USER DEFINED FUNCTIONS //////////

boolean readPMSdata2(Stream *s) {
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
  memcpy((void *)&data2, (void *)buffer_u16, 30);
 
  if (sum != data2.checksum) {
    //Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
