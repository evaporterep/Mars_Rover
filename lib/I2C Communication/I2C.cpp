/*#include <Arduino.h>
#include <Wire.h>

uint8_t crc8(const uint8_t *data, size_t len) {
  uint8_t crc = 0xFF;
  //crc = 1111 1111
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) {
        //& 1000 0000
        crc = (crc << 1) ^ 0x31;
        //^ 00110001
      } else {
        crc = crc << 1;
      }
    }
  }
  return crc;
}

bool I2CSendCommand(uint8_t adress, uint8_t cmd) {
  Wire.beginTransmission(adress);
  //Send start bit and SHT41 adress
  Wire.write(cmd);
  //Send write bit and command
  return Wire.endTransmission() == 0;  
  // 0 means ACK bit received
}

bool I2CReadBytes(uint8_t adress, uint8_t expected, uint8_t buf[], int crc[] = {0}) {
  uint8_t got = Wire.requestFrom(adress, (uint8_t)expected);
  //The number of bytes received
  if(got != expected) return false;
  //Make sure 6 bytes were received from the SHT41

  for(uint8_t i=0; i<expected; i++){
    buf[i] = Wire.read();
  }
  //Read from the wire and assign to buffer array

  if (crc8(&buf[0], 2) != buf[2]) return false;
  if (crc8(&buf[3], 2) != buf[5]) return false;
  return true;
  //Ensure the crc received matches the calculated crc
}

bool sht41Restart(){
  bool restarted = sht41SendCommand(CMD_SOFT_RESET);
  delay(2);
  return restarted;
  //Restart the SHT41
}

bool sht41ReadSerial(uint32_t &serial){
  if (!sht41SendCommand(CMD_READ_SERIAL)) return false;
  delay(2);
  //Send command to read serial number of SHT41 and wait for operation to be performed

  uint8_t buf[6];
  if(!sht41ReadSixBytes(buf)) return false;
  //Assign data to buffer and validate it through crc

  serial = (((uint32_t) buf[0] << 24) | ((uint32_t) buf[1] << 16) | ((uint32_t) buf[3] << 8) | ((uint32_t) buf[4]));
  //Convert the 4 bytes to one variable
  return true;
}

bool sht41Read(float &temp, float &humidity){
  if(!sht41SendCommand(CMD_MEASURE_HIGH)) return false;
  delay(10);
  //Send the command to perform a high precision reading on the SHT41 and wait for it to be performed

  uint8_t buf[6];
  if(!sht41ReadSixBytes(buf)) return false;
  //Assign data to buffer and validate it through crc

  
  uint16_t tempRaw = ((uint16_t) buf[0] << 8 | buf[1]);
  uint16_t humidityRaw = ((uint16_t) buf[3] << 8 | buf[4]);
  //Assign bytes from buffer to raw variables

  temp = -45.0f + 175.0f * (float)tempRaw  / 65535.0f;
  humidity =  -6.0f + 125.0f * (float)humidityRaw / 65535.0f;
  //Convert the raw readings to readable values using fomula from datasheet

  if (humidity > 100) humidity = 100.0f;
  if (humidity < 0) humidity = 0.0f;
  //The conversion can go past min and max values so bind them to reality

  return true;
}
*/