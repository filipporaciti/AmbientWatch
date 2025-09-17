#include <PubSubClient.h>
#include <WiFi.h>

/* Classe per la gestione del WiFi e della comunicazione con il server MQTT */
class WiFi_MQTT: public PubSubClient {

  private:
    // istanza per la connessione WiFi
    WiFiClient _wifiClient; 
    // WiFi SSID
    const char *_ssid;
    // WiFI password
    const char *_password;
    // Indirizzo al server MQTT
    const char *_mqttServer;
    // Porta del server MQTT. Nel costruttore di default viene impostato a 1883.
    int _mqttPort;

  public:
    // True se nell'ultimo tentativo il dispositivo era connesso al WiFi.
    bool wifiStatus = false;
    // True se nell'ultimo tentativo il dispositivo era connesso al server MQTT.
    bool mqttStatus = false;

    WiFi_MQTT(const char *SSID, const char *password, const char *mqttServer, int mqttPort=1883);
    
    /*
    Manda i dati della co2, temperatura, umidità e decibel al server MQTT se il dispositivo è connesso, altrimenti ritorna false.

    @param topicPrefix prefisso del topic del publish al server MQTT
    @param co2
    @param temperature
    @param humidity
    @param decibel
    @return true se il dispositivo è connesso al server MQTT, false altrimenti.
    */
    bool publishData(String topicPrefix, uint16_t co2, float temperature, float humidity, float decibel);

    /*
    Manda i limiti per attivare gli alert al server MQTT se il dispositivo è connesso, altrimenti ritorna false.

    @param topicPrefix prefisso del topic del publish al server MQTT
    @param co2_max
    @param temp_min
    @param temp_max
    @param hum_min
    @param hum_max
    @param dec_max
    @return true se il dispositivo è connesso al server MQTT, false altrimenti.
    */
    bool publishAlertLimits(String topicPrefix, uint16_t co2_max, float temp_min, float temp_max, float hum_min, float hum_max, float dec_max);

    /*
    Manda un valore booleano che indica lo stato dell'alert (se è attivo o meno nel dispositivo) al server MQTT se il dispositivo è connesso, altrimenti ritorna false.

    @param topicPrefix prefisso del topic del publish al server MQTT
    @param active
    @return true se il dispositivo è connesso al server MQTT, false altrimenti.
    */
    bool publishAlertStatus(String topic, bool active);

    /*
    Controlla se ci sono nuovi messaggi da parte del server a cui il client si era iscritto in precedenza se il dispositivo è connesso, altrimenti ritorna false.

    @return true se il dispositivo è connesso al server MQTT, false altrimenti.
    */
    bool updateSubscribe();

    /*
    Disconnette e spegne il WiFi.
    */
    void wifiDisconnect();

    /*
    Si connette al server MQTT e fa il subscribe al topic dato come parametro. Esegue tre tentativi.

    @param topic
    @return true se il dispositivo è connesso al server MQTT, false altrimenti.
    */
    bool mqttConnect(const char* topic);

    /*
    Accende e si collega al WiFi.

    @return true in caso di successo, false altrimenti.
    */
    bool wifiConnect();

    /*
    Accendeil WiFi, si collega al WiFi e si connette al server MQTT.

    @return true in caso di successo, false altrimenti.
    */
    bool connect(const char* topic);

    /*
    Si disconnette dal server MQTT e dal WiFi e spegne il WiFi.
    */
    void disconnect();
};

