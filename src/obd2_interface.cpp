#include "obd2_interface.h"

void initOBD2Interface()
{
  Serial2.begin(115200, SERIAL_8N1, LementGatewayConfig::OBD2_RX_PIN, LementGatewayConfig::OBD2_TX_PIN);
  Serial.println("OBD2 interface initialized");
}
