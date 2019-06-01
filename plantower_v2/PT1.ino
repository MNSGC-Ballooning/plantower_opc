void updatePT1(){ 
  if (readPMSdata(&pmsSerial)) {

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
    


// write data
   openPTlog();

   if (ptLog) {
      //Serial.println("tempLog.csv opened..."); // file open successfully 
      ptLog.println(dataLog);
      closePTlog();
   }
   else {
      //Serial.println("error opening file"); // file open failed
      return;
   }
  
// print data log to serial monitor
   //Serial.println(dataLog); 
  }

}

////////// USER DEFINED FUNCTIONS //////////

boolean readPMSdata(Stream *s) {
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
  memcpy((void *)&data, (void *)buffer_u16, 30);
 
  if (sum != data.checksum) {
    //Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
