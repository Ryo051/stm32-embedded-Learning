#include <ESP32SPISlave.h>
/*Pin List
GPIO14 SCK
GPIO13 MOSI
GPIO12 MISO
GPIO15 CS
*/ 
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
  Serial.println("loop alive");
  delay(500);

  rx_buf[0] = 0;
  size_t received_bytes = slave.transfer(tx_buf, rx_buf, BUFFER_SIZE);

  Serial.print("received_bytes = ");
  Serial.println(received_bytes);

  if (received_bytes > 0) {
    Serial.print("Received: 0x");
    Serial.println(rx_buf[0], HEX);
  }
}
