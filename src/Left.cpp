#include <ESP32-TWAI-CAN.hpp>

// ---- CAN settings (change these in one place) ----
constexpr int CAN_TX_PIN     = 4;
constexpr int CAN_RX_PIN     = 3;
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

void setup() {
  delay(10000); //pause for 
  Serial.begin(115200);
  initCan();
}

void loop() {
  CanFrame frame;
  if (ESP32Can.readFrame(frame, 1000)) {   // wait up to 1000 ms
    Serial.printf("ID 0x%03X, len %d\n", frame.identifier, frame.data_length_code);
  } else {
    Serial.println("No CAN frame received");
  }
}