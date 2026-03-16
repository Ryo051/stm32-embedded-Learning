#include <ESP32SPISlave.h>

ESP32SPISlave slave;

static constexpr size_t BUFFER_SIZE = 1;
uint8_t tx_buf[BUFFER_SIZE] = {0xAA};
uint8_t rx_buf[BUFFER_SIZE] = {0};

void setup() {
  Serial.begin(115200);
  delay(2000);

  slave.setDataMode(SPI_MODE0);
  slave.setQueueSize(1);
  slave.begin();

  Serial.println("ESP32 SPI Slave Start");
}

void loop() {
  rx_buf[0] = 0;

  size_t received_bytes = slave.transfer(tx_buf, rx_buf, BUFFER_SIZE);

  if (received_bytes > 0) {
    if (rx_buf[0] == 0x01) {
      Serial.println("Motion Detected");
    } else if (rx_buf[0] == 0x00) {
      Serial.println("No Motion");
    } else {
      Serial.print("Unknown: 0x");
      Serial.println(rx_buf[0], HEX);
    }
  }
}