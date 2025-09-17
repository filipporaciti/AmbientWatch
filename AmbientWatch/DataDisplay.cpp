#include <Adafruit_SSD1306.h>
#include "DataDisplay.h"
#include <Wire.h>
#include <Fonts/TomThumb.h> // font piccolo

DataDisplay::DataDisplay(): Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}


bool DataDisplay::begin() {
  return Adafruit_SSD1306::begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
}

void DataDisplay::printValues(uint16_t co2, float temperature, float humidity, float decibel, bool co2_alert, bool temperature_alert, bool humidity_alert, bool decibel_alert) {
  Adafruit_SSD1306::setTextSize(1);
  Adafruit_SSD1306::setTextColor(SSD1306_WHITE);

  Adafruit_SSD1306::setCursor(5, 14);
  Adafruit_SSD1306::print(F("CO2: "));
  Adafruit_SSD1306::print(String(co2).c_str());
  Adafruit_SSD1306::print(F(" ppm\n"));
  Adafruit_SSD1306::println();

  Adafruit_SSD1306::setCursor(5, 27);
  Adafruit_SSD1306::print(F("Temperature: "));
  char buf[6];
  dtostrf(temperature, 5, 2, buf);
  Adafruit_SSD1306::print(buf);
  Adafruit_SSD1306::print(F(" C\n"));
  Adafruit_SSD1306::println();

  Adafruit_SSD1306::setCursor(5, 40);
  Adafruit_SSD1306::print(F("Humidity: "));
  dtostrf(humidity, 5, 2, buf);
  Adafruit_SSD1306::print(buf);
  Adafruit_SSD1306::print(F(" %\n"));

  Adafruit_SSD1306::setCursor(5, 53);
  Adafruit_SSD1306::print(F("Decibel: "));
  dtostrf(decibel, 5, 2, buf);
  Adafruit_SSD1306::print(buf);
  Adafruit_SSD1306::print(F(" db\n"));

  // ----- In caso di alert -----
  if (co2_alert) {
    Adafruit_SSD1306::fillRect(3, 12, Adafruit_SSD1306::width()-6, 11, SSD1306_INVERSE);
  }
  if (temperature_alert) {
    Adafruit_SSD1306::fillRect(3, 25, Adafruit_SSD1306::width()-6, 11, SSD1306_INVERSE);
  }
  if (humidity_alert) {
    Adafruit_SSD1306::fillRect(3, 38, Adafruit_SSD1306::width()-6, 11, SSD1306_INVERSE);
  }
  if (decibel_alert) {
    Adafruit_SSD1306::fillRect(3, 51, Adafruit_SSD1306::width()-6, 11, SSD1306_INVERSE);
  }
  Adafruit_SSD1306::display();
  if (co2_alert || temperature_alert || humidity_alert || decibel_alert) {
    delay(500);
  }

  if (co2_alert) {
    Adafruit_SSD1306::fillRect(3, 12, Adafruit_SSD1306::width()-6, 11, SSD1306_INVERSE);
  }
  if (temperature_alert) {
    Adafruit_SSD1306::fillRect(3, 25, Adafruit_SSD1306::width()-6, 11, SSD1306_INVERSE);
  }
  if (humidity_alert) {
    Adafruit_SSD1306::fillRect(3, 38, Adafruit_SSD1306::width()-6, 11, SSD1306_INVERSE);
  }
  if (decibel_alert) {
    Adafruit_SSD1306::fillRect(3, 51, Adafruit_SSD1306::width()-6, 11, SSD1306_INVERSE);
  }
  Adafruit_SSD1306::display();
  if (co2_alert || temperature_alert || humidity_alert || decibel_alert) {
    delay(500);
  }
  // ---------------
}


void DataDisplay::printGraph(const char *text, float *values, int len, float maxValue) {
  const int marginLeft = 3;
  const int marginBottom = 3 + 7 + 2;
  const int graphWidth = Adafruit_SSD1306::width() - marginLeft - 1;
  const int graphHeight = Adafruit_SSD1306::height() - marginBottom - 10;

  int originX = marginLeft;
  int originY = Adafruit_SSD1306::height() - marginBottom;

  // Asse Y
  Adafruit_SSD1306::drawLine(originX, originY, originX, originY - graphHeight, SSD1306_WHITE);
  Adafruit_SSD1306::drawLine(originX, originY - graphHeight, originX - 2, originY - graphHeight + 4, SSD1306_WHITE); // freccia
  Adafruit_SSD1306::drawLine(originX, originY - graphHeight, originX + 2, originY - graphHeight + 4, SSD1306_WHITE); // freccia

  // Asse X
  Adafruit_SSD1306::drawLine(originX, originY, originX + graphWidth, originY, SSD1306_WHITE);
  Adafruit_SSD1306::drawLine(originX + graphWidth, originY, originX + graphWidth - 4, originY - 2, SSD1306_WHITE); // freccia
  Adafruit_SSD1306::drawLine(originX + graphWidth, originY, originX + graphWidth - 4, originY + 2, SSD1306_WHITE); // freccia

  // Tacche asse Y
  for (int i = 0; i < 6; i++) {
    int y = originY - (i * graphHeight / 6);
    Adafruit_SSD1306::drawLine(originX - 2, y, originX, y, SSD1306_WHITE);
  }

  // Tacche asse X
  int space_x = graphWidth / len;

  for (int i = 0; i < (len-1); i++) { // il -1 perchè si sovrappone alla freccia
    int x = originX + (i * space_x);
    Adafruit_SSD1306::drawLine(x, originY, x, originY + 2, SSD1306_WHITE);
  }

  // disegno il grafico
  for (int i = 1; i < len; i++) {
    int x0 = originX + (i - 1) * space_x;
    int y0 = originY - (values[i - 1] * graphHeight / maxValue);

    int x1 = originX + i * space_x;
    int y1 = originY - (values[i] * graphHeight / maxValue);

    Adafruit_SSD1306::drawLine(x0, y0, x1, y1, SSD1306_WHITE);
  }

  // prin bottom text
  Adafruit_SSD1306::setTextSize(1);
  Adafruit_SSD1306::setTextColor(SSD1306_WHITE);
  Adafruit_SSD1306::setCursor(0, Adafruit_SSD1306::height() - 7);
  Adafruit_SSD1306::print(text);
  Adafruit_SSD1306::print(values[len-1]);

  Adafruit_SSD1306::display();
}

void DataDisplay::printConnectionStatus(bool wifiConnected, bool mqttConnected) {
  Adafruit_SSD1306::setTextSize(1);
  Adafruit_SSD1306::setTextColor(SSD1306_WHITE);
  Adafruit_SSD1306::setCursor(0, 0);              // In alto a sinistra

  // Simbolo WiFi
  Adafruit_SSD1306::print("WiFi: ");
  Adafruit_SSD1306::print(wifiConnected ? "Yes" : "No");

  // Spazio tra i simboli
  Adafruit_SSD1306::print("  ");

  // Simbolo MQTT
  Adafruit_SSD1306::print("MQTT: ");
  Adafruit_SSD1306::print(mqttConnected ? "Yes" : "No");

  Adafruit_SSD1306::drawLine(0, 8, Adafruit_SSD1306::width(), 8, SSD1306_WHITE);

  Adafruit_SSD1306::display();
}
