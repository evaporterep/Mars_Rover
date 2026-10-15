#include <ESP32-TWAI-CAN.hpp>

// ---- CAN settings (change these in one place) ----
constexpr int CAN_TX_PIN     = 21;
constexpr int CAN_RX_PIN     = 20;
constexpr int CAN_SPEED_KBPS = 250;

// Print a message and stop forever
void halt(const char *message) {
  Serial.println(message);
  while (true) delay(1000);
}

void initCan() {
  ESP32Can.setPins(CAN_TX_PIN, CAN_RX_PIN);
  ESP32Can.setSpeed(ESP32Can.convertSpeed(CAN_SPEED_KBPS));

  if (!ESP32Can.begin()) {
    halt("CAN failed to start. Check wiring and transceiver power.");
  }
  Serial.println("CAN started");
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

void setup() {
  delay(10000); //pause for 
  Serial.begin(115200);
  initCan();
}

unsigned long lastSend = 0;

void loop() {
  if (millis() - lastSend >= 1000) {
    Serial.println("testing");
    lastSend = millis();

    uint8_t payload[] = { 0x01, 0xFF, 0x10 };
    sendFrame(0x123, payload, sizeof(payload));
  }
}
