#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

/* Classe per la gestione del display e di tutte le informazioni che verranno visualizzate */
class DataDisplay: public Adafruit_SSD1306 {

  public:
    DataDisplay();

    /*
    Inizializzazione display.

    @return stato inizializzazione
    */
    bool begin();

    /*
    Visualizza i valori di co2, temperatura, umidità e decibel nel display.
    I valori co2_alert, temperature_alert, humidity_alert, decibel_alert sono valori booleani che se sono a true faranno lampeggiare il valore corrispondente una volta.

    @param co2
    @param temperature
    @param humidity
    @param decibel
    @param co2_alert
    @param temperature_alert
    @param humidity_alert
    @param debicel_alert
    */
    void printValues(uint16_t co2, float temperature, float humidity, float decibel, bool co2_alert, bool temperature_alert, bool humidity_alert, bool decibel_alert);
    
    /*
    Visualizza un piano cartesiano nel display. L'asse x rappresenta l'indice dei valori mentre l'asse y il rispettivo valore.

    @param text una scritta disegnata sotto il piano cartesiano
    @param values array in ordine dei valori da visualizzare
    @param len numero di elementi dell'array
    @maxValue valore massimo rappresentabile nell'asse y del piano cartesiano
    */
    void printGraph(const char *text, float *values, int len, float maxValue);

    /*
    Visualizza lo stato del WiFi e della connessione MQTT nella parte alta del diaplay. 

    @param wifiConnected stato WiFi
    @param mqttConnected stato MQTT
    */
    void printConnectionStatus(bool wifiConnected, bool mqttConnected);
    

};