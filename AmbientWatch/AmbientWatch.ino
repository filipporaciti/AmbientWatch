#include <Preferences.h>
#include <ESP32Encoder.h>
#include <Wire.h>
#include "DataDisplay.h"
#include "WiFi_MQTT.h"
#include "INMP441.h"
#include "SCD41.h"
#include "secrets.h"

#define TRA 15
#define TRB 17
#define BUZZER_PIN 12

#define LIGHT_SLEEP_TIME 60000 * 5

#define MQTT_TOPIC_SUBSCRIBE "esame/pippo/alert_limits/#"
#define MQTT_PUBLISH_ALERT_SOFT "esame/pippo/alert_limits/soft"
#define MQTT_PUBLISH_ALERT_STRONG "esame/pippo/alert_limits/strong"
#define MQTT_PUBLISH_ALERT_STATUS_SOFT "esame/pippo/alert_status/soft"
#define MQTT_PUBLISH_ALERT_STATUS_STRONG "esame/pippo/alert_status/strong"
#define MQTT_PUBLISH_DATA "esame/pippo/data"

TaskHandle_t core0TaskHandle = NULL;

static IRAM_ATTR void resumeDisplayTask(void* arg) {
  if (core0TaskHandle != NULL) {
    vTaskResume(core0TaskHandle);
  }
}

uint16_t co2 = 0;
float temperature = 0.0;
float humidity = 0.0;
float decibel = 0.0;

bool co2_alert = false;
bool temperature_alert = false;
bool humidity_alert = false;
bool decibel_alert = false;

uint16_t soft_co2_max = 0; 
float soft_temp_min = 0;
float soft_temp_max = 0;
float soft_hum_min = 0;
float soft_hum_max = 0;
float soft_dec_max = 0;

uint16_t strong_co2_max = 0; 
float strong_temp_min = 0;
float strong_temp_max = 0;
float strong_hum_min = 0;
float strong_hum_max = 0;
float strong_dec_max = 0;

bool sound_alert = false;

const int numValues = 31;

float co2Values[numValues] = { 0 };
float co2MaxVal = co2;

float tempValues[numValues] = { 0 };
float tempMaxVal = temperature;

float humValues[numValues] = { 0 };
float humMaxVal = humidity;

float decValues[numValues] = { 0 };
float decMaxVal = decibel;

SCD41 sensor;
DataDisplay display;
WiFi_MQTT client(WIFI_SSID, WIFI_PASSWORD, MQTT_IP);
INMP441 decibelSensor;

Preferences prefs;

ESP32Encoder encoder(true, resumeDisplayTask);

long oldPosition = -999;
int encoderValue = 0;

void core0Task(void* parameter) {
  encoderTrigger();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println("");

  loadAlertSettings();
  
  pinMode(BUZZER_PIN, OUTPUT);

  encoder.attachSingleEdge(TRA, TRB);
  encoder.clearCount();

  Wire.begin();

  if (!display.begin()) {
    Serial.println("ERROR: Display begin error");
    for (;;);
  }
  display.clearDisplay(); // stampo schermo nero
  display.display();

  client.connect(MQTT_TOPIC_SUBSCRIBE);
  client.setCallback(callback);
  client.publishAlertLimits(MQTT_PUBLISH_ALERT_SOFT, soft_co2_max, soft_temp_min, soft_temp_max, soft_hum_min, soft_hum_max, soft_dec_max);
  client.publishAlertLimits(MQTT_PUBLISH_ALERT_STRONG, strong_co2_max, strong_temp_min, strong_temp_max, strong_hum_min, strong_hum_max, strong_dec_max);
  client.disconnect();
  
  // lo stampo dopo in questo modo se la batteria è scarica mostra solo lo schermo nero e non quello dei dati lampeggianti
  display.clearDisplay();
  display.printConnectionStatus(false, false);
  display.printValues(co2, temperature, humidity, decibel, false, false, false, false);
  display.display();

  if (!sensor.begin()) {
    Serial.println("ERROR: SCD41 sensor error begin");
    for (;;);
  }

  xTaskCreatePinnedToCore(
    core0Task,         // nome funzione
    "core0Task",       // nome task
    4096,              // grandezza stack
    NULL,              // parametri
    1,                 // priorità
    &core0TaskHandle,  // handle
    0                  // Core 0
  );

}

void loop() {
  Serial.println("----- Start loop -----");

  unsigned long mill_start = millis();

  // ----- Reading data -----
  Serial.print("Reading co2, temperature, humidity values...");
  sensor.getValues(co2, temperature, humidity);
  pushInCoda(co2Values, (float)co2, numValues);
  pushInCoda(tempValues, temperature, numValues);
  pushInCoda(humValues, humidity, numValues);
  if (tempMaxVal < temperature) {
    tempMaxVal = temperature;
  }
  if (co2MaxVal < co2) {
    co2MaxVal = co2;
  }
  if (humMaxVal < humidity) {
    humMaxVal = humidity;
  }
  Serial.println("end");

  Serial.print("Reading decibel...");
  decibel = decibelSensor.readAvgValue();
  pushInCoda(decValues, decibel, numValues);
  if (decMaxVal < decibel) {
    decMaxVal = decibel;
  }
  Serial.println("end");

  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.println(" ppm");

  Serial.print("Environment temperature : ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Relative humidity : ");
  Serial.print(humidity);
  Serial.println(" RH");

  Serial.print("Decibel : ");
  Serial.print(decibel);
  Serial.println(" db");
  // ----- End reading data -----

  checkValues();

  // ----- MQTT -----
  client.connect(MQTT_TOPIC_SUBSCRIBE);
  client.publishData(MQTT_PUBLISH_DATA, co2, temperature, humidity, decibel);
  client.updateSubscribe();
  saveAlertSettings(); // se ci sono stati aggironamenti, li salvo
  // client.publishSoftAlertLimits(soft_co2_max, soft_temp_min, soft_temp_max, soft_hum_min, soft_hum_max, soft_dec_max); // potrei metterli per eliminare i retain
  // client.publishStrongAlertLimits(strong_co2_max, strong_temp_min, strong_temp_max, strong_hum_min, strong_hum_max, strong_dec_max);
  client.publishAlertStatus(MQTT_PUBLISH_ALERT_STATUS_SOFT, co2_alert || temperature_alert || humidity_alert || decibel_alert);
  client.publishAlertStatus(MQTT_PUBLISH_ALERT_STATUS_STRONG, sound_alert);
  client.disconnect();
  // ----- End MQTT -----

  updateDisplay();

  if (!(co2_alert || temperature_alert || humidity_alert || decibel_alert || sound_alert)) {  // se non ho alert allora posso andare il sleep, altrimenti aggiorno i valori
    while (eTaskGetState(core0TaskHandle) != eSuspended) { // aspetta che il task abbia finito
      delay(70);
    }
    unsigned long m = millis();
    unsigned long sleep_time = LIGHT_SLEEP_TIME - (m - mill_start);
    if (sleep_time < 0 || sleep_time > LIGHT_SLEEP_TIME) {
      sleep_time = LIGHT_SLEEP_TIME;
    }
    anyway_sleep_timer(sleep_time);
  }
}

/*
Fa andare l'esp32 in light sleep mode per il tempo impostato dal parametro time_sleep. 
L'esp32 può uscire dalla sleep mode attraverso un interrupt oppure quando il tempo è terminato.
In caso l'esp32 esca dalla sleep mode attraverso l'interrupt, la funzione si occuperà di rimettere l'esp32 in sleep mode appena possibile fino alla fine del tempo prestabilito.

@param time_sleep tempo in cui l'esp32 dovrà stare in sleep mode
*/
void anyway_sleep_timer(unsigned long time_sleep) {
  unsigned long m_start = 0;
  unsigned long time_left = time_sleep;

  while (time_left > 1000) {  // qualche millisecondo di range per evitare falsi positivi
    Serial.print("Light sleep mode. Time left: ");
    Serial.println(time_left);
    Serial.flush();

    m_start = millis();
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 0);
    esp_sleep_enable_timer_wakeup(time_left * 1000);  // vuole microsecondi come parametro
    esp_light_sleep_start();

    resumeDisplayTask(NULL);
    while (eTaskGetState(core0TaskHandle) != eSuspended) { // aspetta che il task abbia finito
      delay(70);
      m_start += 70;
    }

    unsigned long m_now = millis();
    unsigned long sleep = time_left - (m_now - m_start);  // vedo quanto è passato
    if (time_left < m_now - m_start) {
      sleep = 0;
    }

    if (sleep > time_left || m_now < m_start) {  // per eventuali overflow
      Serial.println("Timer reset");
      time_left = 0;
    } else {
      time_left = sleep;
    }
  }
}

/*
Serve per gestire l'encoder e l'aggiornamento del display.
*/
void encoderTrigger() {
  while (true) {
    unsigned long encoderTime = millis();

    while (millis() - encoderTime < 1000) {
      long newPosition = encoder.getCount();
      if (newPosition != oldPosition) {
        encoderTime = millis(); // resetto il tempo di fine task (se non ci sono valori critici)
        oldPosition = newPosition;
        encoderValue = newPosition;
        if (encoderValue < 0) { // il modulo di un numero negativo lo tratta come se fosse positivo :(
          int x = (abs(encoderValue)/5)+1;
          encoderValue += 5*x;
        }
        encoderValue = (encoderValue) % 5;
        Serial.print("Display position: ");
        Serial.println(encoderValue);
        // il display devo aggiornarlo almeno una volta, e in caso ci sia un alert tutte le volte finchè non termina l'alert
        do {
          if (co2_alert || temperature_alert || humidity_alert || decibel_alert) encoderValue = 0;
          checkValues();
          printDisplay();
        } while (co2_alert || temperature_alert || humidity_alert || decibel_alert);
      }
      yield();  // per evitare il watchdog. yield permette di lasciare la CPU ad altri task
    }
    Serial.println("Display task suspend");
    vTaskSuspend(core0TaskHandle);
    Serial.println("Display task resume");
  }
}

/*
Aggiorna il display.
*/
void updateDisplay() {
  oldPosition = -999;
  resumeDisplayTask(NULL);
}

/*
Visualizza le informazioni nel display in base al valore dell'encoder in quel momento.
*/
void printDisplay() {
  display.clearDisplay();
  display.printConnectionStatus(client.wifiStatus, client.mqttStatus);
  if (encoderValue == 0) {
    display.printValues(co2, temperature, humidity, decibel, co2_alert, temperature_alert, humidity_alert, decibel_alert);
  } else if (encoderValue == 1) {
    display.printGraph("Decibel: ", decValues, numValues, decMaxVal);
  } else if (encoderValue == 2) {
    display.printGraph("Humidity: ", humValues, numValues, humMaxVal);
  } else if (encoderValue == 3) {
    display.printGraph("Temperature: ", tempValues, numValues, tempMaxVal);
  } else if (encoderValue == 4) {
    display.printGraph("CO2: ", co2Values, numValues, co2MaxVal);
  } else {
    display.clearDisplay();
    display.display();
  }
}

/*
Dato un'array, la funzione mette l'elemento in posizione i nella posizione i-1.
L'elemento in posizione 0 verrà eliminato mentre l'elemento nell'ultima posizione sarà il parametro newVal.

@param arr array da modificare
@param newVal nuovo valore che verrà posizionato in posizione len-1
@param len lunghezza array
*/
void pushInCoda(float* arr, float newVal, int len) {
  for (int i = 0; i < len - 1; i++) {
    arr[i] = arr[i + 1];
  }
  arr[len - 1] = newVal;
}

/*
Controlla che i valori non superino i soft e strong limit e imposta le variabili degli alert a true se li supera, altrimenti a false. 
La funzione inoltre attiva e disattiva il buzzer in caso i valori superino gli strong limit.
*/
void checkValues() {
  if (co2 != 0 && temperature != 0 && humidity != 0) { // in caso non siano stati presi i valori, non faccio il check dei valori
    co2_alert = co2 > soft_co2_max;
    temperature_alert = temperature < soft_temp_min || temperature > soft_temp_max;
    humidity_alert = humidity < soft_hum_min || humidity > soft_hum_max;
    decibel_alert = decibel > soft_dec_max;
    sound_alert = co2 > strong_co2_max || temperature < strong_temp_min || temperature > strong_temp_max || humidity < strong_hum_min || humidity > strong_hum_max;
    if (sound_alert) {
      digitalWrite(BUZZER_PIN, HIGH); 
    } else {
      digitalWrite(BUZZER_PIN, LOW);    
    }
  }
}

/*
Salva in maniera persistente i soft e strong limit dalle variabili corrispondenti.
*/
void saveAlertSettings() {
  prefs.begin("AmbientWatch", false); // false = scrittura

  prefs.putUShort("soft_co2_max", soft_co2_max);
  prefs.putFloat("soft_temp_min", soft_temp_min);
  prefs.putFloat("soft_temp_max", soft_temp_max);
  prefs.putFloat("soft_hum_min", soft_hum_min);
  prefs.putFloat("soft_hum_max", soft_hum_max);
  prefs.putFloat("soft_dec_max", soft_dec_max);

  prefs.putUShort("strong_co2_max", strong_co2_max);
  prefs.putFloat("strong_temp_min", strong_temp_min);
  prefs.putFloat("strong_temp_max", strong_temp_max);
  prefs.putFloat("strong_hum_min", strong_hum_min);
  prefs.putFloat("strong_hum_max", strong_hum_max);
  prefs.putFloat("strong_dec_max", strong_dec_max);

  prefs.end();
  Serial.println("Save alert settings");
}

/*
Carica dalla memoria i soft e strong limit e li mette nelle variabili corrispondenti. In caso non ci siano, saranno caricati dei valori di default.
*/
void loadAlertSettings() {
  prefs.begin("AmbientWatch", true); // true = lettura

  soft_co2_max = prefs.getUShort("soft_co2_max", 2000);
  soft_temp_min = prefs.getFloat("soft_temp_min", 19);
  soft_temp_max = prefs.getFloat("soft_temp_max", 30);
  soft_hum_min = prefs.getFloat("soft_hum_min", 30);
  soft_hum_max = prefs.getFloat("soft_hum_max", 70);
  soft_dec_max = prefs.getFloat("soft_dec_max", 70);

  strong_co2_max = prefs.getUShort("strong_co2_max", 5000);
  strong_temp_min = prefs.getFloat("strong_temp_min", 10);
  strong_temp_max = prefs.getFloat("strong_temp_max", 37);
  strong_hum_min = prefs.getFloat("strong_hum_min", 15);
  strong_hum_max = prefs.getFloat("strong_hum_max", 90);
  strong_dec_max = prefs.getFloat("strong_dec_max", 120);

  prefs.end();
  Serial.println("Load alert settings");
}

/*
Questa funzione verrà richiamata ogni volta che l'esp32 riceve un messaggio a cui aveva fatto il subscribe. 
La funzione individua il topic e salva il valore del payload nella variabile corrispondente (serve per modificare i valori dei limiti).
*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("subscribe: Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);

  if (strcmp(topic, MQTT_PUBLISH_ALERT_SOFT "/co2_max") == 0) {
    soft_co2_max = message.toInt();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_SOFT "/temp_min") == 0) {
    soft_temp_min = message.toFloat();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_SOFT "/temp_max") == 0) {
    soft_temp_max = message.toFloat();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_SOFT "/hum_min") == 0) {
    soft_hum_min = message.toFloat();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_SOFT "/hum_max") == 0) {
    soft_hum_max = message.toFloat();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_SOFT "/dec_max") == 0) {
    soft_dec_max = message.toFloat();
  }

  if (strcmp(topic, MQTT_PUBLISH_ALERT_STRONG "/co2_max") == 0) {
    strong_co2_max = message.toInt();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_STRONG "/temp_min") == 0) {
    strong_temp_min = message.toFloat();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_STRONG "/temp_max") == 0) {
    strong_temp_max = message.toFloat();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_STRONG "/hum_min") == 0) {
    strong_hum_min = message.toFloat();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_STRONG "/hum_max") == 0) {
    strong_hum_max = message.toFloat();
  }
  if (strcmp(topic, MQTT_PUBLISH_ALERT_STRONG "/dec_max") == 0) {
    strong_dec_max = message.toFloat();
  }
}