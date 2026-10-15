/*#include <Arduino.h>
#include <Wire.h>
#include <ESP32-TWAI-CAN.hpp>

constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;

constexpr uint8_t CAN_TX = 4;
constexpr uint8_t CAN_RX = 3;

//constexpr uint8_t CAN_TX = 21;
//constexpr uint8_t CAN_RX = 20;

constexpr uint8_t SHT41_ADDR        = 0x44;  // I2C address (SHT41-AD1B)
constexpr uint8_t CMD_MEASURE_HIGH  = 0xFD;  // high precision measurement
constexpr uint8_t CMD_READ_SERIAL   = 0x89;  // read 32-bit serial number
constexpr uint8_t CMD_SOFT_RESET    = 0x94;  // reset the sensor

unsigned long lastSend = 0;

//Used to validate that data wasnt corrupted
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

bool sht41SendCommand(uint8_t cmd) {
  Wire.beginTransmission(SHT41_ADDR);
  //Send start bit and SHT41 adress
  Wire.write(cmd);
  //Send write bit and command
  return Wire.endTransmission() == 0;  
  // 0 means ACK bit received
}

bool sht41ReadSixBytes(uint8_t buf[6]) {
  uint8_t got = Wire.requestFrom(SHT41_ADDR, (uint8_t)6);
  //The number of bytes received
  if(got!=6) return false;
  //Make sure 6 bytes were received from the SHT41

  for(uint8_t i=0; i<6; i++){
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

void sendFrame(uint32_t id, const uint8_t *data, uint8_t length) {
  CanFrame frame = { 0 };            // start with everything zeroed
  frame.identifier       = id;       // message ID
  frame.extd             = 0;        // 0 = standard 11-bit ID, 1 = extended 29-bit
  frame.data_length_code = length;   // number of data bytes (0 to 8)

  for (uint8_t i = 0; i < length; i++) {
    frame.data[i] = data[i];
  }

  if (ESP32Can.writeFrame(frame)) {
    Serial.println("Frame sent");
  } else {
    Serial.println("Send failed");
  }
}

//-----------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);

  //I2C Setup Code
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);  // 400 kHz fast mode

  if (!sht41Restart()) {
    Serial.println("SHT41 did not respond. Check wiring and address.");
    return;
  }

  uint32_t serial;
  if (sht41ReadSerial(serial)) {
    Serial.print("SHT41 serial number: 0x");
    Serial.println(serial, HEX);
  } else {
    Serial.println("Could not read serial number (CRC or bus error).");
  }
  

  //CAN Setup Code

  ESP32Can.setPins(CAN_TX, CAN_RX);
  ESP32Can.setSpeed(ESP32Can.convertSpeed(250));  // 250 kbps

  if (!ESP32Can.begin()) {
    Serial.println("Starting CAN failed!");
    while (1) delay(1000);
  }


}

void loop() {
  float temp, humidity;

  //Read temperature and humidity from the SHT41 sensor
  if (sht41Read(temp, humidity)) {
    Serial.print("Temperature: ");
    Serial.print(temp, 2);
    Serial.print(" C   Humidity: ");
    Serial.print(humidity, 2);
    Serial.println(" %");
  } else {
    Serial.println("Read failed (no ACK, short read, or CRC mismatch).");
  }
  


  //Listen for incoming CAN frames
  CanFrame frame;
  if (ESP32Can.readFrame(frame, 1000)) {   // wait up to 1000 ms
    Serial.printf("ID 0x%03X, len %d\n", frame.identifier, frame.data_length_code);
  } else {
    Serial.println("No CAN frame received within timeout.");
  }
  


  delay(60000); 
  if (millis() - lastSend >= 1000) {
    lastSend = millis();

    uint8_t payload[] = { 0x01, 0xFF, 0x10 };
    sendFrame(0x123, payload, sizeof(payload));
  }
}
*/