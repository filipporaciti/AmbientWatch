#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include "SCD41.h"

SCD41::SCD41(): SensirionI2cScd4x() {}

bool SCD41::begin() {
  Wire.begin();
  SensirionI2cScd4x::begin(Wire, SCD41_I2C_ADDR_62);

  delay(30); // delay per evitare di inviale richieste quando il sensore non è pronto

  int16_t error = SensirionI2cScd4x::wakeUp();
  if (error != 0) {
      return false;
  }
  error = SensirionI2cScd4x::stopPeriodicMeasurement();
  if (error != 0) {
      return false;
  }
  error = SensirionI2cScd4x::reinit();
  if (error != 0) {
      return false;
  }
  return true;
}

bool SCD41::getValues(uint16_t& co2, float& temperature, float& humidity) {

  for (int i=0; i<3; i++) { // fai massimo tre tentativi
    Serial.print(i+1);
    Serial.print(" try...");

    // Avvia misurazione single-shot
    uint16_t error = SensirionI2cScd4x::measureSingleShot();
    if (error != 0) {
      Serial.print("Errore single-shot: ");
      Serial.println(error);
      return false;
    }
    // Attendi completamento (5 secondi per SCD41)
    delay(5000);

    error = SensirionI2cScd4x::readMeasurement(co2, temperature, humidity);
    if (error == 0) {
        Serial.println("Read success!!!");
        return true;
    }
    begin();
    delay(1000);
  }
  return false;
}
